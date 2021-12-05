#include "../includes/statistics.h"

#include <fstream>

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

/*
        Public methods
*/

Statistic::Statistic(int period, int number)
{
        this->periodLength = period;
        this->numberOfPeriods = number;
        this->smoothingCoeff = 0;
        this->windowSize = 0;
        this->sensitivity = 30;
        this->mainCentralLine = 0;
        this->standartDeviation = 0;
        this->hardLimit = 0;

        this->smoothedValue = 0;

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

        for (int i = 0; i < RESULT_TRAINING_TIME; i++)
        {
                timer.setTimeCutoff(STATISTIC_CUTOFF_F);
        }

        message(NOTICE_M, "Training will start in " + std::to_string(RESULT_TRAINING_TIME + MAIN_STEP / 1000) + " seconds ...");

        std::this_thread::sleep_until(timer.getTimeCutoff(STATISTIC_CUTOFF_F));
        timer.setTimeCutoff(STATISTIC_CUTOFF_F);

        message(NOTICE_M, "Start of training ...");
        mode = MODE_TRAINING;

        std::vector<long long> fullCapture;
        std::vector<long long> differentiatedCapture;

        std::vector<long long> capture;
        std::string str = "";

        std::fstream file("debug_files/traffic.txt", std::ios::in);
        std::string s = "";
        message(NOTICE_M, "DEBUG: open debug file");

        // message(NOTICE_M, "DEBUG: setting timer to 10 sec");
        // for (int i = 0; i < 10; i++)
        // {
        //         std::this_thread::sleep_until(timer.getTimeCutoff(STATISTIC_CUTOFF_F));
        // }

        for (int i = 0; i < this->numberOfPeriods; i++)
        {

                if (interruptFlag)
                {
                        message(NOTICE_M, "Training completed");
                        mode = MODE_DEFAULT;
                        return;
                }

                message(NOTICE_M, "Start of " + std::to_string(i + 1) + " capture out of " + std::to_string(this->numberOfPeriods) +
                                      " (" + secondsToString(this->periodLength) + ") ...");

                for (int j = 0; j < this->periodLength; j++)
                {
                        if (interruptFlag)
                        {
                                message(NOTICE_M, "Training completed");
                                mode = MODE_DEFAULT;
                                return;
                        }

                        if (std::getline(file, s))
                        {
                                capture.push_back(atoi(s.c_str()));
                                fullCapture.push_back(atoi(s.c_str()));
                        }

                        // long long bytes = sniffer.getTrafficPerSec();
                        // capture.push_back(bytes);
                        // fullCapture.push_back(bytes);
                }

                message(NOTICE_M, std::to_string(i + 1) + " capture out of " + std::to_string(this->numberOfPeriods) + " completed");

                differentiatedCapture.push_back(this->averaging(capture));

                capture.clear();
        }

        file.close();
        message(NOTICE_M, "DEBUG: close debug file");

        this->mainCentralLine = this->averaging(fullCapture);

        str = bytesToString(mainCentralLine);
        message(NOTICE_M, "Central line: " + str);

        this->smoothingCoeffCalculation(differentiatedCapture);

        str = std::to_string(this->smoothingCoeff);
        str = str.substr(0, str.size() - 4);
        message(NOTICE_M, "Smoothing coeffitient: " + str);

        if (this->smoothingCoeff > 1 || this->smoothingCoeff <= 0)
        {
                message(ERROR_M, "invalid value of smoothing coeffitient");
                status = STATUS_ERROR;
                std::raise(SIGINT);
        }
        else if (this->smoothingCoeff >= 0.5)
        {
                message(WARNING_M, "incorrect calculations are possible, repeat the capture");
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

        this->k = this->windowSize * 0.7;

        str.clear();

        for (size_t i = 0; i < fullCapture.size(); i++)
        {
                this->standartDeviation += pow(fullCapture[i] - this->mainCentralLine, 2);
        }
        this->standartDeviation /= (float)(fullCapture.size());

        this->hardLimit = this->mainCentralLine + 5 * sqrt((this->smoothingCoeff / (2 - this->smoothingCoeff)) * this->standartDeviation);
        message(NOTICE_M, "Hard limit: " + std::to_string(this->hardLimit));

        message(NOTICE_M, "Training completed");
        mode = MODE_DEFAULT;

        return;
}

void Statistic::detection(Timer &timer, Sniffer &sniffer)
{
        for (int i = 0; i < RESULT_TRAINING_TIME; i++)
        {
                timer.setTimeCutoff(STATISTIC_CUTOFF_F);
        }

        message(NOTICE_M, "Detection will start in " + std::to_string(RESULT_TRAINING_TIME + MAIN_STEP / 1000) + " seconds ...");

        // std::this_thread::sleep_until(timer.getTimeCutoff(STATISTIC_CUTOFF_F));
        // timer.setTimeCutoff(STATISTIC_CUTOFF_F);

        message(NOTICE_M, "Start of detection ...");
        mode = MODE_DETECTION;

        long long previousSmoothedValue = sniffer.getTrafficPerSec();
        this->smoothedValue = sniffer.getTrafficPerSec();
        long long trafficPerSec = 0;

        long long windowCentralLine = this->mainCentralLine;
        std::vector<long long> window;
        
        bool interruption = false;
        int count = 0;
        int excessCount = 0;
        
        std::fstream file("debug_files/traffic.txt", std::ios::in);
        std::string s = "";
        message(NOTICE_M, "DEBUG: open debug file");
        long long iterCount = 0;

        long long uclExcessCount = 0;
        long long floatExcessCount = 0;

        while (!interruption)
        {
                if (interruptFlag || iterCount == (this->numberOfPeriods * this->periodLength))
                {
                        interruption = !interruption;
                }

                if (std::getline(file, s))
                {
                        trafficPerSec = atoi(s.c_str());
                        window.push_back(trafficPerSec);
                }

                previousSmoothedValue = this->smoothedValue;
                this->smoothedValue = this->smoothing(this->smoothingCoeff, trafficPerSec, previousSmoothedValue);

                count++;
                if (count == this->windowSize)
                {
                        windowCentralLine = this->averaging(window);

                        window.clear();
                        count = 0;
                }

                if (trafficPerSec > windowCentralLine * ((this->sensitivity / 100) + 1))
                {
                        excessCount++;
                }
                else
                {
                        excessCount = 0;
                }

                if (trafficPerSec > this->hardLimit || excessCount == this->k)
                {
                        uclExcessCount++;
                        status = STATUS_ALARM;
                }
                else if (excessCount == this->k)
                {
                        floatExcessCount++;
                        status = STATUS_ALARM;
                }
                else
                {
                        status = STATUS_OK;
                }

                iterCount++;
        }

        file.close();
        message(NOTICE_M, "DEBUG: close debug file");

        message(NOTICE_M, "Detection completed");
        mode = MODE_DEFAULT;

        message(NOTICE_M, "DEBUG: total values: " + std::to_string(this->numberOfPeriods * this->periodLength));
        message(NOTICE_M, "DEBUG: attacks detected: " + std::to_string(uclExcessCount) + " UCL exceeded");
        message(NOTICE_M, "DEBUG: attacks detected: " + std::to_string(floatExcessCount) + " float limit exceeded");

        float errPercent = (uclExcessCount + floatExcessCount) * 100 / (float)(this->numberOfPeriods * this->periodLength);
        message(NOTICE_M, "DEBUG: error percent: " + std::to_string(errPercent) + " %");

        interruptFlag = true;

        return;
}

long long Statistic::getCentralLine()
{
        return this->mainCentralLine;
}

float Statistic::getSmoothingCoeff()
{
        return this->smoothingCoeff;
}

int Statistic::getWindowSize()
{
        return this->windowSize;
}

long long Statistic::getHardLimit()
{
        return this->hardLimit;
}

long long Statistic::getSmoothingValue()
{
        return this->smoothedValue;
}