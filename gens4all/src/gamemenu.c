#include "gens4all.h"
#include "vdp_rend.h"
#include "blit.h"
#include "input.h"
#include "rom.h"
#include "../../../libsh4/sh4.h"
#include "../../../libsh4/sq.h"
#include "../pvrvdp/npvr_vertex.h"
#include "../pvrvdp/pvr_rend.h"

#ifdef VDP_HIRES
#define	CENTER_X	320
#define	CENTER_Y	240
#else
#define	CENTER_X	160
#define	CENTER_Y	120
#endif

#define MENU_BG_COLOR	0xb0202030

#define ARR_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define LINE_HEIGHT	(emufont->lineheight)

struct MenuItem 
{
	const char *string;
	int selectable;
	const char * (*getstr)(const struct MenuItem *mi);
	void (*picked)(const struct MenuItem *mi);
	void (*left)(const struct MenuItem *mi);
	void (*right)(const struct MenuItem *mi);
	int idx;
};

static const char *MyStr(const struct MenuItem *mi) 
{
	return mi->string;
}
	
static float line(float y) 
{
	return 25 + y * emufont->lineheight;
}

static int string_length_pixels(struct BMFFont *font, const char * str) 
{
	int len = strlen(str);
	return BMFStringWidth(font, AsciiReader, str, len);
}

static void puts_center(struct BMFFont *font, float y, const char * str) 
{
	BMFputs(font, CENTER_X - string_length_pixels(font, str)/2, y, 500, AsciiReader, str);
}

static void ProcessMenu(const struct MenuItem *menu, int menusize, int *selection, float y) 
{
	int sel = *selection;
	
	//Make sure selection is in bounds and on a selectable item
	sel = sel < 0 ? 0 : sel;
	sel = sel >= menusize ? 0 : sel;
	while(!menu[sel].selectable) 
	{
		sel++;
		sel = sel >= menusize ? 0 : sel;
	}
	
	//Move selection according to input
	if (UICont->c.d) 
	{
		sel++;
		sel = sel >= menusize ? 0 : sel;
		while(!menu[sel].selectable) 
		{
			sel++;
			sel = sel >= menusize ? 0 : sel;
		}
	}
	
	if (UICont->c.u) 
	{
		sel--;
		while(!menu[sel].selectable) 
		{
			sel--;
			sel = sel < 0 ? menusize-1 : sel;
		}
	}
	
	//Activate selection
	if (UICont->c.a) 
	{
		if (menu[sel].picked != NULL) 
		{
			menu[sel].picked(menu+sel);
		}
	} 
	else if (UICont->c.l) 
	{
		if (menu[sel].left != NULL) 
		{
			menu[sel].left(menu+sel);
		}
	} 
	else if (UICont->c.r) 
	{
		if (menu[sel].right != NULL) 
		{
			menu[sel].right(menu+sel);
		}
	}
	
	void *pc = sqPrepare((void*)PVR_TA_INPUT);
	
	//Draw selector
	draw_rect_wh(24, line(sel)-3 + y, 320-48, emufont->lineheight, 420, 0x80ff0000, pc);
	//Draw menu text
	int i;
	for(i = 0; i < menusize; i++) 
	{
		puts_center(emufont, line(i) + y, menu[i].getstr(menu+i));
	}
	
	//Update selection
	*selection = sel;
}


static void MainMenu();
static void (*MenuMode)() = &MainMenu;

void BringUpGameMenu() 
{
	Paused = 1;
	//~ MenuMode = &MainMenu;
	//~ Block_Hold_And_Click();
	Block_Hold();
	//~ Block_Click();
}

void CloseGameMenu() 
{
	Paused = 0;
	//~ MenuMode = &MainMenu;
	Block_Click();
}

static void ExitGame() 
{
	dbgf("Exit\n");
	Gens_Running = 0;
	extern int Reset_Input_Lockout;
	Reset_Input_Lockout = 1;
}

static void VideoMenu();
static void EnterVideoMenu() 
{
	MenuMode = &VideoMenu;
}

