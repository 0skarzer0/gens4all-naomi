#include "gens4all.h"
#include "vdp_rend.h"
#include "blit.h"
#include "input.h"
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

#define ARR_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct MenuItem {
	const char *string;
	int selectable;
	const char * (*getstr)(const struct MenuItem *mi);
	void (*picked)();
	void (*left)();
	void (*right)();
};

static const char *MyStr(const struct MenuItem *mi) {
	return mi->string;
}
	
static float line(int y) {
	return 25 + y * emufont->lineheight;
}

static int string_length_pixels(struct BMFFont *font, const char * str) {
	int len = strlen(str);
	return BMFStringWidth(font, AsciiReader, str, len);
}

static void puts_center(struct BMFFont *font, float y, const char * str) {
	BMFputs(font, CENTER_X - string_length_pixels(font, str)/2, y, 500, AsciiReader, str);
}
static void ProcessMenu(const struct MenuItem *menu, int menusize, int *selection, float y) {
	int sel = *selection;
	
	//Make sure selection is in bounds and on a selectable item
	sel = sel < 0 ? 0 : sel;
	sel = sel >= menusize ? 0 : sel;
	while(!menu[sel].selectable) {
		sel++;
		sel = sel >= menusize ? 0 : sel;
	}
	
	//Move selection according to input
	if (UICont->c.d) {
		sel++;
		sel = sel >= menusize ? 0 : sel;
		while(!menu[sel].selectable) {
			sel++;
			sel = sel >= menusize ? 0 : sel;
		}
	}
	if (UICont->c.u) {
		sel--;
		while(!menu[sel].selectable) {
			sel--;
			sel = sel < 0 ? menusize-1 : sel;
		}
	}
	
	//Activate selection
	if (UICont->c.a) {
		if (menu[sel].picked != NULL) {
			menu[sel].picked();
		}
	} else if (UICont->c.l) {
		if (menu[sel].left != NULL) {
			menu[sel].left();
		}
	} else if (UICont->c.r) {
		if (menu[sel].right != NULL) {
			menu[sel].right();
		}
	}
	
	void *pc = sqPrepare((void*)PVR_TA_INPUT);
	
	//Draw selector
	draw_rect_wh(24, line(sel)-3 + y, 320-48, emufont->lineheight, 420, 0x80ff0000, pc);
	//Draw menu text
	int i;
	for(i = 0; i < menusize; i++) {
		puts_center(emufont, line(i) + y, menu[i].getstr(menu+i));
	}
	
	//Update selection
	*selection = sel;
}


static void MainMenu();
static void (*MenuMode)() = &MainMenu;

void BringUpGameMenu() {
	Paused = 1;
	//~ MenuMode = &MainMenu;
	Block_Hold();
	Block_Click();
}

void CloseGameMenu() {
	Paused = 0;
	//~ MenuMode = &MainMenu;
	Block_Click();
}

static void ExitGame() {
	dbgf("Exit\n");
	Gens_Running = 0;
	extern int Reset_Input_Lockout;
	Reset_Input_Lockout = 1;
}

////////////////////////////////////////////////////////////////////////////
//Debug
extern int Show_Timing;
extern int Show_Rasters;
extern int Enable_Shadow_Highlight;
static void ToggleProf() {
	Show_Timing = !Show_Timing;
}
static void ToggleRaster() {
	Show_Rasters = !Show_Rasters;
}
static void ToggleRenderPoint() {
	Render_Point = !Render_Point;
}
static void ToggleSH() {
	Enable_Shadow_Highlight = !Enable_Shadow_Highlight;
}
static const char *ProfStr(const struct MenuItem *mi) {
	return Show_Timing ? "Timing is shown" : "Timing is hidden";
}
static const char *RasterStr(const struct MenuItem *mi) {
	return Show_Rasters ? "Raster lines shown" : "Raster lines hidden";
}
static const char *RPointStr(const struct MenuItem *mi) {
	return Render_Point ? "Rendering at VBlank End" : "Rendering at VBlank Start";
}
static const char *SHStr(const struct MenuItem *mi) {
	return Enable_Shadow_Highlight ? "S/H Enabled" : "S/H Disabled";
}

////////////////////////////////////////////////////////////////////////////
//Controller
struct ControllerSetting {
	const char *name;
	EmuInputButtons *mapping;
};

extern EmuInputButtons ContMap4to3;
extern EmuInputButtons ContMap4to6;
extern EmuInputButtons ContMap6to6;
extern EmuInputButtons ContMap4toViking;
extern EmuInputButtons ContMap4toSF2;

