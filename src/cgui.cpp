#include "../includes/cgui.h"

static std::mutex cliMutex;

bool cguiInit = false;

/*
        Private methods
*/

void Cgui::clearScreen(WINDOW *window, int weight, int height)
{
        std::string str = "";

        for (int i = 0; i < weight - 2; i++)
                str += " ";

        for (int i = 0; i < height - 2; i++)
                mvwaddstr(window, i + 1, 1, str.c_str());

        return;
}

void Cgui::refreshScreens()
{
        wrefresh(this->infoWin);
        wrefresh(this->messageWin);
        wrefresh(this->graphWin);

        return;
}

void Cgui::printTemplate()
{
        std::string str = "";

        clear();

        box(this->infoWin, 0, 0);
        box(this->messageWin, 0, 0);
        box(this->graphWin, 0, 0);

        str = "[Main info]";
        mvwaddstr(this->infoWin, 0, 0, str.c_str());
        str = "[Messages]";
        mvwaddstr(this->messageWin, 0, 0, str.c_str());
        str = "[Graph]";
        mvwaddstr(this->graphWin, 0, 0, str.c_str());

        for (size_t i = 0; i < this->infoArray.size(); i++)
        {
                for (size_t j = 0; j < this->infoArray[0].size(); j++)
                {
                        mvwaddstr(this->infoWin, INFO_WIN_INDENT_TOP + i, this->infoColumns[j], this->infoArray[i][j].c_str());
                }
        }

        this->refreshScreens();

        return;
}

void Cgui::printData(Timer &timer, Sniffer &sniffer, Statistic &statistic)
{

        this->infoArray[0][1] = sniffer.getDevice();
        this->infoArray[0][3] = timer.getCurrentTime();
        this->infoArray[1][3] = timer.getWorkTime();
        this->infoArray[3][1] = mode;
        this->infoArray[3][3] = status;
        this->infoArray[6][0] = bytesToString(sniffer.getTrafficAll());
        this->infoArray[6][2] = packetsToString(sniffer.getPacketsAll());
        this->infoArray[7][2] = packetsToString(sniffer.getPacketsPerSec());
        this->infoArray[10][1] = bytesToString(sniffer.getTrafficPerSec()) + "/sec";

        std::string str = std::to_string(statistic.getSmoothingCoeff());
        this->infoArray[10][3] = str.substr(0, str.size() - 4);
        this->infoArray[11][1] = bytesToString(statistic.getSmoothingValue()) + "/sec";
        this->infoArray[11][3] = std::to_string(statistic.getWindowSize());
        this->infoArray[12][1] = bytesToString(statistic.getLimit()) + "/sec";

        for (size_t i = 0; i < this->infoArray.size(); i++)
        {
                for (size_t j = 0; j < this->infoArray[0].size(); j++)
                {
                        if (this->infoArray[i][j].size() > (infoColumns[j + 1] - infoColumns[j]))
                        {
                                this->infoArray[i][j] = "<err>";
                        }
                }
        }

        this->clearScreen(this->infoWin, INFOWIN_W, INFOWIN_H);

        cliMutex.lock();

        for (size_t i = 0; i < this->infoArray.size(); i++)
        {
                for (size_t j = 0; j < this->infoArray[0].size(); j++)
                {
                        mvwaddstr(this->infoWin, INFO_WIN_INDENT_TOP + i, this->infoColumns[j], this->infoArray[i][j].c_str());
                }
        }

        cliMutex.unlock();

        return;
}

