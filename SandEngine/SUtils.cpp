#include "SUtils.h"

#include <cmath>
#include <algorithm>

namespace Sand
{
	void string_to_wstring(std::string* str, std::wstring* wstr)
	{
		*wstr = string_to_wstring(str);
	}

	std::wstring string_to_wstring(std::string* str)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
		return converter.from_bytes(*str);
	}

	void wstring_to_string(std::wstring* wstr, std::string* str)
	{
		*str = wstring_to_string(wstr);
	}
	
	std::string wstring_to_string(std::wstring* wstr)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
		return converter.to_bytes(*wstr);
	}

	void PlatformPath(std::wstring& wstr)
	{
#if __WINDOWS__
		std::replace(wstr.begin(), wstr.end(), L'/', L'\\');
#endif
	}

	bool Equal(const double& a, const double& b)
	{
#define EPSILON (1.e-6)
		return std::abs(a - b) < EPSILON;
	}

}