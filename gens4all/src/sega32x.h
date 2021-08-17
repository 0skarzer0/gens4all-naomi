/****************************************************************************/
/* SEGA 32X main module                                                     */
/****************************************************************************/

#ifdef __GENS_SEGA_32X_H__

#define __GENS_SEGA_32X_H__

#include <stdio.h>
#include "gens4all.h"
#include "vdp_io.h"
#include "vdp_32x.h"
#include "sh2.h"

int Init_32X (struct Rom *MD_Rom);

void Reset_32X (void);

int Do_32X_Frame_No_VDP (void);

int Do_32X_Frame (void);


#endif