void Cgui::printGraph(Sniffer &sniffer, Statistic &statistic)
{
        const wchar_t *str1 = GRAPH_SIGN_1;
        const wchar_t *str2 = GRAPH_SIGN_2;
        const wchar_t *str3 = GRAPH_SIGN_3;
        const wchar_t *str4 = GRAPH_SIGN_4;
        const wchar_t *str5 = GRAPH_SIGN_5;
        const wchar_t *str6 = GRAPH_SIGN_6;
        const std::string str7 = GRAPH_SIGN_7;

        std::vector<std::vector<int>> graphArray;
        for (int i = 0; i < GRAPH_X_MAX; i++)
        {
                std::vector<int> temp;
                for (int j = 0; j < GRAPH_Y_MAX; j++)
                {
                        temp.push_back(0);
                }
                graphArray.push_back(temp);
                temp.clear();
        }

        long long limit = 0;
        long long traffic = 0;
        long long smoothed = 0;

        long long maxValue = 0;
        float rate = 0;
        long long coordinates[NUMBER_OF_GRAPH_COORDINATES];

        mvwaddwstr(this->graphWin, GRAPHWIN_H / 4, INFO_WIN_INDENT_LEFT, str1);
        mvwaddwstr(this->graphWin, GRAPHWIN_H / 4 + 1, INFO_WIN_INDENT_LEFT, str2);
        mvwaddwstr(this->graphWin, GRAPHWIN_H / 4 + 2, INFO_WIN_INDENT_LEFT, str3);

        traffic = sniffer.getTrafficPerSec();

        if (mode == MODE_DETECTION)
        {
                limit = statistic.getLimit();
                smoothed = statistic.getSmoothingValue();
        }
        else
        {
                limit = 0;
                smoothed = 0;
        }

        for (int i = 0; i < GRAPH_X_MAX - 1; i++)
        {
                this->graphValues[i] = this->graphValues[i + 1];
        }

        this->graphValues[GRAPH_X_MAX - 1][0] = limit;
        this->graphValues[GRAPH_X_MAX - 1][1] = traffic;
        this->graphValues[GRAPH_X_MAX - 1][2] = smoothed;

        for (int i = GRAPH_X_MAX - ((GRAPH_X_MAX) / ZOOM_RANGE); i < GRAPH_X_MAX; i++)
        {
                for (int j = 1; j < NUMBER_OF_GRAPH_VALUES; j++)
                {
                        if (this->graphValues[i][j] > maxValue)
                        {
                                maxValue = this->graphValues[i][j];
                        }
                }
        }
        maxValue = (long long)(maxValue * 1.1);

        rate = (float)(GRAPH_Y_MAX) / maxValue;

        for (int i = 0; i < GRAPH_X_MAX; i++)
        {
                int symbols[NUMBER_OF_GRAPH_VALUES];

                for (int j = 0; j < NUMBER_OF_GRAPH_VALUES; j++)
                {
                        symbols[j] = (long long)(this->graphValues[i][j] * rate);
                        if (symbols[j] > GRAPH_Y_MAX - 1)
                        {
                                symbols[j] = GRAPH_Y_MAX;
                        }
                }
                if (symbols[0] > GRAPH_Y_MAX - 1)
                {
                        symbols[0] = 0;
                }

                for (int j = 0; j < symbols[1]; j++)
                {
                        graphArray[i][j] = 2;
                }

                if (symbols[0] > 0)
                {
                        graphArray[i][symbols[0] - 1] = 1;
                }

                for (int j = 0; j < symbols[2]; j++)
                {
                        graphArray[i][j] = 3;
                }

                if (symbols[1] == GRAPH_Y_MAX || symbols[2] == GRAPH_Y_MAX) {
                        graphArray[i][GRAPH_Y_MAX - 1] = 4;
                }
        }

        for (int i = 0; i < NUMBER_OF_GRAPH_COORDINATES; i++)
        {
                std::string coordinate = "";

                for (int j = 0; j < GRAPH_RIGHT_INDENT - 1; j++)
                {
                        coordinate += " ";
                }
                mvwaddstr(this->graphWin, GRAPH_Y_MAX - (((GRAPH_Y_MAX) / NUMBER_OF_GRAPH_COORDINATES) * i), GRAPH_X + GRAPH_X_MAX + 1, coordinate.c_str());

                coordinates[i] = (((GRAPH_Y_MAX) / NUMBER_OF_GRAPH_COORDINATES) * i) / rate;
                coordinate = "<-" + bytesToString(coordinates[i]) + "/sec";

                mvwaddstr(this->graphWin, GRAPH_Y_MAX - (((GRAPH_Y_MAX) / NUMBER_OF_GRAPH_COORDINATES) * i), GRAPH_X + GRAPH_X_MAX + 1, coordinate.c_str());
        }

        for (int i = 0; i < GRAPH_X_MAX; i++)
        {
                for (int j = 0; j < GRAPH_Y_MAX; j++)
                {
                        if (graphArray[i][j] == 0)
                        {
                                mvwaddch(this->graphWin, GRAPH_Y_MAX - j, GRAPH_X + i, ' ');
                        }
                        else if (graphArray[i][j] == 1)
                        {
                                mvwaddwstr(this->graphWin, GRAPH_Y_MAX - j, GRAPH_X + i, str4);
                        }
                        else if (graphArray[i][j] == 2)
                        {
                                mvwaddwstr(this->graphWin, GRAPH_Y_MAX - j, GRAPH_X + i, str5);
                        }
                        else if (graphArray[i][j] == 3)
                        {
                                mvwaddwstr(this->graphWin, GRAPH_Y_MAX - j, GRAPH_X + i, str6);
                        }
                        else if (graphArray[i][j] == 4)
                        {
                                mvwaddstr(this->graphWin, GRAPH_Y_MAX - j, GRAPH_X + i, str7.c_str());
                        }
                }
        }

        for (std::vector<int> i : graphArray)
        {
                i.clear();
        }
        graphArray.clear();

        return;
}

