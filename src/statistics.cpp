#include "../includes/statistics.h"

#include <stdlib.h>

std::string mode = MODE_DEFAULT;
std::string status = STATUS_OK;

/*
        Private methods
*/

long long Statistic::averaging(std::vector<long long> &capture)
{
        long long sum = 0;

        for (size_t i = 0; i < capture.size(); i++)
                sum += capture[i];

        return sum / capture.size();
}

long long Statistic::smoothing(float coeff, long long secondValue, long long firstValue)
{
        return (long long)floor(((1 - coeff) * firstValue) + (coeff * secondValue));
}

void Statistic::smoothingCoeffCalculation(std::vector<long long> &capture)
{
        float min = -1;

        for (int j = 1; j < 100; j++)
        {
                float lambda = (float)j / (float)100;
                long long smoothedValues[capture.size() - 1];
                long long sum = 0;
                float ase = 0;

                smoothedValues[0] = capture[0];

                for (size_t i = 1; i < capture.size() - 1; i++)
                {
                        smoothedValues[i] = this->smoothing(lambda, capture[i], smoothedValues[i - 1]);
                }

                for (size_t i = 0; i < capture.size() - 1; i++)
                {
                        sum += pow(capture[i + 1] - smoothedValues[i], 2);
                }

                ase = sum / (float)(capture.size() - 1);
                if (min == -1)
                {
                        min = ase;
                        this->smoothingCoeff = lambda;
                }
                if (min > ase)
                {
                        min = ase;
                        this->smoothingCoeff = lambda;
                }
        }

        return;
}

void Statistic::anomalyChecking(long long excesses)
{
        if (excesses >= this->numberOfExcesses)
        {
                if (!this->fixingLimit)
                {
                        this->fixingLimit = true;
                }

                this->excessSeconds++;

                if (status != STATUS_ALARM && this->excessSeconds >= this->alarmTime)
                {
                        status = STATUS_ALARM;

                        message(ALARM_M, "Time of exceeding a fixed limit: " + secondsToString(this->excessSeconds));

                        this->anomalysCount++;
                }
                else if (status != STATUS_WARNING && this->excessSeconds >= this->warningTime && this->excessSeconds < this->alarmTime)
                {
                        status = STATUS_WARNING;

                        message(WARNING_M, "Limit is fixed: " + bytesToString(this->limit));
                        message(WARNING_M, "Time of exceeding a fixed limit: " + secondsToString(this->excessSeconds));
                }
                else if (this->excessSeconds < this->warningTime)
                {
                        status = STATUS_OK;
                }
        }
        else if (excesses < this->numberOfExcesses && this->fixingLimit)
        {
                if (status == STATUS_ALARM)
                {
                        message(NOTICE_M, "Limit released");
                        message(NOTICE_M, "Duration of anomaly: " + secondsToString(this->excessSeconds));
                }
                else if (status == STATUS_WARNING)
                {
                        message(NOTICE_M, "Limit released");
                }

                this->fixingLimit = false;

                if (status == STATUS_ALARM && this->excessSeconds > this->maxExcessSeconds)
                {
                        this->maxExcessSeconds = this->excessSeconds;
                }
                this->excessSeconds = 0;
                status = STATUS_OK;
        }
        else
        {
                status = STATUS_OK;
        }
}

/*
        Public methods
*/

Statistic::Statistic(int period, int number)
{
        this->periodLength = period;
        this->numberOfPeriods = number;
        this->smoothingCoeff = 0;
        this->windowSize = 0;
        this->numberOfExcesses = 0;
        this->standartDeviation = 0;
        this->limit = 0;

        this->smoothed = 0;

        this->anomalysCount = 0;
        this->fixingLimit = false;
        this->excessSeconds = 0;
        this->maxExcessSeconds = 0;

        this->warningTime = 10;
        this->alarmTime = 60;

        message(NOTICE_M, "Statistic: initialization completed");

        return;
}

Statistic::~Statistic()
{
        message(NOTICE_M, "Statistic: end of statistic");

        return;
}

