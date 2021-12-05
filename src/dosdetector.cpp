#include <sstream>

#include "../includes/cgui.h"

bool initDone = false;
bool interruptFlag = false;

void interruptHandlingFunc(int signum)
{
	message(NOTICE_M, "Interruption ...");

        if (!initDone)
        {
                message(ERROR_M, "initializatioan interrupted");
                exit(2);
        }
        else
        {
                interruptFlag = true;
        }

        return;
}

void statisticCalculationThreadFunc(Timer &timer, Sniffer &sniffer, Statistic &statistic)
{
        statistic.training(timer, sniffer);

        if (!interruptFlag)
        {
                statistic.detection(timer, sniffer);
        }

        return;
}

void snifferThreadFunc(Timer &timer, Sniffer &sniffer)
{
        sniffer.sniff(timer);
}

void getDataThreadFunc(Timer &timer, Sniffer &sniffer)
{
        sniffer.countTraffic(timer);
}

void cguiRenderingThreadFunc(Timer &timer, Sniffer &sniffer, Statistic &statistic, Cgui &cgui)
{
        cgui.renewalCgui(timer, sniffer, statistic);
}

int main(int argc, char *argv[])
{
        std::vector<std::string> args;
        std::ofstream log("dosdetector.log", std::ios_base::trunc);
        log.close();
        std::string device = "";
        int timeGap = 2 * 3600;
        int numOfGaps = 12;

        std::string usage = "\n\n-d         \tdevice\
                               \n           \t(mandatory)\n\
                               \n-s -m -h   \ttime of one training capture (seconds/minutes/hours)\
                               \n           \t(default 2 hours)\n\
                               \n-g         \tnumber of captures\
                               \n           \t(default 12 captures)\n";

        for (int i = 0; i < argc - 1; i++)
        {
                args.push_back(argv[i + 1]);
        }

        if (args.size() == 0)
        {
                message(ERROR_M, "bad arguments");
                message(USAGE_M, usage);
                std::raise(SIGINT);
        }
        else 
        {
                for (size_t i = 0; i < args.size(); i += 2)
                {
                        std::string str = args[i];

                        if (str == "-d")
                        {
                                device = args[i + 1];
                        }
                        else if (str == "-s")
                        {
                                std::stringstream tmp(args[i + 1]);

                                if (!(tmp >> timeGap) || timeGap < 1)
                                {
                                        message(ERROR_M, "bad arguments");
                                        message(USAGE_M, usage);
                                        std::raise(SIGINT);
                                }
                        }
                        else if (str == "-m")
                        {
                                std::stringstream tmp(args[i + 1]);

                                if (!(tmp >> timeGap) || timeGap < 1)
                                {
                                        message(ERROR_M, "bad arguments");
                                        message(USAGE_M, usage);
                                        std::raise(SIGINT);
                                }
                                timeGap *= 60;
                        }
                        else if (str == "-h")
                        {
                                std::stringstream tmp(args[i + 1]);

                                if (!(tmp >> timeGap) || timeGap < 1)
                                {
                                        message(ERROR_M, "bad arguments");
                                        message(USAGE_M, usage);
                                        std::raise(SIGINT);
                                }
                                timeGap *= 3600;
                        }
                        else if (str == "-g")
                        {
                                std::stringstream tmp(args[i + 1]);

                                if (!(tmp >> numOfGaps) || numOfGaps < 1)
                                {
                                        message(ERROR_M, "bad arguments");
                                        message(USAGE_M, usage);
                                        std::raise(SIGINT);
                                }
                        }
                        else
                        {
                                message(ERROR_M, "bad arguments");
                                message(USAGE_M, usage);
                                std::raise(SIGINT);
                        }
                }
        }

        if (device == "")
        {
                message(ERROR_M, "bad arguments");
                message(USAGE_M, usage);
                std::raise(SIGINT);
        }

        message(NOTICE_M, "Arguments: device " + device);
        message(NOTICE_M, "Arguments: time of capture " + secondsToString(timeGap));
        message(NOTICE_M, "Arguments: number of captures " + std::to_string(numOfGaps));

        std::signal(SIGINT, interruptHandlingFunc);

        Timer timer;
        // Cgui cgui;
        Sniffer sniffer(device);
        Statistic statistic(timeGap, numOfGaps);

        initDone = true;

        // std::thread cguiRenderingThread(cguiRenderingThreadFunc, std::ref(timer), std::ref(sniffer), std::ref(statistic), std::ref(cgui));
        std::thread snifferThread(snifferThreadFunc, std::ref(timer), std::ref(sniffer));
        std::thread getDataThread(getDataThreadFunc, std::ref(timer), std::ref(sniffer));
        std::thread statisticCapturesThread(statisticCalculationThreadFunc, std::ref(timer), std::ref(sniffer), std::ref(statistic));

        // cguiRenderingThread.join();
        snifferThread.join();
        getDataThread.join();
        statisticCapturesThread.join();

        return 0;
}