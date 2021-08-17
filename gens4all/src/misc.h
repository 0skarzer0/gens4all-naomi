/* Miscellaneous functions */

#ifndef __MISC_GENS_H__

#define __MISC_GENS_H__

#include "util/SFont.h"
extern SFont_Font* Font;

#define MD_SCREEN_SIZE  336 * 240
#define MD_PALETTE_SIZE 0x100

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

typedef struct
{
	u32 Fader;
	u32 Control;
	u32 Cur_Comm;
	u16 Rcv_Status[5];
	u8 Trans_Comm[10];
	u32 Status;
	u32 Minute;
	u32 Seconde;
	u32 Frame;
	u32 Ext;
} CDD_T;

/* Sleep for the specified number of milliseconds */ 
#define Sleep(MS) SDL_Delay((MS))

/* Sets the text of the emulator window */
#define SetWindowText(STR) SDL_WM_SetCaption((STR),NULL)

/* Sets the gens icon in the window */
#define setWindowIcon(ICONFILE) SDL_WM_SetIcon(SDL_LoadBMP((ICONFILE)),NULL)

/* int Half_Blur(void) */
int Half_Blur(void);

/* void Byte_Swap(void *Ptr, int NumByte) */
void Byte_Swap(void *Ptr, int NumByte);

/* void Print_Text(char *str, int Pos_X, int Pos_Y, int Style) */
#define Print_Text(SURFACE,STR,POSX,POSY,STYLE) SFont_Write((SURFACE),Font,(POSX),(POSY),(STR))

/* void Cell_8x8_Dump(unsigned char *Adr, int Palette) */
void Cell_8x8_Dump(unsigned char *Adr, int Palette);

/* void Cell_16x16_Dump(unsigned char *Adr, int Palette) */
void Cell_16x16_Dump(unsigned char *Adr, int Palette);

/* Round the value to the nearest integer */
int Round_Double (double val);

/* Indica si el caracter especificado es imprimible */
#define isPrintable(CST) (((CST) >= 32) && ((CST) <= 126))

#endif