void Statistic::training(Timer &timer, Sniffer &sniffer)
{
        std::this_thread::sleep_until(timer.getTimeCutoff(STATISTIC_CUTOFF_F));
        timer.setTimeCutoff(STATISTIC_CUTOFF_F);

        for (int i = 0; i < WAIT_TRAINING_TIME; i++)
        {
                timer.setTimeCutoff(STATISTIC_CUTOFF_F);
        }

        message(NOTICE_M, "Training will start in " + std::to_string(WAIT_TRAINING_TIME + MAIN_STEP / 1000) + " seconds ...");

        std::this_thread::sleep_until(timer.getTimeCutoff(STATISTIC_CUTOFF_F));
        timer.setTimeCutoff(STATISTIC_CUTOFF_F);

        message(NOTICE_M, "Start of training ...");
        mode = MODE_TRAINING;

        std::vector<long long> fullCapture;
        std::vector<long long> averagingCapture;

        std::vector<long long> capture;
        std::string str = "--";

        for (int i = 0; i < this->numberOfPeriods; i++)
        {
                message(NOTICE_M, "Start of " + std::to_string(i + 1) + " capture out of " + std::to_string(this->numberOfPeriods) + " (" + secondsToString(this->periodLength) + ") ...");

                for (int j = 0; j < this->periodLength; j++)
                {
                        if (interrupt)
                        {
                                message(NOTICE_M, "Training interrupted");
                                mode = MODE_DEFAULT;
                                return;
                        }

                        std::this_thread::sleep_until(timer.getTimeCutoff(STATISTIC_CUTOFF_F));
                        timer.setTimeCutoff(STATISTIC_CUTOFF_F);

                        long long bytes = sniffer.getTrafficPerSec();
                        capture.push_back(bytes);
                        fullCapture.push_back(bytes);
                }

                message(NOTICE_M, std::to_string(i + 1) + " capture out of " + std::to_string(this->numberOfPeriods) + " completed");

                averagingCapture.push_back(this->averaging(capture));

                capture.clear();
        }

        long long centralLine = this->averaging(fullCapture);

        str = bytesToString(centralLine);
        message(NOTICE_M, "Central line: " + str);

        this->smoothingCoeffCalculation(averagingCapture);

        str = std::to_string(this->smoothingCoeff);
        str = str.substr(0, str.size() - 4);
        message(NOTICE_M, "Smoothing coeffitient: " + str);

        if (this->smoothingCoeff > 1 || this->smoothingCoeff <= 0)
        {
                message(ERROR_M, "Invalid value of smoothing coeffitient");
                status = STATUS_ERROR;
                std::raise(SIGINT);
        }
        else if (this->smoothingCoeff >= 0.5)
        {
                message(WARNING_M, "Incorrect calculations are possible, repeat the capture");
                status = STATUS_WARNING;
        }
        this->smoothingCoeff = 0.3;

        this->windowSize = (2 / (this->smoothingCoeff)) - 1;
        message(NOTICE_M, "Floating window size: " + std::to_string(this->windowSize));

        if (this->windowSize < 1)
        {
                message(ERROR_M, "invalid value of floating window size");
                status = STATUS_ERROR;
                std::raise(SIGINT);
        }

        this->numberOfExcesses = this->windowSize * PERCENTAGE_FOR_EXCESS;
        message(NOTICE_M, "Number of exceedances to determine the anomaly: " + std::to_string(this->numberOfExcesses));

        if (this->numberOfExcesses < 1)
        {
                message(ERROR_M, "Invalid value of number of exceedance ( < 1)");

                this->numberOfExcesses = 1;
                message(ERROR_M, "Number of exceedance set is set to: " + std::to_string(this->numberOfExcesses));
        }

        for (size_t i = 0; i < fullCapture.size(); i++)
        {
                this->standartDeviation += pow(fullCapture[i] - centralLine, 2);
        }
        this->standartDeviation /= (float)(fullCapture.size());

        message(NOTICE_M, "Training completed");
        mode = MODE_DEFAULT;

        return;
}

