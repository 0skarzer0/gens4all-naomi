#ifndef G_DDRAW_H
#define G_DDRAW_H

#include <time.h>
#include <SDL.h>

extern int Flag_Clr_Scr;
extern int Sleep_Time;
extern int FS_VSync;
extern int W_VSync;
extern int Stretch;
extern int Blit_Soft;
extern int Effect_Color;
extern int FPS_Style;
extern int Message_Style;
extern int Kaillera_Error;
#ifndef DREAMCAST
extern void (*Blit_FS)(unsigned char *Dest, int pitch, int x, int y, int offset);
extern void (*Blit_W)(unsigned char *Dest, int pitch, int x, int y, int offset);
#else
void Blit_Dreamcast(unsigned char *Dest, const int x, const int y, const int offset);
#endif
extern int (*Update_Frame)(void);
extern int (*Update_Frame_Fast)(void);

int Init_Fail(int hwnd, char *err);
int Init_Video_SubSystem(int w, int h, int flags);
int Clear_Primary_Screen(void);
int Clear_Back_Screen(void);
int Update_Gens_Logo(void);
int Update_Crazy_Effect(void);
int Update_Emulation(void);
int Update_Emulation_One(void);
int Update_Emulation_Netplay(int player, int num_player);
int Eff_Screen(void);
int Pause_Screen(void);
void Put_Info(char *Message, int Duree);
int Show_Genesis_Screen(void);
int Flip(void);
void Restore_Primary(void);
int Take_Shot();
void Shutdown_Video_SubSystem(void);
void Put_Info(char *,int);
#ifndef DEBUG_VDP
extern SDL_Surface *surface;
#endif

#endif
