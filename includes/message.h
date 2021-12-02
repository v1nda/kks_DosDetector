#ifndef MESSAGE_H
#define MESSAGE_H

#include <ctime>
#include <vector>
#include <iostream>
#include <fstream>
#include <mutex>

#include "../includes/global.h"

#define USAGE_M "USAGE"
#define NOTICE_M "NOTICE"
#define ERROR_M "ERROR"
#define WARNING_M "WARNING"
#define ALARM_M "ALARM"

void message(std::string type, std::string text);
std::vector<std::string> getLastMessages(int number);

class Message
{
private:
        std::string text;
        std::string time;
        std::string type;

        std::string messageString;

        void messageToString();
        void printToCli();
        void printToCgui();
        void printToLog();

public:
        Message(std::string type, std::string text);
        ~Message();
};

#endif