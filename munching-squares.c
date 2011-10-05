#include <stdio.h>
main() {
    int t;
    for(t=0;;t++)
        putchar(
            t & (t >> 8)
        );
}
