
#ifndef _GENS4ALL_H
#define _GENS4ALL_H

#ifdef DREAMCAST
#include <kos.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <SDL/SDL.h>

#include "debug.h"
#include "profiler.h"

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef void VOID;

typedef struct _POINT {
	int x;
	int y;
} POINT;

typedef enum {
	NORMAL,
	DOUBLE,
	INTERPOLATED,
	FULL_SCANLINE,
	SCANLINE_50,
	SCANLINE_25,
	INTERPOLATED_SCANLINE,
	INTERPOLATED_SCANLINE_50,
	INTERPOLATED_SCANLINE_25,
	KREED,
	SCALE2X,
	HQ2X,
	NB_FILTER} _filters;
	

#define FAR

//#define CLOCK_NTSC 53700000			// More accurate for division round
//#define CLOCK_PAL  53200000

#define CLOCK_NTSC 53693175
#define CLOCK_PAL  53203424

//#define GENS_DEBUG

#define SEPERATOR '/'
#define default_path "./"
#define CHAR_SEP "/"
#define stricmp strcasecmp
#define strnicmp strncasecmp

#define EMU_MODE 0x01
#define BLANC    0x00
#define BLEU     0x02
#define VERT     0x04
#define ROUGE    0x06
#define TRANS    0x08
#define SIZE_X2  0x10

//Must be power of 2
#define CycleTabSize	2048

#include "rom.h"
#include "sound.h"
#include "draw.h"
#include "io.h"
#include "vdp_rend.h"
#include "vdp_32x.h"
#include "save.h"
#include "cd_aspi.h"
#include "vdp_io.h"
#include "blit.h"
#include "gym.h"
#include "mem_m68k.h"
#include "mem_s68k.h"
#include "cpu_m68k.h"
#include "mem_sh2.h"
#include "cpu_sh2.h"
#include "mem_z80.h"
#include "cpu_z80.h"
#include "ym2612.h"
#include "psg.h"
#include "pwm.h"
#include "pcm.h"
#include "genesis.h"

#ifdef RAZE
#include "raze/raze.h"
#else
#include "stef/z80.h"
#endif

#include "bmf.h"

extern int Debug;
extern int Frame_Skip;
extern int Frame_Number;
extern int DAC_Improv;
extern int RMax_Level;
extern int GMax_Level;
extern int BMax_Level;
extern int Contrast_Level;
extern int Brightness_Level;
extern int Greyscale;
extern int Invert_Color;


int Round_Double(double val);
void Init_Tab(void);
void Recalculate_Palettes(void);
void Check_Country_Order(void);
char* Detect_Country_SegaCD(void);
void Detect_Country_Genesis(void);

int Init_32X(struct Rom *MD_Rom);
void Reset_32X();
int Do_32X_Frame_No_VDP(void);
int Do_32X_Frame(void);

int Init_SegaCD(char *iso_name);
int Reload_SegaCD(char *iso_name);
void Reset_SegaCD();
int Do_SegaCD_Frame_No_VDP(void);
int Do_SegaCD_Frame(void);
int Do_SegaCD_Frame_Cycle_Accurate(void);
int Do_SegaCD_Frame_No_VDP_Cycle_Accurate(void);

extern int Paused;
extern int Net_Play;
extern int Current_State;
extern int Full_Screen;
extern int Fast_Blur;
extern int Render_W;
extern int Render_FS;
extern int Show_FPS;
extern int Show_Message;
extern int Show_LED;
extern int Auto_Pause;
extern int Auto_Fix_CS;
extern int Language;
extern int Country;
extern int Country_Order[3];
extern int Kaillera_Client_Running;
extern int Intro_Style;
extern int SegaCD_Accurate;
extern int Active;

extern POINT Window_Pos;

extern char Str_Tmp[1024];
extern char Gens_Path[1024];
extern char Language_Path[1024];
extern char CGOffline_Path[1024];
extern char Manual_Path[1024];

extern char **language_name;
	
extern unsigned char Keys[];
extern unsigned char joystate[];

void system_reset();
#define sync_gens_ui()
#define open_game_genie()
extern int Gens_Running;
#define close_gens() Gens_Running = 0
#define run_gens() Gens_Running = 1
int Init_Input(void);
void End_Input(void);
int Set_Render(int Full, int Num, int Force);
int Change_68K_Type(int hWnd, int Num, int Reset_SND);
//extern void Read_To_68K_Space(int adr);

