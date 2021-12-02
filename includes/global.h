#ifndef GLOBAL_H
#define GLOBAL_H

#define MODE_DEFAULT "IDLING"
#define MODE_TRAINING "TRAINING"
#define MODE_DETECTION "DETECTION"
#define MODE_RESULT "RESULT"

#define STATUS_OK "OK"
#define STATUS_WARNING "WARNING"
#define STATUS_ERROR "ERROR"
#define STATUS_ALARM "ALARM"

#include <string>

extern bool initDone;
extern bool cguiInit;
extern bool interruptFlag;

extern std::string mode;
extern std::string status;

#endif