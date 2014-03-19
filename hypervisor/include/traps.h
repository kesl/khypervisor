#ifndef _TRAPS_H_
#define _TRAPS_H_

void emulate_access_to_cp15(unsigned int iss, unsigned int il);
void emulate_wfi_wfe(unsigned int iss, unsigned int il);

#endif