////////////////////////////////////////////////////////////////////////////
//Debug
extern int Show_Timing;
extern int Show_Rasters;
extern int Enable_Shadow_Highlight;
extern int Show_FPS;

static void ToggleProf() 
{
	Show_Timing = !Show_Timing;
}

static void ToggleShowRaster() 
{
	Show_Rasters = !Show_Rasters;
}

static void ToggleRenderPoint() 
{
	if (Render_Point <= 1)
		Render_Point = !Render_Point;
}

static void ToggleSH() 
{
	Enable_Shadow_Highlight = !Enable_Shadow_Highlight;
}

static const char *ProfStr(const struct MenuItem *mi) 
{
	return Show_Timing ? "Perf. Timing (Shown)" : "Perf. Timing (Hidden)";
}

static const char *ShowRasterStr(const struct MenuItem *mi) 
{
	return Show_Rasters ? "Raster Events (Shown)" : "Raster Events (Hidden)";
}

static const char *RPointStr(const struct MenuItem *mi) 
{
	if (Render_Point == 0)
		return "Rendering at VBlank Start";
	if (Render_Point == 1)
		return "Rendering at VBlank End";
	if (Render_Point == 2 || Render_Point == 3)
		return "Rendering at first blank";
	return "Rendering at ???";
}

static const char *SHStr(const struct MenuItem *mi) 
{
	return Enable_Shadow_Highlight ? "S/H Enabled" : "S/H Disabled";
}

static void ToggleFPS() 
{
	Show_FPS = !Show_FPS;
}

static const char *FPSStr(const struct MenuItem *mi) 
{
	return Show_FPS ? "FPS (Shown)" : "FPS (Hidden)";
}

#define MAX_MAPA	1
#define MAX_MAPB	1
extern int Enable_MapA;
extern int Enable_MapB;
extern int Enable_Win;
extern int Enable_Sprites;

static void NextMapA() 
{
	Enable_MapA++;
	if (Enable_MapA > MAX_MAPA) Enable_MapA = 0;
}

static void PrevMapA() 
{
	Enable_MapA--;
	if (Enable_MapA < 0) Enable_MapA = MAX_MAPA;
}

static void NextMapB() 
{
	Enable_MapB++;
	if (Enable_MapB > MAX_MAPB) Enable_MapB = 0;
}

static void PrevMapB() 
{
	Enable_MapB--;
	if (Enable_MapB < 0) Enable_MapB = MAX_MAPB;
}

static const char *MapAStr(const struct MenuItem *mi) 
{
	if (Enable_MapA == 1)
		return "Map A (On)";
	if (Enable_MapA == 2)
		return "Map A (Old)";
	return "Map A (Off)";
}

static const char *MapBStr(const struct MenuItem *mi) 
{
	if (Enable_MapB == 1)
		return "Map B (On)";
	if (Enable_MapB == 2)
		return "Map B (Old)";
	return "Map B (Off)";
}

static void ToggleWin() 
{
	Enable_Win = !Enable_Win;
}

static const char *WinStr(const struct MenuItem *mi) 
{
	return Enable_Win ? "Window (On)" : "Window (Off)";
}

static void ToggleSprites() 
{
	Enable_Sprites = !Enable_Sprites;
}

static const char *SpritesStr(const struct MenuItem *mi) 
{
	return Enable_Sprites ? "Sprites (On)" : "Sprites (Off)";
}

