#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <iterator>

#include <locale>
#include <codecvt>


namespace Sand
{
	extern void StringToWString(std::string* str, std::wstring* wstr);
	extern std::wstring StringToWString(std::string* str);

	extern void WStringToString(std::wstring* wstr, std::string* str);
	extern std::string WStringToString(std::wstring* wstr);

	extern bool Equal(const double& a, const double& b);

	template <typename... T>
	void ConsoleLog(std::wstring message, T&&... params)
	{
		wchar_t buffer[512];
		swprintf(buffer, 512, message.c_str(), std::forward<T>(params)...);
#if __WINDOWS__
		OutputDebugString(buffer);
#endif
	}
}