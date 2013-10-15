#include <arch_types.h>
#include "print.h"
#include <armv7_p15.h>
#include "context.h"
#include "abort_trap_handler.h"
#include "virtual_devices.h"

void abort_trap_handler(unsigned int iss, struct arch_regs *regs)
{
	//far, fipa, il
	unsigned int far = read_hdfar();
	unsigned int fipa;
	unsigned int sas, srt, wnr;
	unsigned int result;

	fipa = (read_hpfar() & HPFAR_FIPA_MASK) >> HPFAR_FIPA_SHIFT;
	fipa = fipa << HPFAR_FIPA_PAGE_SHIFT;
	fipa = fipa | (far & HPFAR_FIPA_PAGE_MASK);
	sas = (iss & ISS_SAS_MASK) >> ISS_SAS_SHIFT;
	srt = (iss & ISS_SRT_MASK) >> ISS_SRT_SHIFT;
	wnr = iss & ISS_WNR;

	switch (iss & ISS_FSR_MASK) {
		case TRANS_FAULT_LEVEL1:
		case TRANS_FAULT_LEVEL2:
		case TRANS_FAULT_LEVEL3:
			if (iss & ISS_VALID) {

        		if (wnr) { //ISWRITE
						result = find_vdev_handler(fipa, wnr, srt, regs);
				}
				else { //ISREAD?
						result = find_vdev_handler(fipa, wnr, srt, regs);
				}
			}
		break;

		case ACCESS_FAULT_LEVEL1:
		case ACCESS_FAULT_LEVEL2:
		case ACCESS_FAULT_LEVEL3:
			printh("ACCESS fault %d\n", iss & ISS_FSR_MASK);
		break;
		default:
		break;
	}

	regs->pc += 4;
}
