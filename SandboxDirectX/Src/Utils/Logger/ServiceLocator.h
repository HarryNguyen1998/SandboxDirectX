#pragma once
#include "FileLogPolicy.h"
#include "Logger.h"

// STL
#include <memory>

namespace Util
{
	class ServiceLocator
	{
	private:
		static std::shared_ptr<Logger<FileLogPolicy> > s_fileLogger;
	public:
		static Logger<FileLogPolicy>* GetFileLogger() { return s_fileLogger.get(); };
		static void ProvideFileLoggerService(std::shared_ptr<Logger<FileLogPolicy> > providedFileLogger);
	};

}