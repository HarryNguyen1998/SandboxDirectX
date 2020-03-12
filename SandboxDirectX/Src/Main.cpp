// Utils
#include "Utils/Logger/ServiceLocator.h"
#include "Utils/Expected/Expected.h"
// STL
#include <exception>
#include <stdexcept>
#include <string>

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

Util::Expected<int> Nani(int x) 
{
	if (x == 10) {
		return Util::Expected<int>::fromException(
			std::invalid_argument("I hate number ten"));
	}
	else {
		Util::Expected<int> res(x);
		return res;
	}
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
	int x = 7;
	Util::Expected<int> e = Nani(x);
	if (e.isValid()) {
		std::string s = "The value of e: " + std::to_string(e.get());
		MessageBox(nullptr, s.c_str(), "Value", MB_OK);
	}
	else {
		MessageBox(nullptr, "e is not created", "Critical Error!", MB_ICONERROR | MB_OK);
	}
	return 0;
}