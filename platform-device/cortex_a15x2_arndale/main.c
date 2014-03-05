#include <guest.h>

int main(void)
{
    start_guest();
    /* The code flow must not reach here */
    while (1)
        ;
    return 0;
}
