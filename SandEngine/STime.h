#pragma once

#include <ctime>
#include <chrono>

class STime
{
public:
	static long long GetTime() {
		const auto now = std::chrono::system_clock::now();
		return now.time_since_epoch().count();
	}
};