static const struct ControllerSetting contsettings[] = {
	{"DC 4B to Gen 3B", &ContMap4to3},
	{"DC 4B to Gen 6B", &ContMap4to6},
	{"DC 4B to Gen 6B (SF2)", &ContMap4toSF2},
	{"DC 4B to Gen 6B (Viking)", &ContMap4toViking},
	{"DC 6B to Gen 6B", &ContMap6to6}, 
};
static int ControlSetting = 0;
static const char *ControlStr(const struct MenuItem *mi) {
	return contsettings[ControlSetting].name;
}
static void UpdateControlSetting() {
	extern EmuInputButtons *ContMap[MAX_CONTROLLERS];
	ContMap[0] = contsettings[ControlSetting].mapping;
	ContMap[1] = contsettings[ControlSetting].mapping;
	ContMap[2] = contsettings[ControlSetting].mapping;
	ContMap[3] = contsettings[ControlSetting].mapping;
	//Always use 6 button
	Controller_1_Type |= 1;
	Controller_2_Type |= 1;
	
	//Except for first option
	if (ControlSetting == 0) {
		Controller_1_Type &= ~1;
		Controller_2_Type &= ~1;
	}
		
}
static void NextControlSetting() {
	ControlSetting++;
	if (ControlSetting >= ARR_SIZE(contsettings))
		ControlSetting = 0;
	UpdateControlSetting();
}
static void PrevControlSetting() {
	ControlSetting--;
	if (ControlSetting < 0)
		ControlSetting = ARR_SIZE(contsettings)-1;
	UpdateControlSetting();
}

////////////////////////////////////////////////////////////////////////////
//Render setting
struct LineDrawSetting {
	const char *name;
	DrawRow drawa;
	DrawRow drawb;
};

extern DrawRow DrawRowB;
extern DrawRow DrawRowA;
void DrawMapRowAutoSplit4(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth);
void DrawMapRowAutoSplit2(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth);
void DrawMapRowAutoSplit1(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth);
void DrawMapRowAutoSplit4Cell(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth);
void DrawMapRowAutoSplit2Cell(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth);
void DrawMapRowAutoSplit1Cell(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth);
void DrawRowFastCell(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth);
void DrawRowFast(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth);

static const struct LineDrawSetting drawsettings[] = {
	{"Line Tile Auto Split (1 div)", &DrawMapRowAutoSplit1, &DrawMapRowAutoSplit1},
	{"Line Tile Auto Split (2 div)", &DrawMapRowAutoSplit2, &DrawMapRowAutoSplit2},
	{"Line Tile Auto Split (4 div A)", &DrawMapRowAutoSplit4, &DrawMapRowAutoSplit1},
	{"Line Tile Auto Split (4 div B)", &DrawMapRowAutoSplit1, &DrawMapRowAutoSplit4},
	{"Cell Scroll", &DrawRowFastCell, &DrawRowFastCell},
	{"Cell Scroll Auto Split (1 div)", &DrawMapRowAutoSplit1Cell, &DrawMapRowAutoSplit1Cell},
	{"Cell Scroll Auto Split (2 div)", &DrawMapRowAutoSplit2Cell, &DrawMapRowAutoSplit2Cell},
	{"Cell Scroll Auto Split (4 div A)", &DrawMapRowAutoSplit4Cell, &DrawMapRowAutoSplit1Cell},
	{"Cell Scroll Auto Split (4 div B)", &DrawMapRowAutoSplit1Cell, &DrawMapRowAutoSplit4Cell},
	{"Line Tilt", &DrawRowFast, &DrawRowFast},
};
static int DrawSetting = 1;
static const char *DrawStr(const struct MenuItem *mi) {
	return drawsettings[DrawSetting].name;
}
static void UpdateDrawSetting() {
	DrawRowB = drawsettings[DrawSetting].drawb;
	DrawRowA = drawsettings[DrawSetting].drawa;
}
static void NextDrawSetting() {
	DrawSetting++;
	if (DrawSetting >= ARR_SIZE(drawsettings))
		DrawSetting = 0;
	UpdateDrawSetting();
}
static void PrevDrawSetting() {
	DrawSetting--;
	if (DrawSetting < 0)
		DrawSetting = ARR_SIZE(drawsettings)-1;
	UpdateDrawSetting();
}

