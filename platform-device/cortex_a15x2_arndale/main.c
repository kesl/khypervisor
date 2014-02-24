#include <context.h>

int main(void)
{
    start_guest_os();
    /* The code flow must not reach here */
    while (1)
        ;
    return 0;
}
