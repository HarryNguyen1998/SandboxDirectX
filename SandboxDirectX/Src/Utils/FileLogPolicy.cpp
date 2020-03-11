#include "FileLogPolicy.h"

// STL
#include <string>
#include <sstream>

// WinAPI
#include <ShlObj_core.h>

namespace Util
{
	bool FileLogPolicy::OpenOutputStream(const std::wstring& fileName)
	{
		PWSTR docPath = nullptr;
		SHGetKnownFolderPath(FOLDERID_Documents, NULL, nullptr, &docPath);

		// Specify path and create (if not exists)
		std::wstringstream path;
		path << docPath << L"\\SandboxDirectX\\logs\\";

		// Don't need docPath anymore, free to avoid mem leak.
		::CoTaskMemFree(static_cast<void*>(docPath));

		SHCreateDirectory(nullptr, path.str().c_str());

		// Append filename, then open (create if not exists) & set output precision.
		path << fileName.c_str();
		m_output.open(path.str().c_str(), std::ios_base::binary | std::ios_base::out);
		m_output.precision(20);

		return true;
	}

	void FileLogPolicy::CloseOutputStream()
	{
		m_output.close();
	}

	void FileLogPolicy::Write(const std::string& msg)
	{
		m_output << msg << std::endl;
	}
}