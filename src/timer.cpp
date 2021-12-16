#include "../includes/timer.h"

static std::mutex cutoffMutex;

/*
        Non-class function
*/

std::string secondsToString(int seconds)
{
        char buffer[18];

        std::time_t now = std::time(NULL) - std::time(NULL);
        std::tm *time = std::localtime(&now);
        time->tm_mday -= 1;
        time->tm_mon -= 1;
        time->tm_year -= 70;
        time->tm_hour -= 3;

        if (seconds / 60 > 0)
        {
                if (seconds / 3600 > 0)
                {
                        time->tm_hour = seconds / 3600;
                        seconds %= 3600;
                }
                time->tm_min = seconds / 60;
                seconds %= 60;
        }
        time->tm_sec = seconds;

        std::strftime(buffer, 18, "%H:%M:%S", time);

        return buffer;
}

/*
        Public methods
*/

Timer::Timer()
{
        std::chrono::milliseconds shift(THREAD_SHIFT);
        this->timeStartProgram = std::chrono::steady_clock::now() + shift;
        this->timeSystemStartProgram = std::time(nullptr);

        message(NOTICE_M, "Timer: start of timer");

        for (int i = 0; i < NUMBER_CUTOFF_F; i++)
        {
                std::chrono::milliseconds shift(i * TRIGGER_SHIFT);
                this->timeCutoff[i] = this->timeStartProgram + shift;
        }

        message(NOTICE_M, "Timer: initialisation completed");

        return;
}

Timer::~Timer()
{
        message(NOTICE_M, "Timer: working time: " + this->getWorkTime());
        message(NOTICE_M, "Timer: end of timer");

        return;
}

void Timer::setTimeStartSniffing()
{
        for (int i = 0; i < NUMBER_CUTOFF_F; i++)
        {
                std::chrono::milliseconds shift(i * THREAD_SHIFT);
                this->timeCutoff[i] += shift;
        }

        return;
}

void Timer::setTimeCutoff(int flag)
{
        cutoffMutex.lock();

        std::chrono::milliseconds step(MAIN_STEP);
        this->timeCutoff[flag] += step;

        cutoffMutex.unlock();

        return;
}

std::chrono::time_point<std::chrono::steady_clock> Timer::getTimeCutoff(int flag)
{
        return this->timeCutoff[flag];
}

std::string Timer::getWorkTime()
{
        std::string str;

        auto duration = std::chrono::steady_clock::now() - this->timeStartProgram;

        return secondsToString(duration.count() / pow(10, 9));
}

std::string Timer::getCurrentTime()
{
        char buffer[18];

        std::time_t now = std::time(NULL);
        std::strftime(buffer, 18, "%d.%m.%y %H:%M:%S", std::localtime(&now));

        return buffer;
}