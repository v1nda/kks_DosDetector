#ifndef CLI_H
#define CLI_H

#include <ncurses.h>

#include "../includes/statistics.h"

#define INFOWIN_H LINES / 2
#define INFOWIN_W COLS / 2
#define INFOWIN_X 0
#define INFOWIN_Y 0

#define INFO_WIN_INDENT_LEFT 3
#define INFO_WIN_INDENT_TOP 2

#define MESSAGEWIN_H LINES / 2
#define MESSAGEWIN_W COLS / 2
#define MESSAGEWIN_X COLS / 2
#define MESSAGEWIN_Y 0

#define GRAPHWIN_H LINES / 2
#define GRAPHWIN_W COLS
#define GRAPHWIN_X 0
#define GRAPHWIN_Y LINES / 2

#define GRAPH_X 30
#define GRAPH_Y 0
#define GRAPH_RIGHT_INDENT 20
#define GRAPH_X_MAX COLS - 2 - (GRAPH_X) - GRAPH_RIGHT_INDENT
#define GRAPH_Y_MAX (LINES / 2) - 2 - GRAPH_Y

#define NUMBER_OF_GRAPH_VALUES 3

#define NUMBER_OF_GRAPH_COORDINATES 5

#define ZOOM_RANGE 8

#define SLEEP_AFTER_INTERRUPCTION 5

class Cgui
{
private:
        WINDOW *infoWin;
        WINDOW *messageWin;
        WINDOW *graphWin;

        std::vector<std::vector<std::string>> infoArray;
        std::vector<u_int> infoColumns;

        std::vector<std::vector<long long>> graphValues;

        void clearScreen(WINDOW *window, int weight, int height);
        void refreshScreens();
        
        void printTemplate();
        void printData(Timer &timer, Sniffer &sniffer, Statistic &statistic);
        void printGraph(Sniffer &sniffer, Statistic &statistic);
        void printMessages();

public:
        Cgui();
        ~Cgui();

        void renewalCgui(Timer &timer, Sniffer &sniffer, Statistic &statistic);
};

#endif