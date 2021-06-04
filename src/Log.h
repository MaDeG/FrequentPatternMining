#ifndef FREQUENTPATTERNMINING_LOG_H
#define FREQUENTPATTERNMINING_LOG_H

#include <string.h>
#include <iomanip>
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define DEBUG(x) if (Log::debug) { cout << "[" << setw(25) << __FILENAME__ << ":" << setw(4) << left << __LINE__ << "]\t"; x; cout << endl;}

struct Log {
	inline static bool debug;
};

#endif //FREQUENTPATTERNMINING_LOG_H