void End_All(void);
void Sleep(int i);

#define MESSAGE_L(str, def, time) \
{\
	GetPrivateProfileString(language_name[Language], (str), (def), Str_Tmp, 1024, Language_Path);	\
	Put_Info(Str_Tmp, (time));\
}



#define MESSAGE_NUM_L(str, def, num, time)\
{\
	char mes_tmp[1024];\
	Str_Tmp[0]='\0';\
	GetPrivateProfileString(language_name[Language], (str), (def), Str_Tmp, 1024, Language_Path);	\
	sprintf(mes_tmp, Str_Tmp, (num));\
	Put_Info(mes_tmp, (time));\
}

#define MINIMIZE \
{\
	if (Sound_Initialised) Clear_Sound_Buffer();\
	if (Full_Screen)\
	{\
		Set_Render(0, -1, 1);\
		FS_Minimised = 1;\
	}\
}

#if 0

#define MENU_L(smenu, pos, flags, id, str, suffixe, def)\
GetPrivateProfileString(language_name[Language], (str), (def), Str_Tmp, 1024, Language_Path);	\
strcat(Str_Tmp, (suffixe));\
InsertMenu((smenu), (pos), (flags), (id), Str_Tmp);

#define WORD_L(id, str, suffixe, def)\
GetPrivateProfileString(language_name[Language], (str), (def), Str_Tmp, 1024, Language_Path);	\
strcat(Str_Tmp, (suffixe));\
SetDlgItemText(hDlg, id, Str_Tmp);

#endif



enum { MB_OK, MB_ICONEXCLAMATION };
void SetCurrentDirectory(const char *directory);
int GetCurrentDirectory(int size,char* buf);
#if 0
unsigned long GetTickCount();
#else
#define GetTickCount() SDL_GetTicks()
#endif
int GetPrivateProfileInt(const char *section, const char *var, int def, const char *filename);
void GetPrivateProfileString(const char *section, const char *var, const char *def, char *get, int length, const char *filename);
void WritePrivateProfileString(const char *section, const char *var, const char *var_name, const char *filename);

void MessageBox (const char *text, const char *error, int type);

void init_timer();
void GetPerformanceFrequency(long long *freq);
void GetPerformanceCounter(long long *now);
unsigned long gettime();


extern int Have_MMX;
extern int MMX_Enable;

void Identify_CPU(void);
int Half_Blur(void);
void Byte_Swap(void *Ptr, int NumByte);
void Print_Text(SDL_Surface *surface, char *str, int Pos_X, int Pos_Y, int Style);

void Cell_8x8_Dump(unsigned char *Adr, int Palette);
void Cell_16x16_Dump(unsigned char *Adr, int Palette);
void Cell_32x32_Dump(unsigned char *Adr, int Palette);

void CDD_Export_Status(void);

void Write_Sound_Mono_MMX(int *Left, int *Right, short *Dest, int lenght);
void Write_Sound_Stereo_MMX(int *Left, int *Right, short *Dest, int lenght);

int Set_Sprite_Over(int Num);
int Set_Current_State(int Num);
int Set_Frame_Skip(int Num);
int Change_Debug(int Debug_Mode);
int Change_Country_Order(int Num);
int Change_Country(int Num);
int Change_SegaCD_Synchro(void);
int	Change_Sound(void);
int Change_Sample_Rate(int Rate);
int Change_Sound_Stereo(void);
int Change_Z80(void);
int Change_PSG(void);
int Change_PCM(void);
int Change_PWM(void);
int Change_DAC(void);
int Change_CDDA(void);
int Change_Fast_Blur(void);
int Change_Stretch(void);
int Change_VSync(void);
int Change_Blit_Style(void);
int Change_DAC_Improv(void);
int Change_SegaCD_SRAM_Size(int num);
int Change_YM2612(void);
int Change_PSG_Improv(void);
int Change_YM2612_Improv(void);

extern int Render_Point;
extern struct BMFFont * emufont;

extern _filters filters;

#define SELECT_RENDERER(renderer_nb, renderer_fn,msg) \
{\
	*Rend = renderer_nb;\
 	*Blit = renderer_fn;\
	MESSAGE_L("Render selected: " msg,"Render selected: " msg, 1500)\
}

#ifdef DREAMCAST
#if INLINE == static__inline__
#undef INLINE
#define INLINE static __inline__
#endif
#endif

#endif