////////////////////////////////////////////////////////////////////////////
//SRAM
static int save_result;
static void SaveResult() {
	if (UICont->c.a || UICont->c.b || UICont->c.s) {
		BringUpGameMenu();
		//~ MenuMode = &MainMenu;
		MenuMode = &MainMenu;
	}
	
	void *pc = sqPrepare((void*)PVR_TA_INPUT);
	
	//Draw shadow
	draw_rect_wh(24*2, 24*2, 320-48*2,240-48*2, 400, 0xa0000000, pc);
	
	if (save_result == -1)
		puts_center(emufont, CENTER_Y - emufont->lineheight/2, "Nothing to save");
	else if (save_result == 0)
		puts_center(emufont, CENTER_Y - emufont->lineheight/2, "Save failed");
	else if (save_result == 1)
		puts_center(emufont, CENTER_Y - emufont->lineheight/2, "Save completed");
	else
		puts_center(emufont, CENTER_Y - emufont->lineheight/2, "Unknown result");
}
static void CommitSave() {
	save_result = Commit_Save_SRAM();
	MenuMode = &SaveResult;
	
	void *pc = sqPrepare((void*)PVR_TA_INPUT);
	//Draw shadow
	draw_rect_wh(24*2, 24*2, 320-48*2,240-48*2, 400, 0xa0000000, pc);
}
static void SaveMenu() {
	if (UICont->c.b || UICont->c.s) {
		Abort_Save_SRAM();
		//~ BringUpGameMenu();
		MenuMode = &MainMenu;
	}
	if (UICont->c.a) {
		//We want "Saving..." to be displayed before saving, so render one frame here,
		//then on the next frame do the actual save in CommitSave
		void *pc = sqPrepare((void*)PVR_TA_INPUT);
		draw_rect_wh(24*2, 24*2, 320-48*2,240-48*2, 400, 0xa0000000, pc);
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
	draw_rect_wh(24*2, 24*2, 320-48*2,240-48*2, 400, 0xa0000000, pc);
	
	//~ puts_center(emufont, CENTER_Y - emufont->lineheight/2, "Nothing to save");
}
static const char *SRAMStr(const struct MenuItem *mi) {
	return Has_SRAM_Changed() ? "Save SRAM (SRAM changed)" : "Save SRAM (SRAM not changed)";
}

static void DoSave() {
	if (!Has_SRAM_Changed()) {
		save_result = -1;
		MenuMode = &SaveResult;
	} else {
		if (Prepare_Save_SRAM()) {
			MenuMode = &SaveMenu;
		} else {
			save_result = 0;
			MenuMode = &SaveResult;
		}
	}
}


////////////////////////////////////////////////////////////////////////////
//Video Menu

static void ExitVideo() {
	MenuMode = &MainMenu;
}

static int VideoMenuSelection = 0;

static void VideoMenu() {
	static const struct MenuItem videomenu[] = {
		{"", 0, &MyStr, 0, 0, 0},
		{"Video Options", 0, &MyStr, 0, 0, 0},
		{"", 0, &MyStr, 0, 0, 0},
		{"Line Draw", 1, &DrawStr, &NextDrawSetting, &PrevDrawSetting, &NextDrawSetting},
		{"S/H En", 1, &SHStr, &ToggleSH, &ToggleSH, &ToggleSH},
		{"Show Prof", 1, &ProfStr, &ToggleProf, &ToggleProf, &ToggleProf},
		{"Show Raster", 1, &RasterStr, &ToggleRaster, &ToggleRaster, &ToggleRaster},
		{"RenderPoint", 1, &RPointStr, &ToggleRenderPoint, &ToggleRenderPoint, &ToggleRenderPoint},
		{"", 0, &MyStr, 0, 0, 0},
		{"", 0, &MyStr, 0, 0, 0},
		{"Back", 1, &MyStr, &ExitVideo, 0, 0},
	};
	
	if (UICont->c.b)
		ExitVideo();
	else if (UICont->c.s || UICont->c.au)
		CloseGameMenu();
	
	//Draw shadow
	draw_rect_wh(24, 24, 320-48,240-48, 400, 0xa0000000, sqPrepare((void*)PVR_TA_INPUT));
	
	ProcessMenu(videomenu, ARR_SIZE(videomenu), &VideoMenuSelection, 1);
}
static void EnterVideoMenu() {
	MenuMode = &VideoMenu;
}

////////////////////////////////////////////////////////////////////////////
//Main Menu
static int MainMenuSelection = 0;
static void MainMenu() {
	static const struct MenuItem mainmenu[] = {
		{"", 0, &MyStr, 0, 0, 0},
		{"Paused", 0, &MyStr, 0, 0, 0},
		{"", 0, &MyStr, 0, 0, 0},
		{"Resume Game", 1, &MyStr, &CloseGameMenu, 0, 0},
		{"Controls", 1, &ControlStr, &NextControlSetting, &PrevControlSetting, &NextControlSetting},
		{"SRAM", 1, &SRAMStr, &DoSave, 0, 0},
		{"Hard Reset", 1, &MyStr, &Reset_Genesis, 0, 0},
		{"Video Options", 1, &MyStr, &EnterVideoMenu, 0, 0},
		{"", 0, &MyStr, 0, 0, 0},
		{"", 0, &MyStr, 0, 0, 0},
		{"Exit Game", 1, &MyStr, &ExitGame, 0, 0},
	};
	
	if (UICont->c.b || UICont->c.s || UICont->c.au)
		CloseGameMenu();
	
	
	void *pc = sqPrepare((void*)PVR_TA_INPUT);
	
	//Draw shadow
	draw_rect_wh(24, 24, 320-48,240-48, 400, 0xa0000000, sqPrepare((void*)PVR_TA_INPUT));
	
	ProcessMenu(mainmenu, ARR_SIZE(mainmenu), &MainMenuSelection, 1);
}

void RunGameMenu() {
	MenuMode();
}