void Cgui::printMessages()
{
        std::vector<std::string> lastMessages = getLastMessages(MESSAGEWIN_H - 2);

        this->clearScreen(this->messageWin, MESSAGEWIN_W, MESSAGEWIN_H);

        cliMutex.lock();

        for (int i = 0; i < MESSAGEWIN_H - 2; i++)
        {
                mvwaddstr(this->messageWin, 1 + i, 1, lastMessages[i].c_str());
        }

        cliMutex.unlock();

        lastMessages.clear();

        return;
}

/*
        Public methods
*/

Cgui::Cgui()
{
        setlocale(LC_ALL, "");

        initscr();
        curs_set(0);
        noecho();

        this->infoWin = newwin(INFOWIN_H, INFOWIN_W, INFOWIN_Y, INFOWIN_X);
        this->messageWin = newwin(MESSAGEWIN_H, MESSAGEWIN_W, MESSAGEWIN_Y, MESSAGEWIN_X);
        this->graphWin = newwin(GRAPHWIN_H, GRAPHWIN_W, GRAPHWIN_Y, GRAPHWIN_X);

        this->infoArray =
        {
                {"Device", "", "Time", ""},
                {"", "", "Work time", ""},
                {"", "", "", ""},
                {"Mode", "", "Status", ""},
                {"", "", "", ""},
                {"", "", "", ""},
                {"", "(all traffic)", "", "(all packets)"},
                {"", "", "", "(packets per sec)"},
                {"", "", "", ""},
                {"", "", "", ""},
                {"Current traffic", "", "Smoothing coef.", ""},
                {"Smoothed traffic", "", "Float window", ""},
                {"Hard limit", "", "", ""},
        };

        int columnDelta = (INFOWIN_W - 2 - INFO_WIN_INDENT_LEFT) / this->infoArray[0].size();
        this->infoColumns.push_back(INFO_WIN_INDENT_LEFT);

        size_t i = 1;
        for (i = 1; i < this->infoArray[0].size(); i++)
        {
                this->infoColumns.push_back(this->infoColumns[i - 1] + columnDelta);
        }
        this->infoColumns.push_back(this->infoColumns[i - 1] + columnDelta);

        for (int j = 0; j < GRAPH_X_MAX; j++)
        {
                std::vector<long long> temp;
                this->graphValues.push_back(temp);

                for (int l = 0; l < NUMBER_OF_GRAPH_VALUES; l++)
                {
                        this->graphValues[j].push_back(0);
                }

                temp.clear();
        }

        message(NOTICE_M, "CGUI: initialisation completed");
        cguiInit = true;

        this->printTemplate();

        return;
}

Cgui::~Cgui()
{
        clear();

        for (std::vector<std::string> i : this->infoArray)
        {
                i.clear();
        }
        this->infoArray.clear();

        this->infoColumns.clear();

        for (std::vector<long long> i : this->graphValues)
        {
                i.clear();
        }
        this->graphValues.clear();

        delwin(this->infoWin);
        delwin(this->messageWin);
        delwin(this->graphWin);
        endwin();

        message(NOTICE_M, "CGUI: end of CGUI");
        cguiInit = false;
}

void Cgui::renewalCgui(Timer &timer, Sniffer &sniffer, Statistic &statistic)
{
        std::this_thread::sleep_until(timer.getTimeCutoff(CLI_CUTOFF_F));
        timer.setTimeCutoff(CLI_CUTOFF_F);

        message(NOTICE_M, "Start of console interface ...");
        
        int interruptionCounter = 0;
        
        while (interruptionCounter < NUMBER_CUTOFF_F - CLI_CUTOFF_F)
        {
                if (interrupt)
                {
                        interruptionCounter++;
                }

                std::this_thread::sleep_until(timer.getTimeCutoff(CLI_CUTOFF_F));
                timer.setTimeCutoff(CLI_CUTOFF_F);
                
                this->printData(timer, sniffer, statistic);
                this->printMessages();
                this->printGraph(sniffer, statistic);
                
                this->refreshScreens();
                
        }

        message(NOTICE_M, "End of console interface");
        this->printMessages();
        this->refreshScreens();

        timer.setTimeCutoff(CLI_CUTOFF_F);
        std::this_thread::sleep_until(timer.getTimeCutoff(CLI_CUTOFF_F));

        return;
}