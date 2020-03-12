#include "ServiceLocator.h"

namespace Util
{
	std::shared_ptr<Logger<FileLogPolicy> > ServiceLocator::s_fileLogger = nullptr;
	void ServiceLocator::ProvideFileLoggerService(std::shared_ptr<Logger<FileLogPolicy> > providedFileLogger)
	{
		s_fileLogger = providedFileLogger;
	}
}