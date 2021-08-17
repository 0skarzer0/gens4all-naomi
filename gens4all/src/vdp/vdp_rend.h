#ifndef VDP_REND_H
#define VDP_REND_H

#ifndef S32
#define S32 int
#endif

extern unsigned short MD_Screen[336 * 240];
extern unsigned short MD_Palette[256];
extern unsigned short Palette[0x1000];
extern S32 TAB336[240];


typedef struct
{
	S32 H_Min;
	S32 H_Max;
} Data_Spr_Type;

typedef struct
{
//	S32 Pattern_Adr;
	S32 Line_7;
	S32 X;
	S32 Cell;
	S32 Start_A;
	S32 Lenght_A;
	S32 Start_W;
	S32 Lenght_W;
	S32 Mask;
	S32 Spr_End;
	S32 Next_Cell;
	S32 Palette;
	S32 Borne;
} Data_Misc_Type;


/*
extern struct
{
	int Pos_X;
	int Pos_Y;
	unsigned int Size_X;
	unsigned int Size_Y;
	int Pos_X_Max;
	int Pos_Y_Max;
	unsigned int Num_Tile;
	int dirt;
} Sprite_Struct[256];
*/

extern int Mode_555;
extern int Sprite_Over;

#ifdef __cplusplus
extern "C" {
#endif
void Render_Line();
void Render_Line_32X();
#ifdef __cplusplus
}
#endif

#endif
