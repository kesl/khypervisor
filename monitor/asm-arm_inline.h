#define isb() asm volatile("isb" : : : "memory")
#define dsb() asm volatile("dsb" : : : "memory")

