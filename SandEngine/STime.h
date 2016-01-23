#pragma once

#include <ctime>
#include <chrono>

class STime
{
public:
	///  Get Current Time In MilliSeconds
	static long long GetTime() {
		const auto now = std::chrono::steady_clock::now();
		auto epochTime = now.time_since_epoch();
		return std::chrono::duration_cast<std::chrono::milliseconds>(epochTime).count();
	}
};