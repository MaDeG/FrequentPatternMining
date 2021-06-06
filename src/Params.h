#ifndef FREQUENTPATTERNMINING_PARAMS_H
#define FREQUENTPATTERNMINING_PARAMS_H

#include <string.h>
#include <iomanip>
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define DEBUG(x) if (Params::debug) { \
                   _Pragma("omp critical(logEntry)") \
                   { \
                     cout << "[" << setw(25) << std::left << (std::string(__FILENAME__) + ":" + std::to_string(__LINE__)) << "]\t"; \
                     x; \
                     cout << endl; \
                   } \
								 }

struct Params {
	inline static bool debug;
	inline static int nThreads;
};

#endif //FREQUENTPATTERNMINING_PARAMS_H
