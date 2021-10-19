#ifndef TIMER_H
#define TIMER_H

#include "ers/typedefs.h"
#include "ers/macros.h"
#include <chrono>

#define TIMER_LOG(timer, msg) for (size_t timer ## _counter = 0; timer ## _counter < n; ++timer ## _counter) {
#define TIMER_LOOP_BEGIN(timer, n) for (size_t timer ## _counter = 0; timer ## _counter < n; ++timer ## _counter) {
#define TIMER_LOOP_END() }

class Timer
{
private:
	typedef std::conditional<
       std::chrono::high_resolution_clock::is_steady,
       std::chrono::high_resolution_clock,
       std::chrono::steady_clock >::type clock_type;
	static_assert(std::chrono::steady_clock::is_steady, "Steady clock not steady.");

public:
	Timer();

	void Begin();
	void End();
	void Reset();
	void Log(const char* msg);
	f64 GetAccumulated();
	f64 GetMean();

private:
	f64 m_accumulatedTime;
	size_t m_counter;
	std::chrono::time_point<clock_type> m_begin;
	std::chrono::time_point<clock_type> m_end;
};

#endif // TIMER_H