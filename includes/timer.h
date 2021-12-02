#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <cmath>

#include "../includes/message.h"

#define NUMBER_CUTOFF_F 4
#define CLI_CUTOFF_F 0
#define SNIFFER_CUTOFF_F 1
#define TRAFFIC_COUNTER_CUTOFF_F 2
#define STATISTIC_CUTOFF_F 3

#define MAIN_STEP 1000
#define THREAD_SHIFT 100
#define TRIGGER_SHIFT 1000

std::string secondsToString(int seconds);

class Timer
{
private:
        std::time_t timeSystemStartProgram;
        std::chrono::time_point<std::chrono::steady_clock> timeCutoff[NUMBER_CUTOFF_F];

        std::string mode;
        std::string status;

public:
        Timer();
        ~Timer();
        std::chrono::time_point<std::chrono::steady_clock> timeStartProgram;

        void setTimeStartSniffing();
        void setTimeCutoff(int flag);
        std::chrono::time_point<std::chrono::steady_clock> getTimeCutoff(int flag);
        std::string getWorkTime();
        std::string getCurrentTime();
};

#endif