static void DebugDispMenu() 
{
	static int DebugDispSelection = 0;
	
	static const struct MenuItem layermenu[] = 
	{
		{"Display Debug", 0, &MyStr, 0, 0, 0},
		{"", 0, &MyStr, 0, 0, 0},
		//~ {"S/H En", 1, &SHStr, &ToggleSH, &ToggleSH, &ToggleSH},
		{"Show Prof", 1, &ProfStr, &ToggleProf, &ToggleProf, &ToggleProf},
		{"Show Raster", 1, &ShowRasterStr, &ToggleShowRaster, &ToggleShowRaster, &ToggleShowRaster},
		{"Show FPS", 1, &FPSStr, &ToggleFPS, &ToggleFPS, &ToggleFPS},
		//~ {"Rasters", 1, &RasterStr, &ToggleRaster, &ToggleRaster, &ToggleRaster},
		{"", 0, &MyStr, 0, 0, 0},
		
		{"Map A", 1, &MapAStr, &NextMapA, &PrevMapA, &NextMapA},
		{"Map B", 1, &MapBStr, &NextMapB, &PrevMapB, &NextMapB},
		{"Window", 1, &WinStr, &ToggleWin, &ToggleWin, &ToggleWin},
		{"Sprites", 1, &SpritesStr, &ToggleSprites, &ToggleSprites, &ToggleSprites},
		
		{"", 0, &MyStr, 0, 0, 0},
		{"Back", 1, &MyStr, &EnterVideoMenu, 0, 0},
	};
	
	if (UICont->h.y)
		return;
	
	if (UICont->c.b)
		EnterVideoMenu();
	else if (UICont->c.s || UICont->c.au)
		CloseGameMenu();
	
	//Draw shadow
	draw_rect_wh(24, 24, 320-48,240-48, 400, MENU_BG_COLOR, sqPrepare((void*)PVR_TA_INPUT));
	
	ProcessMenu(layermenu, ARR_SIZE(layermenu), &DebugDispSelection, 1);
}

static void EnterDebugDisplay() 
{
	MenuMode = &DebugDispMenu;
}

////////////////////////////////////////////////////////////////////////////
//Controller
static int ControlSetting = 0;

struct ControllerSetting 
{
	const char *name;
	EmuInputButtons *mapping;
};

extern EmuInputButtons ContMap4to3;
extern EmuInputButtons ContMap4to3Rev;
extern EmuInputButtons ContMap4to6;
extern EmuInputButtons ContMap6to6;
extern EmuInputButtons ContMap4toViking;
extern EmuInputButtons ContMap4toSF2;

static const struct ControllerSetting contsettings[] = 
{
	{"DC 4B to Gen 3B", &ContMap4to3},
	{"DC 4B to Gen 3B (Rev)", &ContMap4to3Rev}, 
	{"DC 4B to Gen 6B", &ContMap4to6},
	{"DC 4B to Gen 6B (Viking)", &ContMap4toViking},
	{"DC 4B to Gen 6B (SF2)", &ContMap4toSF2},
	{"DC 6B to Gen 6B", &ContMap6to6}, 
};

static void UpdateControlSetting() 
{
	if (ControlSetting < 0 || ControlSetting >= ARR_SIZE(contsettings))
		ControlSetting = 0;
	
	extern EmuInputButtons *ContMap[MAX_CONTROLLERS];
	ContMap[0] = contsettings[ControlSetting].mapping;
	ContMap[1] = contsettings[ControlSetting].mapping;
	ContMap[2] = contsettings[ControlSetting].mapping;
	ContMap[3] = contsettings[ControlSetting].mapping;
	//Always use 6 button
	Controller_1_Type |= 1;
	Controller_2_Type |= 1;
	
	//Except for first option
	if (ControlSetting == 0) 
	{
		Controller_1_Type &= ~1;
		Controller_2_Type &= ~1;
	}
		
}

void SetControls(int controlidx) 
{
	ControlSetting = controlidx;
	UpdateControlSetting();
}


static const char *ControlStr(const struct MenuItem *mi) 
{
	return contsettings[ControlSetting].name;
}

static void NextControlSetting() 
{
	ControlSetting++;
	if (ControlSetting >= ARR_SIZE(contsettings))
		ControlSetting = 0;
	UpdateControlSetting();
}

static void PrevControlSetting() 
{
	ControlSetting--;
	if (ControlSetting < 0)
	{
		ControlSetting = ARR_SIZE(contsettings)-1;
	}
	UpdateControlSetting();
}

static void ExitController() 
{
	MenuMode = &MainMenu;
}

static void ControllerMenu();
static void EnterControllerMenu() 
{
	MenuMode = &ControllerMenu;
}

static void ChangeControls(const struct MenuItem *mi) 
{
	SetControls(mi->idx);
}

