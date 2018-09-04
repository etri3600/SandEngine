#include "SUtils.h"

namespace Sand
{
	void StringToWString(std::string* str, std::wstring* wstr)
	{
		*wstr = StringToWString(str);
	}

	std::wstring StringToWString(std::string* str)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
		return converter.from_bytes(*str);
	}

	void WStringToString(std::wstring* wstr, std::string* str)
	{
		*str = WStringToString(wstr);
	}
	
	std::string WStringToString(std::wstring* wstr)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
		return converter.to_bytes(*wstr);
	}

	bool Equal(const double& a, const double& b)
	{
#define EPSILON (1.e-6)
		return fabs(a - b) < EPSILON;
	}

}