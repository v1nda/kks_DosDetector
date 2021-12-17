#include <sstream>

#include "../includes/cgui.h"

bool initDone = false;
bool interrupt = false;

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
                interrupt = true;
        }

        return;
}

void statisticCalculationThreadFunc(Timer &timer, Sniffer &sniffer, Statistic &statistic)
{
        statistic.training(timer, sniffer);

        if (!interrupt)
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
        long long warningTime = 60;
        long long alarmTime = 180;
        bool analysis = false;

        std::string usage = "\n\n-d         \tdevice\
                               \n           \t(mandatory)\n\
                               \n-s -m -h   \ttime of one training capture (seconds/minutes/hours)\
                               \n           \t(default 2 hours)\n\
                               \n-g         \tnumber of captures\
                               \n           \t(default 12 captures)\n\
                               \n-ws -wm    \twarning reaction time (seconds/minutes)\
                               \n           \t(default 1 minute)\n\
                               \n-as -am    \talarm reaction time  (seconds/minutes)\
                               \n           \t(default 3 minutes)\n\
                               \n--analysis \tanalysis training capture for calculating waring and alarm reaction time\
                               \n           \t(default none)";

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
                        else if (str == "-ws")
                        {
                                std::stringstream tmp(args[i + 1]);

                                if (!(tmp >> warningTime) || warningTime < 1)
                                {
                                        message(ERROR_M, "bad arguments");
                                        message(USAGE_M, usage);
                                        std::raise(SIGINT);
                                }
                        }
                        else if (str == "-wm")
                        {
                                std::stringstream tmp(args[i + 1]);

                                if (!(tmp >> warningTime) || warningTime < 1)
                                {
                                        message(ERROR_M, "bad arguments");
                                        message(USAGE_M, usage);
                                        std::raise(SIGINT);
                                }
                                warningTime *= 60;
                        }
                        else if (str == "-as")
                        {
                                std::stringstream tmp(args[i + 1]);

                                if (!(tmp >> alarmTime) || alarmTime < 1 || alarmTime < warningTime)
                                {
                                        message(ERROR_M, "bad arguments");
                                        message(USAGE_M, usage);
                                        std::raise(SIGINT);
                                }
                        }
                        else if (str == "-am")
                        {
                                std::stringstream tmp(args[i + 1]);

                                if (!(tmp >> alarmTime) || alarmTime < 1 || alarmTime < warningTime)
                                {
                                        message(ERROR_M, "bad arguments");
                                        message(USAGE_M, usage);
                                        std::raise(SIGINT);
                                }
                                alarmTime *= 60;
                        }
                        else if (str == "--analysis")
                        {
                                analysis = true;
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
        message(NOTICE_M, "Arguments: warning reaction time " + std::to_string(warningTime));
        message(NOTICE_M, "Arguments: alarm reaction time " + std::to_string(alarmTime));
        message(NOTICE_M, "Arguments: analysis of training capture " + std::to_string(analysis));

        std::signal(SIGINT, interruptHandlingFunc);

        Timer timer;
        Cgui cgui;
        Sniffer sniffer(device);
        Statistic statistic(timeGap, numOfGaps, warningTime, alarmTime, analysis);

        initDone = true;

        std::thread cguiRenderingThread(cguiRenderingThreadFunc, std::ref(timer), std::ref(sniffer), std::ref(statistic), std::ref(cgui));
        std::thread snifferThread(snifferThreadFunc, std::ref(timer), std::ref(sniffer));
        std::thread getDataThread(getDataThreadFunc, std::ref(timer), std::ref(sniffer));
        std::thread statisticCapturesThread(statisticCalculationThreadFunc, std::ref(timer), std::ref(sniffer), std::ref(statistic));

        cguiRenderingThread.join();
        snifferThread.join();
        getDataThread.join();
        statisticCapturesThread.join();

        return 0;
}