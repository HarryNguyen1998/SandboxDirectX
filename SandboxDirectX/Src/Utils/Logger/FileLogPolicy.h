#pragma once
#include "ILogPolicy.h"

// STL
#include <fstream>

namespace Util
{
	class FileLogPolicy
		: public ILogPolicy
	{
	private:
		std::ofstream m_output;

	public:
		FileLogPolicy() : m_output() {};
		~FileLogPolicy() {};

		bool OpenOutputStream(const std::wstring& fileName) override;
		void CloseOutputStream() override;
		void Write(const std::string& msg) override;
	};

}