#include "../includes/statistics.h"

#include <stdlib.h>
#include <algorithm>

#include <fstream>

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

void Statistic::anomalyChecking(long long excesses, long long traffic)
{
        if (excesses >= this->numberOfExcesses)
        {
                if (!this->fixingLimit)
                {
                        this->fixingLimit = true;
                }

                this->excessSeconds++;
                if (traffic > this->maxAnomalyTraffic)
                {
                        this->maxAnomalyTraffic = traffic;
                }
                this->averageAnomalyTraffic += traffic;

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
                if (status == STATUS_ALARM || status == STATUS_WARNING)
                {
                        message(NOTICE_M, "Limit released");

                        message(NOTICE_M, "Duration of anomaly: " + secondsToString(this->excessSeconds));

                        message(NOTICE_M, "Max anomaly traffic: " + bytesToString(this->maxAnomalyTraffic) + "/sec");

                        this->averageAnomalyTraffic /= this->excessSeconds;
                        message(NOTICE_M, "Average anomaly traffic: " + bytesToString(this->averageAnomalyTraffic) + "/sec");
                }

                this->fixingLimit = false;

                if (status == STATUS_ALARM && this->excessSeconds > this->maxExcessSeconds)
                {
                        this->maxExcessSeconds = this->excessSeconds;
                }

                this->excessSeconds = 0;
                this->maxAnomalyTraffic = 0;
                this->averageAnomalyTraffic = 0;
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

Statistic::Statistic(int period, int number, long long warning, long long alarm, bool analysis)
{
        this->periodLength = period;
        this->numberOfPeriods = number;
        this->analysisFlag = analysis;
        this->warningTime = warning;
        this->alarmTime = alarm;

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
        this->maxAnomalyTraffic = 0;
        this->averageAnomalyTraffic = 0;

        message(NOTICE_M, "Statistic: initialization completed");

        return;
}

Statistic::~Statistic()
{
        message(NOTICE_M, "Statistic: end of statistic");

        return;
}

void Statistic::analysis(Timer &timer, std::vector<long long> &trainingCapture)
{
        message(NOTICE_M, "Start of analysing capture ...");

        if ((int)trainingCapture.size() <= this->windowSize + 1)
        {
                status = STATUS_WARNING;
                message(WARNING_M, "Training capture size is too small. Using defaults values");
                message(WARNING_M, "Analisys interrupted");
                return;
        }

        std::this_thread::sleep_until(timer.getTimeCutoff(STATISTIC_CUTOFF_F));

        for (int i = 0; i < ANALISYS_TIME; i++)
        {
                timer.setTimeCutoff(STATISTIC_CUTOFF_F);
        }

        
        std::vector<long long> anomalyDurings;

        long long trafficPerSec = trainingCapture[0];
        long long previousSmoothedValue = trafficPerSec;
        this->smoothed = trafficPerSec;

        std::vector<long long> window;
        long long centralLine = 0;
        
        int windowCount = 0;
        int excessCount = 0;

        for (int i = 1; i < this->windowSize + 1; i++)
        {
                trafficPerSec = trainingCapture[i];
                window.push_back(trafficPerSec);

                previousSmoothedValue = this->smoothed;
                this->smoothed = this->smoothing(this->smoothingCoeff, trafficPerSec, previousSmoothedValue);
        }

        centralLine = this->averaging(window);
        this->limit = centralLine + 5 * sqrt((this->smoothingCoeff / (2 - this->smoothingCoeff)) * this->standartDeviation);
        window.clear();

        for (size_t i = this->windowSize + 1; i < trainingCapture.size(); i++)
        {
                trafficPerSec = trainingCapture[i];
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

                if (excessCount >= this->numberOfExcesses)
                {
                        if (!this->fixingLimit)
                        {
                                this->fixingLimit = true;
                        }

                        this->excessSeconds++;
                }
                else if (excessCount < this->numberOfExcesses && this->fixingLimit)
                {
                        this->fixingLimit = false;
                        anomalyDurings.push_back(this->excessSeconds);
                        this->excessSeconds = 0;
                }
        }

        std::sort(anomalyDurings.begin(), anomalyDurings.end());

        long long warning = 0;
        long long alarm = 0;
        if (anomalyDurings.size() == 1)
        {
                warning = (int)round(anomalyDurings[0] * WARNING_DURING);
                alarm = anomalyDurings[0];
        }
        else if (anomalyDurings.size() > 1)
        {
                warning = anomalyDurings[(int)round(anomalyDurings.size() * 0.6) - 1];
                alarm = anomalyDurings[anomalyDurings.size() - 1];
        }

        if (anomalyDurings.size() == 0)
        {
                message(NOTICE_M, "No anomalys found. Using default values");
        }
        else
        {
                if (warning > 60)
                {
                        this->warningTime = 60 - (warning % 60) + warning;
                }
                else if (warning % 60 == 0)
                {
                        this->warningTime = warning;
                }
                else
                {
                        this->warningTime = 10 - (warning % 10) + warning;
                }

                if (alarm > 60)
                {
                        this->alarmTime = 60 - (alarm % 60) + alarm;
                }
                else if (alarm % 60 == 0)
                {
                        this->alarmTime = alarm;
                }
                else
                {
                        this->alarmTime = 10 - (alarm % 10) + alarm;
                }
        }

        message(NOTICE_M, "Warning reaction time: " + secondsToString(this->warningTime));
        message(NOTICE_M, "Alarm reaction time: " + secondsToString(this->alarmTime));

        this->excessSeconds = 0;
        this->fixingLimit = false;

        message(NOTICE_M, "Analisys complete");

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

        std::fstream file("test/src/training_dump.txt", std::ios::in);
        std::string s = "";
        message(NOTICE_M, "TEST: open traffic file");
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

                        if (std::getline(file, s))
                        {
                                capture.push_back(atoi(s.c_str()));
                                fullCapture.push_back(atoi(s.c_str()));
                        }
                }

                message(NOTICE_M, std::to_string(i + 1) + " capture out of " + std::to_string(this->numberOfPeriods) + " completed");

                averagingCapture.push_back(this->averaging(capture));

                capture.clear();
        }

        file.close();
        message(NOTICE_M, "TEST: close traffic file");
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

        if (this->analysisFlag)
        {
                this->analysis(timer, fullCapture);
        }

        message(NOTICE_M, "Training completed");
        mode = MODE_DEFAULT;

        return;
}

void Statistic::detection(Timer &timer, Sniffer &sniffer)
{
        std::this_thread::sleep_until(timer.getTimeCutoff(STATISTIC_CUTOFF_F));

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

                anomalyChecking(excessCount, trafficPerSec);
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