static const char *ControlStr2(const struct MenuItem *mi) 
{
	static char buff[100];
	if (ControlSetting == mi->idx)
		snprintf(buff, sizeof(buff)-1, ">>>%s<<<", mi->string);
	else
		snprintf(buff, sizeof(buff)-1, "%s", mi->string);
	return buff;
}

static int ControllerSelection = 0;

static int ShownController;
static const char *ShownControllerName;
int print_buttonmap(float y, const char *dclabel, int genbutton) 
{
	static const char *buttons[] = 
	{
		"Null", "A", "B", "C", "X", "Y", "Z", "Start", "Mode",
		"Up", "Down", "Left", "Right"
	};
	char mapbuff[100];
	
	if (genbutton < 0 || genbutton >= GENC_COUNT) 
	{
		printf("Bad genbutton %i\n", genbutton);
		return y;
	}
	
	if (genbutton == 0)
		return y;
	
	sprintf(mapbuff, "%s -> %s", dclabel, buttons[genbutton]);
	puts_center(emufont, y, mapbuff);
	
	return y + emufont->lineheight;
}

static void ShowController() 
{
	if (UICont->c.b || UICont->c.y) 
	{
		EnterControllerMenu();
	} 
	else if (UICont->c.s || UICont->c.au) 
	{
		CloseGameMenu();
	}
	
	draw_rect_wh(24, 24, 320-48,240-48, 400, MENU_BG_COLOR, sqPrepare((void*)PVR_TA_INPUT));
	
	EmuInputButtons *selcont = contsettings[ShownController].mapping;
	
	int i;
	puts_center(emufont, line(0.5), ShownControllerName);
	puts_center(emufont, line(2.5), "DC -> Gen");
	float y = line(4.5);
	
	if (selcont != &ContMap6to6) 
	{
		y = print_buttonmap(y, "A", selcont->a);
		y = print_buttonmap(y, "B", selcont->b);
		y = print_buttonmap(y, "X", selcont->x);
		y = print_buttonmap(y, "Y", selcont->y);
		y = print_buttonmap(y, "L", selcont->sl);
		y = print_buttonmap(y, "R", selcont->sr);
		y = print_buttonmap(y, "Analog-Down", selcont->ad);
	} 
	else 
	{
		y = print_buttonmap(y, "A", selcont->a);
		y = print_buttonmap(y, "B", selcont->b);
		y = print_buttonmap(y, "C", selcont->c);
		y = print_buttonmap(y, "X", selcont->x);
		y = print_buttonmap(y, "Y", selcont->y);
		y = print_buttonmap(y, "Z", selcont->z);
		y = print_buttonmap(y, "R", selcont->sr);
	}
}

static void ControllerMenu() 
{
	static const struct MenuItem menu[] = 
	{
		{"Controls", 0, &MyStr, 0, 0, 0},
		{"(Y: Show)", 0, &MyStr, 0, 0, 0},
		{"", 0, &MyStr, 0, 0, 0},
	
		{"3 Button", 1, &ControlStr2, &ChangeControls, 0, 0, 0},
		{"3 Button (A/C Reversed)", 1, &ControlStr2, &ChangeControls, 0, 0, 1},
		{"6 Button", 1, &ControlStr2, &ChangeControls, 0, 0, 2},
		{"6 Button (Alt)", 1, &ControlStr2, &ChangeControls, 0, 0, 3},
		{"6 Button (Fighting)", 1, &ControlStr2, &ChangeControls, 0, 0, 4},
		{"6 Button (6B DC)", 1, &ControlStr2, &ChangeControls, 0, 0, 5},
		{"", 0, &MyStr, 0, 0, 0},
		{"Back", 1, &MyStr, &ExitController, 0, 0},
	};
	
	if (UICont->c.y) 
	{
		MenuMode = &ShowController;
		ShownControllerName = menu[ControllerSelection].string;
		ShownController = menu[ControllerSelection].idx;
	} 
	else if (UICont->c.b) 
	{
		ExitController();
	} 
	else if (UICont->c.s || UICont->c.au) 
	{
		CloseGameMenu();
	}
	//Draw shadow
	draw_rect_wh(24, 24, 320-48,240-48, 400, MENU_BG_COLOR, sqPrepare((void*)PVR_TA_INPUT));
	
	ProcessMenu(menu, ARR_SIZE(menu), &ControllerSelection, 1);
}