void Statistic::detection(Timer &timer, Sniffer &sniffer)
{
        for (int i = 0; i < WAIT_TRAINING_TIME; i++)
        {
                timer.setTimeCutoff(STATISTIC_CUTOFF_F);
        }

        message(NOTICE_M, "Detection will start in " + std::to_string(WAIT_TRAINING_TIME + MAIN_STEP / 1000) + " seconds ...");

        std::this_thread::sleep_until(timer.getTimeCutoff(STATISTIC_CUTOFF_F));
        timer.setTimeCutoff(STATISTIC_CUTOFF_F);

        message(NOTICE_M, "Start of detection ...");
        mode = MODE_DETECTION;

        long long trafficPerSec = sniffer.getTrafficPerSec();
        long long previousSmoothedValue = trafficPerSec;
        this->smoothed = trafficPerSec;

        std::vector<long long> window;
        long long centralLine = 0;
        
        int windowCount = 0;
        int excessCount = 0;

        for (int i = 0; i < this->windowSize; i++)
        {
                std::this_thread::sleep_until(timer.getTimeCutoff(STATISTIC_CUTOFF_F));
                timer.setTimeCutoff(STATISTIC_CUTOFF_F);

                trafficPerSec = sniffer.getTrafficPerSec();
                window.push_back(trafficPerSec);

                previousSmoothedValue = this->smoothed;
                this->smoothed = this->smoothing(this->smoothingCoeff, trafficPerSec, previousSmoothedValue);
        }

        centralLine = this->averaging(window);
        this->limit = centralLine + 5 * sqrt((this->smoothingCoeff / (2 - this->smoothingCoeff)) * this->standartDeviation);
        window.clear();

        while (!interrupt)
        {
                std::this_thread::sleep_until(timer.getTimeCutoff(STATISTIC_CUTOFF_F));
                timer.setTimeCutoff(STATISTIC_CUTOFF_F);

                trafficPerSec = sniffer.getTrafficPerSec();
                window.push_back(trafficPerSec);

                previousSmoothedValue = this->smoothed;
                this->smoothed = this->smoothing(this->smoothingCoeff, trafficPerSec, previousSmoothedValue);

                windowCount++;
                if (windowCount == this->windowSize)
                {
                        if (!this->fixingLimit)
                        {
                                centralLine = this->averaging(window);

                                this->limit = centralLine + 5 * sqrt((this->smoothingCoeff / (2 - this->smoothingCoeff)) * this->standartDeviation);
                        }

                        window.clear();
                        windowCount = 0;
                }

                if (this->smoothed > this->limit)
                {
                        excessCount++;
                }
                else
                {
                        excessCount = 0;
                }

                anomalyChecking(excessCount);
        }

        message(NOTICE_M, "Detection completed");
        mode = MODE_DEFAULT;

        message(NOTICE_M, "Number of recodered anomalys " + std::to_string(this->anomalysCount));

        return;
}

float Statistic::getSmoothingCoeff()
{
        return this->smoothingCoeff;
}

int Statistic::getWindowSize()
{
        return this->windowSize;
}

int Statistic::getNumberOfExcesses()
{
        return this->numberOfExcesses;
}

long long Statistic::getWarningTime()
{
        return this->warningTime;
}

long long Statistic::getAlarmTime()
{
        return this->alarmTime;
}

long long Statistic::getSmoothedValue()
{
        return this->smoothed;
}

long long Statistic::getLimit()
{
        return this->limit;
}

long long Statistic::getExcessSeconds()
{
        return this->excessSeconds;
}

std::string Statistic::getFixingLimit()
{
        if (this->fixingLimit)
        {
                return "YES";
        }
        else
        {
                return "NO";
        }

        return "<err>";
}

long long Statistic::getAnomalysCount()
{
        return this->anomalysCount;
}

long long Statistic::getMaxAnomalyTime()
{
        return this->maxExcessSeconds;
}