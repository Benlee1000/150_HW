#include <stdio.h>
#include <time.h>
#include <stdint.h>
 
int main(void)
{
    time_t now;
    struct tm * timeinfo;
    time (&now);
    timeinfo = localtime(&now);
    printf("Time:%s", asctime(timeinfo));
}