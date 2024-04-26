#include "zcf/extern/assert.h"
#include <sstream>
#include <ostream>
#include <stdexcept>

extern "C" void assert2throw(int failed, const char *exp, const char *func, const char *file, int line, const char *str){
    if (failed) {
        std::ostringstream oss;
        oss << "assert failed: (" << exp;
        if(str && *str){
            oss << "," << str;
        }
        oss << "), function " << func << ", file " << file << ", line " << line << ".";
        throw std::runtime_error(oss.str());
    }
}