#ifndef CWS_HASH_H
#define CWS_HASH_H
#include "common.h"

typedef int32 Fnv32_t;
Fnv32_t fnv_32a_str(const char *str, int length);

#endif // !CWS_HASH_H
