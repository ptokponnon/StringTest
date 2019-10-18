
#pragma once
#include <cstdio>

void die(const char *msg)
{
    printf("%s\n", msg);
    exit(1);
}
