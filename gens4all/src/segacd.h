/****************************************************************************/
/* SEGA CD main module                                                      */
/****************************************************************************/

#ifdef __GENS_SEGA_CD_H__

#define __GENS_SEGA_CD_H__

#include "gens4all.h"

int Init_SegaCD (char *iso_name);

int Reload_SegaCD (char *iso_name);

void Reset_SegaCD(void);

int Do_SegaCD_Frame_No_VDP (void);

int Do_SegaCD_Frame_No_VDP_Cycle_Accurate (void);

int Do_SegaCD_Frame (void);

int Do_SegaCD_Frame_Cycle_Accurate (void);

#endif
