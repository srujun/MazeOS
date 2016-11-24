#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define BUFSIZE 10

int main ()
{
    uint8_t buf[BUFSIZE];
    uint32_t cnt, i, num;

    ece391_fdputs(1, (uint8_t*)"Enter number to calculate factorial of:\n");
    if (-1 == (cnt = ece391_read(0, buf, BUFSIZE-1)) ) {
        ece391_fdputs(1, (uint8_t*)"Can't read the number from keyboard.\n");
     return 3;
    }
    buf[cnt] = '\0';

    num = 0;
    i = 0;
    while (buf[i] >= '0' && buf[i] <= '9')
    {
        num *= 10;
        num += (uint32_t)(buf[i] - '0');
        i++;
    }

    cnt = 1;
    while (num != 0)
    {
        cnt *= num;
        num--;
    }

    uint8_t out[BUFSIZE];
    ece391_itoa(cnt, out, 10);

    ece391_fdputs(1, (uint8_t*)"Factorial: ");
    ece391_fdputs(1, out);
    ece391_fdputs(1, (uint8_t*)"\n");

    return 0;
}
