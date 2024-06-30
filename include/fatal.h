#ifndef FATAL_H
#define FATAL_H

#include <debug.h>

void sysHalt(int error_code, const char *msg);
void sysWarn(const char *msg);

#endif