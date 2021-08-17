/* Miscellaneous functions */
#include "misc.h"


extern u16 MD_Screen[MD_SCREEN_SIZE];
extern u16 MD_Palette[MD_PALETTE_SIZE];
extern u32 Mode_555;

#if (TARGET_SYS,x86)
u32 Have_MMX;
u32 CPU_Model;
#endif

extern CDD_T CDD;
extern SFont_Font* Font;

#if 0
static INLINE void fetch_pixel(u16 *src_ptr, u16 *dst_ptr)
{
	
}

/* Sleep for the specified number of milliseconds */ 
INLINE void Sleep (int ms)
{
	SDL_Delay(ms);
}

/* Sets the text of the emulator window */
INLINE void SetWindowText (const char *s)
{
	SDL_WM_SetCaption (s, NULL);
}

/* Sets the gens icon in the window */
INLINE void setWindowIcon(const char *iconfile)
{
	SDL_WM_SetIcon(SDL_LoadBMP(iconfile),NULL);
}
#endif


#if (TARGET_SYS,x86)
/* void Identify_CPU(void) */
void Identify_CPU(void)
{
	/* MMX detection only supported in the assembly version */
	Have_MMX = 0;
	CPU_Model = 0;
}
#endif

/* int Half_Blur(void) */
int Half_Blur(void)
{
	u32 i;
	u16 tmp;
	u16 mask1 = Mode_555?0x3C0F:0x780F;
	u16 mask2 = Mode_555?0x01E0:0x03E0;

	for(i=0; i < MD_SCREEN_SIZE; i++)
	{
		tmp = MD_Screen[i] / 2;
		MD_Screen[i] = (tmp & mask1) + (tmp & mask2);
	}
}

/* void Byte_Swap(void *Ptr, int NumByte) */
void Byte_Swap(void *Ptr, int NumByte)
{
	u32 i;
	u8 tmp;
	u8 *aux_ptr = (u8 *)Ptr;

	for(i=1; i<NumByte; i+=2)
	{
		tmp = aux_ptr[i - 1];
		aux_ptr[i - 1] = aux_ptr[i];
		aux_ptr[i] = tmp;
	}
}

#if 0
/* void Print_Text(char *str, int Pos_X, int Pos_Y, int Style) */
INLINE void Print_Text(SDL_Surface *surface, char *str, int Pos_X, int Pos_Y, int Style)
{
	SFont_Write (surface,Font,Pos_X,Pos_Y,str);
}
#endif

/* void Cell_8x8_Dump(unsigned char *Adr, int Palette) */
void Cell_8x8_Dump(unsigned char *Adr, int Palette)
{
}

/* void Cell_16x16_Dump(unsigned char *Adr, int Palette) */
void Cell_16x16_Dump(unsigned char *Adr, int Palette)
{
}

#ifdef SEGA_CD
/* void CDD_Export_Status(void) */
/* Sin finalizar */
void CDD_Export_Status(void)
{
	CDD.Rcv_Status[0] = CDD.Status;
	CDD.Rcv_Status[1] = CDD.Minute;
	CDD.Rcv_Status[2] = CDD.Seconde;
}
#endif

/* Round the value to the nearest integer */
int Round_Double (double val)
{
	if ((val - (double) (int) val) > 0.5)
		return (int) (val + 1);
	else
		return (int) val;
}

#if 0
/* Indica si el caracter especificado es imprimible */
INLINE int isPrintable(const char c)
{
	return ((c >= 32) && (c <= 126));
}
#endif