////////////////////////////////////////////////////////////////////////////
//Render setting
struct NLineDrawSetting 
{
	const char *name;
	int drawa;
	int drawb;
};

static const struct NLineDrawSetting ndrawsettings[] = 
{
	{"Line Scroll", MAP_DRAW_SPLIT2, MAP_DRAW_SPLIT2},
	{"Line Scroll (4 div A)", MAP_DRAW_SPLIT4, MAP_DRAW_SPLIT2},
	{"Line Scroll (4 div B)", MAP_DRAW_SPLIT2, MAP_DRAW_SPLIT4},
	{"Cell Scroll", MAP_DRAW_SPLIT2_CELL, MAP_DRAW_SPLIT2_CELL},
	{"Cell Scroll (4 div A)", MAP_DRAW_SPLIT4_CELL, MAP_DRAW_SPLIT2_CELL},
	{"Cell Scroll (4 div B)", MAP_DRAW_SPLIT2_CELL, MAP_DRAW_SPLIT4_CELL},
};

static int NDrawSetting = 0;
extern int Sonic_2_Auto_Render;

static const char *NDrawStr(const struct MenuItem *mi) 
{
	if (Sonic_2_Auto_Render)
		return "Sonic 2 Auto";
	else
		return ndrawsettings[NDrawSetting].name;
}

static void NUpdateDrawSetting() 
{
	MapBDrawFuncId = ndrawsettings[NDrawSetting].drawb;
	MapADrawFuncId = ndrawsettings[NDrawSetting].drawa;
}

void SetDrawSetting(int setting) 
{
	if (setting < 0)
		setting = 0;
	if (setting >= ARR_SIZE(ndrawsettings))
		setting = 0;
	NDrawSetting = setting;
	NUpdateDrawSetting();
}

static void NNextDrawSetting() 
{
	if (Sonic_2_Auto_Render)
		return;
	NDrawSetting++;
	if (NDrawSetting >= ARR_SIZE(ndrawsettings))
		NDrawSetting = 0;
	NUpdateDrawSetting();
}

static void NPrevDrawSetting() 
{
	if (Sonic_2_Auto_Render)
		return;
	NDrawSetting--;
	if (NDrawSetting < 0)
		NDrawSetting = ARR_SIZE(ndrawsettings)-1;
	NUpdateDrawSetting();
}

////////////////////////////////////////////////////////////////////////////
//SRAM
static int save_result;
static void SaveResult() 
{
	if (UICont->c.a || UICont->c.b || UICont->c.s) 
	{
		BringUpGameMenu();
		//~ MenuMode = &MainMenu;
		MenuMode = &MainMenu;
	}
	
	void *pc = sqPrepare((void*)PVR_TA_INPUT);
	
	//Draw shadow
	draw_rect_wh(24*2, 24*2, 320-48*2,240-48*2, 400, MENU_BG_COLOR, pc);
	
	if (save_result == -1)
		puts_center(emufont, CENTER_Y - emufont->lineheight/2, "Nothing to save");
	else if (save_result == 0)
		puts_center(emufont, CENTER_Y - emufont->lineheight/2, "Save failed");
	else if (save_result == 1)
		puts_center(emufont, CENTER_Y - emufont->lineheight/2, "Save completed");
	else
		puts_center(emufont, CENTER_Y - emufont->lineheight/2, "Unknown result");
}

static void CommitSave() 
{
	save_result = Commit_Save_SRAM();
	MenuMode = &SaveResult;
	
	void *pc = sqPrepare((void*)PVR_TA_INPUT);
	//Draw shadow
	draw_rect_wh(24*2, 24*2, 320-48*2,240-48*2, 400, MENU_BG_COLOR, pc);
}

