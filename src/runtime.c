
#include "mipi.h"

extern const char * _MIPI_ERR_STRING[]=
{
	[MIPI_ERR_NO_MEM]="OUT OF MEMORY",
	[MIPI_ERR_INV]="INVALID OPERANDS PROVIDED",
	[MIPI_ERR_IO]="I/O OPERATION FAILED DUE TO CONFIGURATION OR HARDWARE",
	[MIPI_ERR_INTERRUPT]="TASK WAS INTERRUPTED",
	[MIPI_ERR_RES_LOCKED]="THE REQUESTED RESOURCE IS BUSY",
};
