#pragma once

// STL
#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <sstream>
#include <thread>
#include <vector>

// Windows
#include <windows.h>

namespace Util
{
	enum class SeverityType
	{
		Info,
		Debug,
		Warning,
		Error,
	};

	// Class declarations
	template<typename LogPolicy>
	class Logger;

	template<typename LogPolicy>
	void LoggingDaemon(Logger<LogPolicy>* logger)
	{
		// Dump log data if present
		std::unique_lock<std::timed_mutex> lock(logger->m_writeMutex, std::defer_lock);
		do
		{
			std::this_thread::sleep_for(std::chrono::milliseconds{ 50 });
			if (logger->m_logBuffer.size()) {
				if (!lock.try_lock_for(std::chrono::milliseconds{ 50 })) {
					continue;
				}

				for (auto& x : logger->m_logBuffer) {
					logger->m_policy.Write(x);
				}

				logger->m_logBuffer.clear();
				lock.unlock();
			}
		} while (logger->b_isStillRunning.test_and_set() || logger->m_logBuffer.size());
	}

	template<typename LogPolicy>
	class Logger
	{
	private:
		// Used to specify line numbers in output.
		unsigned int							m_logLineNumber;

		// Option to give human-readable thread name to each running thread to make reading easier.
		std::map<std::thread::id, std::string>	m_threadName;

		// The policy to use, e.g., print to file or console.
		LogPolicy								m_policy;

		// Used to write content of log buffer to a designeated output src, e.g., file on hard drive.
		// Timed mutex: time lockable obj designed to signal when critical sections of code need
		// exclusive access, like regular mutex, but also provide timed try-lock requests.
		std::timed_mutex						m_writeMutex;

		// Contains elements (events, msgs, warnings, etc) to print.
		std::vector<std::string>				m_logBuffer;

		// Thread used to actually print m_logBuffer.
		std::thread								m_daemon;

		// Used to check whether m_daemon is still alive or not.
		// Atomic flag: lock-free bool atomic obj.
		std::atomic_flag						b_isStillRunning{ ATOMIC_FLAG_INIT };

	public:
		Logger(const std::wstring& fileName);
		~Logger();

		void SetThreadName(const std::string& name);

		template<SeverityType severity>
		void Print(std::stringstream stream);

		template<SeverityType severity>
		void Print(std::string msg);

		template<typename Policy>
		friend void LoggingDaemon(Logger<Policy>* logger);
	};


	////////////////////////////////////////////////////////////////////////////////////////
	//////////////// DEFINITIONS

	template<typename LogPolicy>
	inline Logger<LogPolicy>::Logger(const std::wstring & fileName)
		: m_logLineNumber(0), m_threadName(), m_policy(), m_writeMutex(), m_logBuffer()
	{
		// If opening the outputstream is successful, set atomically to true, daemon moved to
		// running thread.
		if (m_policy.OpenOutputStream(fileName)) {
			b_isStillRunning.test_and_set();
			m_daemon = std::move(std::thread{ LoggingDaemon<LogPolicy>, this });
		} else {
			throw std::runtime_error("Unable to open log file:\n\t");
		}
	}

	template<typename LogPolicy>
	inline Logger<LogPolicy>::~Logger()
	{
#ifndef NDEBUG
		Util::ServiceLocator::GetFileLogger()->Print<Util::SeverityType::Info>("The logger is shutting down!");
#endif
		// Terminate the daemon by clearing the still running flag & letting it join main thread.
		// Join: wait till it finished whatever it was doing then put to rest.
		b_isStillRunning.clear();
		m_daemon.join();

		// Clear m_threadName
		m_threadName.clear();
		std::map<std::thread::id, std::string>().swap(m_threadName);

		// Clear m_logBuffer.
		m_logBuffer.clear();
		m_logBuffer.shrink_to_fit();

		// Close outputstream
		m_policy.CloseOutputStream();
	}

	template<typename LogPolicy>
	inline void Logger<LogPolicy>::SetThreadName(const std::string & name)
	{
		m_threadName[std::this_thread::get_id()] = name;
	}

	template<typename LogPolicy>
	template<SeverityType severity>
	inline void Logger<LogPolicy>::Print(std::stringstream stream)
	{
		std::stringstream logStream;

		// Get time
		SYSTEMTIME localTime;
		GetLocalTime(&localTime);

		// Header content: line number and date (x: xx/xx/xx xx:xx:xx)
		if (m_logLineNumber != 0) {
			logStream << "\r\n";
		}
		logStream << m_logLineNumber++ << ": "
			<< localTime.wDay << "/"
			<< localTime.wMonth << "/"
			<< localTime.wYear << " "
			<< localTime.wHour << ":"
			<< localTime.wMinute << ":"
			<< localTime.wSecond << "\t";

		// Specify warning lvl
		switch (severity)
		{
		case SeverityType::Info:
			logStream << "INFO:     ";
			break;
		case SeverityType::Debug:
			logStream << "DEBUG:     ";
			break;
		case SeverityType::Warning:
			logStream << "WARNING:     ";
			break;
		case SeverityType::Error:
			logStream << "ERROR:     ";
			break;
		}

		// Log thread name
		logStream << m_threadName[std::this_thread::get_id()] << ":\t";

		// Log actual msg
		logStream << stream.str();
		std::lock_guard<std::timed_mutex> lock(m_writeMutex);
		m_logBuffer.push_back(logStream.str());
	}

	template<typename LogPolicy>
	template<SeverityType severity>
	inline void Logger<LogPolicy>::Print(std::string msg)
	{
		std::stringstream stream;
		stream << msg.c_str();
		this->Print<severity>(std::stringstream(stream.str()));
	}

}