#include "../includes/message.h"

static std::mutex cliPrintMutex;
static std::mutex cguiPrintMutex;
static std::mutex logPrintMutex;

static std::vector<std::string> messages;

/*
        Non-class functions
*/

void message(std::string type, std::string text)
{
        Message mes(type, text);

        return;
}

std::vector<std::string> getLastMessages(int number)
{
        std::vector<std::string> lastMessages;

        for (int i = 0; i < number; i++)
        {
                if (i < (int)messages.size())
                {
                        lastMessages.push_back(messages[messages.size() - 1 - i]);
                }
                else
                {
                        lastMessages.push_back("");
                }
        }

        return lastMessages;
}

/*
        Private methods
*/

void Message::messageToString()
{
        this->messageString = "[" + this->time + "] ";

        if (this->type != NOTICE_M)
        {
                this->messageString = this->messageString + this->type + ": ";
        }
        this->messageString += this->text;

        return;
}

void Message::printToCli()
{
        cliPrintMutex.lock();

        std::cout << this->messageString << std::endl;

        cliPrintMutex.unlock();

        return;
}

void Message::printToCgui()
{
        cguiPrintMutex.lock();

        messages.push_back(this->messageString);

        cguiPrintMutex.unlock();

        return;
}

void Message::printToLog()
{
        std::ofstream log;

        logPrintMutex.lock();

        log.open(LOG_FILE, std::ios_base::app);
        log << this->messageString << std::endl;
        log.close();

        logPrintMutex.unlock();

        return;
}

/*
        Public methods
*/

Message::Message(std::string type, std::string text)
{
        char buffer[18];

        std::time_t now = std::time(NULL);
        std::strftime(buffer, 18, "%d.%m.%y %H:%M:%S", std::localtime(&now));

        this->text = text;
        this->type = type;
        this->time = buffer;

        this->messageToString();

        if (!cguiInit)
        {
                this->printToCli();
        }
        else
        {
                this->printToCgui();
        }

        this->printToLog();

        return;
}

Message::~Message()
{
        return;
}