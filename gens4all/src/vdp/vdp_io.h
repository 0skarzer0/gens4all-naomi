#ifndef VDP_IO_H
#define VDP_IO_H

//~ typedef unsigned int VDPReg_t;
typedef unsigned char VDPReg_t;
//~ typedef short VDPReg_t;
struct Reg_VDP_Type
{
	union {
		struct {
			VDPReg_t Set1;
			VDPReg_t Set2;
			VDPReg_t Pat_ScrA_Adr;
			VDPReg_t Pat_Win_Adr;
			VDPReg_t Pat_ScrB_Adr;
			VDPReg_t Spr_Att_Adr;
			VDPReg_t Reg6;
			VDPReg_t BG_Color;
			VDPReg_t Reg8;
			VDPReg_t Reg9;
			VDPReg_t H_Int;
			VDPReg_t Set3;
			VDPReg_t Set4;
			VDPReg_t H_Scr_Adr;
			VDPReg_t Reg14;
			VDPReg_t Auto_Inc;
			VDPReg_t Scr_Size;
			VDPReg_t Win_H_Pos;
			VDPReg_t Win_V_Pos;
			VDPReg_t DMA_Lenght_L;
			VDPReg_t DMA_Lenght_H;
			VDPReg_t DMA_Src_Adr_L;
			VDPReg_t DMA_Src_Adr_M;
			VDPReg_t DMA_Src_Adr_H;
			unsigned int DMA_Lenght;
			unsigned int DMA_Address;
		};
		VDPReg_t r[24];
	};
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

#if 1
struct VDPStat {
	int vdp_current_line;
	int vdp_num_lines;
	int vdp_num_vis_lines;
	int vdp_int;
	int vdp_status;
	int dmat_lenght;
	struct Reg_VDP_Type regs;
};
extern struct VDPStat VDPStats;

#define VDP_Reg VDPStats.regs
#define VDP_Current_Line VDPStats.vdp_current_line
#define VDP_Num_Lines VDPStats.vdp_num_lines
#define VDP_Num_Vis_Lines VDPStats.vdp_num_vis_lines
#define VDP_Int VDPStats.vdp_int
#define VDP_Status VDPStats.vdp_status
#define DMAT_Lenght VDPStats.dmat_lenght

#else
extern struct Reg_VDP_Type VDP_Reg;
extern int VDP_Current_Line;
extern int VDP_Num_Lines;
extern int VDP_Num_Vis_Lines;
extern int VDP_Int;
extern int VDP_Status;
extern int DMAT_Lenght;
#endif
extern int CRam_Flag;
extern int VRam_Flag;
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

extern unsigned int Spr_Addr;

extern int Raster_ColorSplit;
extern int Raster_ColorSplitLine;
extern int Raster_SpriteSplitLine;

typedef struct {
	short y;
	char link, flags;
	short tile;
	short x;
} VDP_Sprite;

typedef struct {
	short vscroll_a, vscroll_b;
	short hscroll_a, hscroll_b;
	//~ VDP_Sprite *spr;
	//~ short *vsram;	//If NULL, use vscroll_x values as full screen scroll, otherwise using vsram as column scroll
	struct Reg_VDP_Type regs;
	int padding[6];
} RasterData;

#define RASTER_LINE_COLOR_CHANGE	(0x01)
#define RASTER_LINE_VSCROLLA_CHANGE	(0x02)
#define RASTER_LINE_VSCROLLB_CHANGE	(0x80)
#define RASTER_LINE_SCREEN_OFF		(0x04)
#define RASTER_LINE_MAPA_CHANGE		(0x08)
#define RASTER_LINE_MAPB_CHANGE		(0x10)
#define RASTER_LINE_SPRLIST_CHANGE	(0x20)
#define RASTER_LINE_WIN_CHANGE		(0x40)
//~ #define RASTER_LINE_HSCROLL_CHANGE	(0x80)

#define MAX_SPRITES	(80)
#define MAX_REGISTER_RASTERS	(8)
#define MAX_SPRITE_RASTERS	(2)

extern VDP_Sprite Raster_Sprites[MAX_SPRITE_RASTERS][MAX_SPRITES];
extern RasterData Raster_Data[256];
extern char Raster_Lines[257];
extern short Raster_BgColor[256*2];
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
