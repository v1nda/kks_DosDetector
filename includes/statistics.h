#ifndef STATISTIC_H
#define STATISTIC_H

#include "../includes/sniffer.h"

#define PERCENTAGE_FOR_EXCESS 0.8

#define SNIFFING_STATUS "--"
#define TRAINING_CAPTURE_STATUS "capturing"
#define TRAINING_CALCULATING_STATUS "calculating"

#define RESULT_TRAINING_TIME 4

class Statistic
{
private:
        int periodLength;
        int numberOfPeriods;
        float smoothingCoeff;
        int windowSize;
        int sensitivity;
        int k;
        long long mainCentralLine;
        long long standartDeviation;
        long long hardLimit;

        long long smoothedValue;

        long long averaging(std::vector<long long> &capture);
        long long smoothing(float smoothingCoeff, long long secondValue, long long firstValue);
        void smoothingCoeffCalculation(std::vector<long long> &capture);

public:
        Statistic(int periodLength, int numberOfPeriods);
        ~Statistic();

        void training(Timer &timer, Sniffer &sniffer);
        void detection(Timer &timer, Sniffer &sniffer);

        long long getCentralLine();
        float getSmoothingCoeff();
        int getWindowSize();
        long long getHardLimit();

        long long getSmoothingValue();
};

#endif