static void SaveMenu() 
{
	if (UICont->c.b || UICont->c.s) 
	{
		Abort_Save_SRAM();
		//~ BringUpGameMenu();
		MenuMode = &MainMenu;
	}
	else if (UICont->c.a) 
	{
		//We want "Saving..." to be displayed before saving, so render one frame here,
		//then on the next frame do the actual save in CommitSave
		void *pc = sqPrepare((void*)PVR_TA_INPUT);
		draw_rect_wh(24*2, 24*2, 320-48*2,240-48*2, 400, MENU_BG_COLOR, pc);
		puts_center(emufont, CENTER_Y - emufont->lineheight/2, "Saving...");
		MenuMode = &CommitSave;
		return;
	}
	
	puts_center(emufont, 50, "Saving to");
	puts_center(emufont, 50+emufont->lineheight, FriendlySaveDest);
	puts_center(emufont, 178, "(A) Save   (B) Don't Save");
	
	//~ if (Blocks_Required)
		BMFprintf(emufont, 100, 100, 500, "Blocks Required: %i\nFree Space: %i", Blocks_Required, Destination_Free_Space);
	
	void *pc = sqPrepare((void*)PVR_TA_INPUT);
	
	//Draw shadow
	draw_rect_wh(24*2, 24*2, 320-48*2,240-48*2, 400, MENU_BG_COLOR, pc);
	
	//~ puts_center(emufont, CENTER_Y - emufont->lineheight/2, "Nothing to save");
}

static const char *SRAMStr(const struct MenuItem *mi) 
{
	return Has_SRAM_Changed() ? "Save SRAM (SRAM changed)" : "Save SRAM (SRAM not changed)";
}

