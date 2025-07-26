#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef unsigned int int32;
typedef unsigned short int int16;
typedef unsigned char int8;

#define GROW_CAPACITY(cap) (cap < 8 ? 8 : cap * 2);

#endif // COMMON_H
