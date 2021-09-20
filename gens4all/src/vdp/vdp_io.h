#ifndef VDP_IO_H
#define VDP_IO_H

struct Reg_VDP_Type
{
	unsigned int Set1;
	unsigned int Set2;
	unsigned int Pat_ScrA_Adr;
	unsigned int Pat_Win_Adr;
	unsigned int Pat_ScrB_Adr;
	unsigned int Spr_Att_Adr;
	unsigned int Reg6;
	unsigned int BG_Color;
	unsigned int Reg8;
	unsigned int Reg9;
	unsigned int H_Int;
	unsigned int Set3;
	unsigned int Set4;
	unsigned int H_Scr_Adr;
	unsigned int Reg14;
	unsigned int Auto_Inc;
	unsigned int Scr_Size;
	unsigned int Win_H_Pos;
	unsigned int Win_V_Pos;
	unsigned int DMA_Lenght_L;
	unsigned int DMA_Lenght_H;
	unsigned int DMA_Src_Adr_L;
	unsigned int DMA_Src_Adr_M;
	unsigned int DMA_Src_Adr_H;
	unsigned int DMA_Lenght;
	unsigned int DMA_Address;
};

extern unsigned char VRam[64 * 1024];
extern unsigned char CRam[256];
extern unsigned char VSRam[256];
extern unsigned int VSRam_Over[8+30];
extern unsigned char H_Counter_Table[512 * 2];
extern unsigned int Spr_Link[256];
extern int Genesis_Started;
extern int SegaCD_Started;
extern int _32X_Started;

extern struct Reg_VDP_Type VDP_Reg;
extern int VDP_Current_Line;
extern int VDP_Num_Lines;
extern int VDP_Num_Vis_Lines;
extern int CRam_Flag;
extern int VRam_Flag;
extern int VDP_Int;
extern int VDP_Status;
extern int DMAT_Lenght;
struct Ctrl_VDP_Type
{
	int Flag;
	int Data;
	int Write;
	int Access;
	int Address;
	int DMA_Mode;
	int DMA;
};

extern struct Ctrl_VDP_Type Ctrl;

extern int Raster_ColorSplit;
extern int Raster_ColorSplitLine;

struct RasterData {
	short Spr[80*4];
	short Pal[16*4];
	unsigned int *MapA;
	unsigned int *MapB;
	unsigned int *Win;
	int linescroll;
	short VSRAM[80];
};

#define RASTER_LINE_COLOR_CHANGE	(0x01)
#define RASTER_LINE_SCROLL_CHANGE	(0x02)
#define RASTER_LINE_SCREEN_OFF		(0x04)
#define RASTER_LINE_MAPA_CHANGE		(0x08)
#define RASTER_LINE_MAPB_CHANGE		(0x10)
#define RASTER_LINE_SPRLIST_CHANGE	(0x20)
#define RASTER_LINE_WIN_CHANGE		(0x40)
extern char Raster_Lines[256];
extern unsigned char Raster_VSRam[256];
extern char Raster_BgColor[256*2];
extern unsigned char Raster0_CRam[256];
extern unsigned char Raster1_CRam[256];

void Reset_VDP(void);
unsigned int Update_DMA(void);
unsigned short Read_VDP_Data(void);
unsigned short Read_VDP_Status(void);
unsigned char Read_VDP_H_Counter(void);
unsigned char Read_VDP_V_Counter(void);
int Write_Low_Byte_VDP_Data(unsigned char Data);
int Write_High_Byte_VDP_Data(unsigned char Data);
void Write_Word_VDP_Data(unsigned short Data);
int Write_VDP_Ctrl(unsigned short Data);
int Set_VDP_Reg(int Num_Reg, int Val);
#ifdef __cplusplus
extern "C" {
#endif
unsigned char Int_Ack(void);  // function called by the 68k when an interrupt is acknowledged...
#ifdef __cplusplus
}
#endif
void Update_IRQ_Line(void);
void Write_Byte_VDP_Data(unsigned char Data);

#endif