static void DoSave() 
{
	if (!Has_SRAM_Changed()) 
	{
		save_result = -1;
		MenuMode = &SaveResult;
	} 
	else 
	{
		if (Prepare_Save_SRAM()) 
		{
			MenuMode = &SaveMenu;
		} 
		else 
		{
			save_result = 0;
			MenuMode = &SaveResult;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
//Video Menu

static void ExitVideo() 
{
	MenuMode = &MainMenu;
}

static int VideoMenuSelection = 0;

static void VideoMenu() 
{
	static const struct MenuItem videomenu[] = 
	{
		{"", 0, &MyStr, 0, 0, 0},
		{"", 0, &MyStr, 0, 0, 0},
		{"Video Options", 0, &MyStr, 0, 0, 0},
		{"", 0, &MyStr, 0, 0, 0},
		//~ {"Line Draw", 1, &DrawStr, &NextDrawSetting, &PrevDrawSetting, &NextDrawSetting},
		{"Line Draw", 1, &NDrawStr, &NNextDrawSetting, &NPrevDrawSetting, &NNextDrawSetting},
		{"S/H En", 1, &SHStr, &ToggleSH, &ToggleSH, &ToggleSH},
		{"RPoint", 1, &RPointStr, &ToggleRenderPoint, &ToggleRenderPoint, &ToggleRenderPoint},
		//~ {"Show Prof", 1, &ProfStr, &ToggleProf, &ToggleProf, &ToggleProf},
		//~ {"Show Raster", 1, &ShowRasterStr, &ToggleShowRaster, &ToggleShowRaster, &ToggleShowRaster},
		//~ {"Show FPS", 1, &FPSStr, &ToggleFPS, &ToggleFPS, &ToggleFPS},
		{"", 0, &MyStr, 0, 0, 0},
		{"Display Debug", 1, &MyStr, &EnterDebugDisplay, 0, 0},
		//~ {"Layers", 1, &MyStr, &EnterLayerMenu, 0, 0},
		{"", 0, &MyStr, 0, 0, 0},
		{"Back", 1, &MyStr, &ExitVideo, 0, 0},
	};
	
	if (UICont->h.y)
	{
		return;
	}
	else if (UICont->c.b)
	{
		ExitVideo();
	}
	else if (UICont->c.s || UICont->c.au)
	{
		CloseGameMenu();
	}
	
	//Draw shadow
	draw_rect_wh(24, 24+LINE_HEIGHT, 320-48,240-48-LINE_HEIGHT, 400, MENU_BG_COLOR, sqPrepare((void*)PVR_TA_INPUT));
	
	ProcessMenu(videomenu, ARR_SIZE(videomenu), &VideoMenuSelection, 1);
}


////////////////////////////////////////////////////////////////////////////
//Main Menu

static void Reset(const struct MenuItem * mi) 
{
	Reset_Genesis();
}

static int MainMenuSelection = 0;

static const char save_state_str[2][12] = { "Save state", "Load state"};

static struct MenuItem mainmenu[] = 
{
	{ "", 				0, &MyStr,	 NULL, 					0, 0 },
	{ "Paused", 		0, &MyStr,	 NULL, 					0, 0 },
	{ "", 				0, &MyStr,	 NULL, 					0, 0 },
	{ "Resume Game", 	1, &MyStr,	 &CloseGameMenu, 		0, 0 },
	{ "Controls", 		1, &MyStr,	 &EnterControllerMenu,  0, 0 },
	{ "SRAM", 			1, &SRAMStr, &DoSave, 				0, 0 },
	{ "Hard Reset", 	1, &MyStr,	 &Reset, 				0, 0 },
	{ "Video Options", 	1, &MyStr,	 &EnterVideoMenu, 		0, 0 },
	{ "", 				0, &MyStr,	 NULL, 					0, 0 },
	{ "", 				0, &MyStr,	 NULL, 					0, 0 },
	{ "Exit Game", 		1, &MyStr,	 &ExitGame, 			0, 0 }
};

static void MainMenu()
{
	if (UICont->c.b || UICont->c.s || UICont->c.au) 
	{
		CloseGameMenu();
	}
	
	if (UICont->h.y)
	{
		return;
	}
	
	void *pc = sqPrepare((void*)PVR_TA_INPUT);
	
	//Draw shadow
	draw_rect_wh(24, 24, 320-48,240-48, 400, MENU_BG_COLOR, sqPrepare((void*)PVR_TA_INPUT));
	
	ProcessMenu(mainmenu, ARR_SIZE(mainmenu), &MainMenuSelection, 1);
}

extern int dev_mounted;
extern const char *current_game_dir;

static void DoSaveState(const struct MenuItem * mi)
{
	char SSname[512];
	char *RName = Game->Rom_Name;
	
	if (dev_mounted & 2)
	{
		sprintf(SSname, "/hdd/gens4all/savestate/%s.gs", RName);
	}
	else if (dev_mounted & 1)
	{
		sprintf(SSname, "/sd/gens4all/savestate/%s.gs", RName);
	}
	else
	{
		sprintf(SSname, "%s/savestate/%s.gs", ROM_PREFIX, RName);
	}
	
	save_result = Save_State(SSname) - 1;
	
	if (!save_result)
	{
		save_result = 1;
	}
	
	MenuMode = &SaveResult;
	
	void *pc = sqPrepare((void*)PVR_TA_INPUT);
	//Draw shadow
	draw_rect_wh(24*2, 24*2, 320-48*2,240-48*2, 400, MENU_BG_COLOR, pc);
}

static void DoLoadState(const struct MenuItem * mi)
{
	char SSname[512];
	char *RName = Game->Rom_Name;
	
	if (dev_mounted & 2)
	{
		sprintf(SSname, "/hdd/gens4all/savestate/%s.gs", RName);
	}
	else if (dev_mounted & 1)
	{
		sprintf(SSname, "/sd/gens4all/savestate/%s.gs", RName);
	}
	else
	{
		sprintf(SSname, "%s/savestate/%s.gs", ROM_PREFIX, RName);
	}
	
	Load_State(SSname);
}

void RunGameMenu() 
{
	MenuMode();
}

void Enable_SaveState()
{
	mainmenu[8].string = save_state_str[0];
	mainmenu[8].selectable = 1;
	mainmenu[8].picked = &DoSaveState;
	mainmenu[9].string = save_state_str[1];
	mainmenu[9].selectable = 1;
	mainmenu[9].picked = &DoLoadState;
}

