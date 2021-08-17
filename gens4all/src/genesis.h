/*************************************/
/* Module to manage genesis stuff    */
/*************************************/

/*************************************/
/*              GENESIS              */
/*************************************/

#ifndef __GENESIS_H__

#define __GENESIS_H__

/* Initialise Genesis system */
void Init_Genesis_Bios (void);

/* Initialise Genesis system */
int Init_Genesis (struct Rom *MD_Rom);

/* Reset Genesis system */
void Reset_Genesis(void);

int Do_VDP_Only(void);

int Do_Genesis_Frame_No_VDP (void);

int Do_Genesis_Frame(void);

#endif
