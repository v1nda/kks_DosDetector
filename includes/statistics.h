#ifndef STATISTIC_H
#define STATISTIC_H

#include "../includes/sniffer.h"

#define PERCENTAGE_FOR_EXCESS 0.7

#define WAIT_TRAINING_TIME 4

class Statistic
{
private:
        int periodLength;
        int numberOfPeriods;
        float smoothingCoeff;
        int windowSize;
        int sensitivity;
        int numberOfExcesses;
        long long centralLine;
        long long standartDeviation;
        long long limit;

        long long smoothed;

        long long averaging(std::vector<long long> &capture);
        long long smoothing(float smoothingCoeff, long long secondValue, long long firstValue);
        void smoothingCoeffCalculation(std::vector<long long> &capture);

public:
        Statistic(int periodLength, int numberOfPeriods);
        ~Statistic();

        void training(Timer &timer, Sniffer &sniffer);
        void detection(Timer &timer, Sniffer &sniffer);

        float getSmoothingCoeff();
        int getWindowSize();
        long long getLimit();

        long long getSmoothingValue();
};

#endif