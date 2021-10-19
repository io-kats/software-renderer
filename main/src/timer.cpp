#include "timer.h"

Timer::Timer() : m_accumulatedTime(0.0), m_counter(0) { }

void Timer::Begin()
{
    m_begin = clock_type::now();
}

void Timer::End()
{
    m_end = clock_type::now();
    std::chrono::duration<f64> elapsed_time = m_end - m_begin;	
    m_accumulatedTime += elapsed_time.count();
    ++m_counter;
}

void Timer::Reset()
{
    m_accumulatedTime = 0.0;
    m_counter = 0;
}

void Timer::Log(const char* msg)
{
    printf("*************************\n");
    printf("%s\n", msg);
    printf("Mean time: %.18g s\n", GetMean());
    printf("*************************\n\n");
}

f64 Timer::GetAccumulated()
{
    return m_accumulatedTime;
}

f64 Timer::GetMean()
{
    return m_accumulatedTime / (double) m_counter;
}