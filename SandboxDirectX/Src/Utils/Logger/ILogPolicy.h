#pragma once
#include <string>

class ILogPolicy
{
public:
	virtual ~ILogPolicy() noexcept = default;

	virtual bool OpenOutputStream(const std::wstring& name) = 0;
	virtual void CloseOutputStream() = 0;
	virtual void Write(const std::string& msg) = 0;
};
