#pragma once

#include <vector>
#include <map>
#include <iostream>
#include <iterator>

#include <locale>
#include <codecvt>

namespace Sand
{
	extern void string_to_wstring(std::string* str, std::wstring* wstr);
	extern std::wstring string_to_wstring(std::string* str);

	extern void wstring_to_string(std::wstring* wstr, std::string* str);
	extern std::string wstring_to_string(std::wstring* wstr);

	extern void PlatformPath(std::wstring& wstr);

	extern bool Equal(const double& a, const double& b);

	template <typename... T>
	void ConsoleLog(std::wstring message, T&&... params)
	{
		wchar_t buffer[512];
		swprintf(buffer, 512, message.c_str(), std::forward<T>(params)...);
		std::clog << buffer << std::endl;
	}
}