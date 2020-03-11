// Utils
#include "Utils/ServiceLocator.h"

// STL
#include <exception>
#include <stdexcept>

// Windows
#include <windows.h>

void StartLoggingService()
{
	// Create file logger
	std::shared_ptr<Util::Logger<Util::FileLogPolicy> > engineLogger(new Util::Logger<Util::FileLogPolicy>(L"Engine.log"));

	// Set name of current thread
	engineLogger->SetThreadName("MainThread");

	// Register the logging service
	Util::ServiceLocator::ProvideFileLoggerService(engineLogger);

#ifndef NDEBUG
	Util::ServiceLocator::GetFileLogger()->Print<Util::SeverityType::Info>("The file was created successfully!");
#endif
}

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	try {
		StartLoggingService();
	}
	catch (std::runtime_error) {
		MessageBox(nullptr, "Unable to start Logging service!", "Critical Error!", MB_ICONERROR | MB_OK);

		return -1;
	}


	return 0;
}