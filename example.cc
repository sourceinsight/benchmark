#include "benchmark.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Efficient Integer to String Conversions, by Matthew Wilson.
const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
size_t convert(char buf[], int value)
{
    int i = value;
    char* p = buf;

    do
    {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);

    if (value < 0)
    {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}


BENCHMARK("itoa:sprintf", [](benchmark::Context* ctx) {
    char buf[25] = {0};
    for (int i = INT_MIN; i <= INT_MAX; i++)
    {
        sprintf(buf, "%d", i);
    }
})


BENCHMARK("itoa:Matthew Wilson", [](benchmark::Context* ctx) {
    char buf[25] = {0};
    for (int i = INT_MIN; i <= INT_MAX; i++)
    {
        convert(buf, i);
    }
})

