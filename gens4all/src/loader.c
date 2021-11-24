#include <assert.h>
#include <kos.h>
#include <math.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "bmf.h"
#include "gamelist.h"
#include "pvrvdp/npvr_vertex.h"
#include "pvrvdp/sq.h"
#include "util/save.h"

#define LOADER_MODE game_list

extern void watchdog_pet();

static void game_list();
extern void * draw_rect_wh(float x, float y, float w, float h, float z, unsigned int argb, void *dst);

static pvr_init_params_t loader_pvr_params = {
	{ PVR_BINSIZE_8, PVR_BINSIZE_0, PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16 },
	(int)(0.5*1024*1024),
	0,
	0,
	0, //sort
};

typedef struct {
	int a,b,c,x,y,z,s;
	int u,d,l,r;
} LoaderInputs;

LoaderInputs click;
LoaderInputs hold;
LoaderInputs last;
LoaderInputs holdtime;

GameListEntry *GLCurGame;

int Reset_Input_Lockout = 0;

static struct {
	float left, top, right, bottom;
} Disp = {40, 40, 640-40, 480-40};

static void read_cont() {
	maple_device_t *contr = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
	if (!contr)
		return;
	cont_state_t *cond = (cont_state_t *)maple_dev_status(contr);
	if (!cond)
		return;
	
	#define CheckMapleButton(condition, var) \
		do { var = !!(condition); } while(0)
	#define CheckClick(button) do { if (hold.button && !last.button) { click.button = 1; holdtime.button = 20; } else { click.button = 0; } } while(0)
	#define CheckRepeat(button) do { \
		if (hold.button && holdtime.button-- == 0) { \
			holdtime.button = 1; click.button = 1; \
		} } while(0)
	
	memcpy(&last, &hold, sizeof(last));
	
	if (Reset_Input_Lockout && cond->buttons != 0) {
		//If the user exitted the game with ABXY+Start, wait until the user
		//releases all buttons before responding
		memset(&hold, 0, sizeof(hold));
		memset(&click, 0, sizeof(hold));
		return;
	} else {
		Reset_Input_Lockout = 0;
	}
	
	CheckMapleButton(cond->buttons & CONT_START, hold.s);
	CheckMapleButton(cond->buttons & CONT_A, hold.a);
	CheckMapleButton(cond->buttons & CONT_B, hold.b);
	CheckMapleButton(cond->buttons & CONT_X, hold.x);
	CheckMapleButton(cond->buttons & CONT_Y, hold.y);
	CheckMapleButton(cond->buttons & CONT_DPAD_UP, hold.u);
	CheckMapleButton(cond->buttons & CONT_DPAD_DOWN, hold.d);
	CheckMapleButton(cond->buttons & CONT_DPAD_LEFT, hold.l);
	CheckMapleButton(cond->buttons & CONT_DPAD_RIGHT, hold.r);
	
	CheckClick(a);
	CheckClick(b);
	CheckClick(x);
	CheckClick(y);
	CheckClick(s);
	CheckClick(u);
	CheckClick(d);
	CheckClick(l);
	CheckClick(r);
	
	CheckRepeat(u);
	CheckRepeat(d);
	CheckRepeat(l);
	CheckRepeat(r);
}

static struct BMFFont * font;
static struct BMFFont * titlefont;

#define MAXDIRENTRIES	(1000)
#define NAMEBUFSIZE	(100*1024)

typedef struct {
	int is_dir;
	const char *name;
} LoadDirEntry;

static int ListingDirMode = 1;
static char dirnamebuff[NAMEBUFSIZE];
static int direntrycnt;
static LoadDirEntry direntries[MAXDIRENTRIES];
static GameListHeader *gl = (GameListHeader*)&dirnamebuff;
static GameListEntry *games = ((GameListHeader*)&dirnamebuff)->games;

#define MAX_DIRNAME	500
//~ static char curdir[MAX_DIRNAME] = "/";
static char curdir[MAX_DIRNAME] = "/pc/home/alex/Emu/Genesis/sm";
//~ static char curdir[MAX_DIRNAME] = "/pc/home/alex/Emu/Genesis";

static int IsDir(const char *name) {
	struct stat dstat;
	memset(&dstat, 0, sizeof(dstat));
	stat(name, &dstat);
	
	return S_ISDIR(dstat.st_mode);
}

static int string_length_pixels(struct BMFFont *font, const char * str) {
	int len = strlen(str);
	return BMFStringWidth(font, AsciiReader, str, len);
}

static void puts_center(struct BMFFont *font, float y, const char * str) {
	BMFputs(font, 320 - string_length_pixels(font, str)/2, y, 1, AsciiReader, str);
}
static void puts_center_x(struct BMFFont *font, float x, float y, float z, int color, const char * str) {
	BMFcputs(font, x - string_length_pixels(font, str)/2, y, z, color, AsciiReader, str);
}
int dircmp (const void *a, const void *b) {
	const LoadDirEntry *ad = a;
	const LoadDirEntry *bd = b;
	return strcmp(ad->name, bd->name);
}
static void LoadGameList(const char *list) {
	ListingDirMode = 0;
	
	printf("Opening: %s\n", list);
	int f = open(list, O_RDONLY);
	//~ if (errno == 
	perror("Open: ");
	size_t readamt = read(f, dirnamebuff, sizeof(dirnamebuff));
	perror("Read: ");
	close(f);
	
	printf("Readamt: %i\n", (int)readamt);
	fflush(stdout);
	
	//TODO make sure the list fits
	if (readamt >= sizeof(dirnamebuff)) {
		exit(0);
	}
	
	assert(memcmp(gl->fourcc, GameListHeaderFourCC, 4) == 0);
	assert(gl->version == 1);
	assert(gl->game_cnt < 500);
	assert(gl->game_cnt > 0);
	
	//Adjust string pointers
	int str_ofs = (int)&dirnamebuff;
	int i;
	for(i = 0; i < gl->game_cnt; i++) {
		uintptr_t str = (uintptr_t)games[i].strings;
		//~ printf("%i %p: %08x %i\n", i, (void*)(games+i) - (void*)dirnamebuff, str, str);
		assert(str < readamt);
		games[i].strings += str_ofs;
	}
	
	printf("Switching to dir: %s\n",gl->dir);
	strcpy(curdir, gl->dir);
	chdir(curdir);
	
	printf("Header size: %i\n", sizeof(GameListHeader));
	printf("Game entry: %i\n", sizeof(GameListEntry));
}
static const char * NextString(const char * str, int idx) {
	while(idx--) {
		str = strrchr(str, 0) + 1;
	}
	return str;
}
static const char * GetGameName(int idx) {
	return games[idx].strings;
}
static const char * GetRomName(int idx) {
	return NextString(games[idx].strings, 1);
}
static const char * GetSaveTitle(int idx) {
	return NextString(games[idx].strings, 2);
}
static const char * GetSaveVMUTitle(int idx) {
	return NextString(games[idx].strings, 3);
}

const char * GetCurGameSaveTitle() {
	if (GLCurGame)
		return NextString(GLCurGame->strings, 2);
	else
		return "Gens4All Save Data";
}
const char * GetCurGameSaveVMUTitle() {
	if (GLCurGame)
		return NextString(GLCurGame->strings, 3);
	else
		return "Gens4All Save Data";
}

static int LoadDir(const char *dir) {
	DIR *d;
	
	int dir_is_root = strcmp(dir, "/") == 0;
	
	chdir(dir);
	d = opendir(".");

	if (d == NULL) {
		chdir(curdir);
		return -1;
	}
	
	if (1) {
		read_cont();
		pvr_wait_ready();
		pvr_scene_begin();
		pvr_list_begin(PVR_LIST_TR_POLY);
		puts_center(font, 240 - font->lineheight, "Getting listing of");
		puts_center(font, 240 + font->lineheight, dir);
		//~ BMFprintf(font, 50, 240 - font->lineheight, 1, "Getting listing of\n%s", dir);
		pvr_list_finish();
		pvr_scene_finish();
	}
	
	int cnt = 0;
	int namepos = 0;
	
	ListingDirMode = 1;
	
	struct dirent *de = readdir(d);
	while(de) {
		if (cnt >= MAXDIRENTRIES)
			break;
		
		//Don't list hidden files
		if (de->d_name[0] != '.') {
			//Record name pointer and dir-ness
			//KOS says nothing in root counts as a directory. If we are moving into root, treat it as a directory anyways.
			direntries[cnt].is_dir = dir_is_root || IsDir(de->d_name);
			direntries[cnt].name = dirnamebuff + namepos;
			
			//Save name to buffer
			int namelen = strlen(de->d_name);
			memcpy(dirnamebuff + namepos, de->d_name, namelen);
			dirnamebuff[namepos + namelen] = 0;
			namepos += namelen+2;
			
			cnt++;
		}
		watchdog_pet();	//Listing PC dirs over BBA is slow
		de = readdir(d);
	}

	closedir(d);
	
	strcpy(curdir, dir);
	direntrycnt = cnt;
	
	if (SORTDIR)
		qsort(direntries, cnt, sizeof(LoadDirEntry), &dircmp);
	
	return 0;
}

static void init_loader_video() {
	pvr_shutdown();
	vid_init(DM_640x480, PM_RGB565);
	pvr_init(&loader_pvr_params);
	pvr_set_bg_color(0,0,.1);
	
	chdir("/rd/");
	font = BMFLoad("notosans.fnt", "vq");
	titlefont = BMFLoad("genesis.fnt", "vq");
	chdir(curdir);
}

static void (*browser_mode)();
static void (*loader_mode)();

static void update_save();

static int linesvisible = 13;
static int cursorpos = 0;
static int viewtop = 0;

void DrawRectAlpha(float x, float y, float xb, float yb, float z, int color) {
	pvr_context cxt;
	pvr_vertex64s spr;
	pc_set_default_sprite(&cxt);
	pc_set_sprite_color_packed(&cxt, color);
	pv_setr_cmd_vertex_eos(spr);
	pv_setr_rect(spr, x, y, xb, yb, z);
	pvr_prim(&cxt, sizeof(cxt));
	pvr_prim(&spr, sizeof(spr));
}
void DrawRectAlphaWH(float x, float y, float w, float h, float z, int color) {
	DrawRectAlpha(x,y, x+w, y+h, z, color);
}

typedef void (*PrintListName)(float y, int idx);
static void DrawList(PrintListName printname, int size) {
	float top = Disp.top;
	
	//Draw directory list
	int i;
	float y = top + font->lineheight;
	int linesdrawn = 0;
	if ((viewtop + linesvisible) > size) {
		viewtop = size - linesvisible-2;
	}
	if (viewtop < 0) viewtop = 0;
	for(i = viewtop; i < size; i++) {
		//~ if (direntries[i].is_dir)
			//~ BMFprintf(font, left, y, 1, "[D]");
		
		printname(y, i);
		//~ BMFprintf(font, left + dirmarkwidth, y, 1, " %s", 
			//~ GetGameName(i));
		
		y += font->lineheight;
		
		if (linesdrawn++ > linesvisible)
			break;
	}
	
	//Draw cursor
	float cursory = top + (cursorpos-viewtop+1) * font->lineheight;
	//BMFprintf(font, left-16, cursory, 1, ">");
	
	DrawRectAlphaWH(0, cursory, 640, font->lineheight, 0.5, 0xff601010);
	
	//Scroll bar
	float scrollx = 570;
	float percentage = (float)cursorpos/(size-1);
	float scrmargin = 40;
	float boxheight = 20;
	float scrhieght = 480 - scrmargin*2 - boxheight;
	float box_y = percentage * scrhieght + scrmargin;
	DrawRectAlphaWH(scrollx, box_y, 16, boxheight, 0.7, pv_pack_argb(.7, 0, 1, 0));
	DrawRectAlphaWH(scrollx-8, scrmargin-8, 32, scrhieght+boxheight+16, 0.6, pv_pack_argb(0.8, .3, .3, .3));
}

void UpdateListCursor(int size) {
	//Move cursor
	if (click.d) cursorpos++;
	if (click.u) cursorpos--;
	if (click.r) cursorpos += linesvisible+1;
	if (click.l) cursorpos -= linesvisible+1;
	
	//Keep cursor in list
	if (cursorpos < 0) cursorpos = 0;
	if (cursorpos >= size) cursorpos = size-1;
	
	//Move view to show cursor
	if (cursorpos < viewtop) viewtop = cursorpos;
	if (cursorpos > (viewtop+linesvisible+1)) viewtop = cursorpos - linesvisible-1;
	if (viewtop < 0) viewtop = 0;
}



void PrintGameListName(float y, int idx) {
	BMFprintf(font, 40+30, y, 1, " %s", GetGameName(idx));
}
static float dirmarkwidth;
void PrintDirName(float y, int idx) {
	if (direntries[idx].is_dir)
		BMFprintf(font, 40, y, 1, "[D]");
	BMFprintf(font, 40 + dirmarkwidth, y, 1, " %s", 
		direntries[idx].name);
}

static void DrawGameList() {
	
	DrawList(&PrintGameListName, gl->game_cnt);
	return;
}

static struct {
	int x, y;
} IndexSelection;
static const char * IndexOptions[] = {
	"#", "A", "B", "C", "D", "E", "F", 
	"G", "H", "I", "J", "K", "L", "M",
	"N", "O", "P", "Q", "R", "S", "T", 
	"U", "V", "W", "X", "Y", "Z", " "};

#define IDX_SIZE	27
#define IDX_COLS	6
#define IDX_ROWS	5
#define IDX_LAST_ROW	(IDX_ROWS-1)
#define IDX_LAST_ROW_COLS	(IDX_SIZE % IDX_COLS)
static void GameListIndex() {
	int selectid = IndexSelection.y * IDX_COLS + IndexSelection.x;
	if (click.a && gl->index[selectid] != -1 && selectid < IDX_SIZE) {
		cursorpos = gl->index[selectid];
		viewtop = cursorpos - linesvisible/2;
		loader_mode = &game_list;
		return;
	}
	if (click.b) {
		loader_mode = &game_list;
		return;
	}
	if (click.u) IndexSelection.y--;
	if (click.d) IndexSelection.y++;
	if (click.l) IndexSelection.x--;
	if (click.r) IndexSelection.x++;
	if (IndexSelection.x < 0) IndexSelection.x = IDX_COLS-1;
	if (IndexSelection.x >= IDX_COLS) IndexSelection.x = 0;
	if (IndexSelection.y < 0) IndexSelection.y = IDX_ROWS-1;
	if (IndexSelection.y >= IDX_ROWS) IndexSelection.y = 0;
	if (IndexSelection.y == IDX_LAST_ROW) {
		if (IndexSelection.x >= IDX_LAST_ROW_COLS) {
			if (click.r) IndexSelection.x = 0;
			if (click.l) IndexSelection.x = IDX_LAST_ROW_COLS-1;
			if (click.d) IndexSelection.y = 0;
			if (click.u) IndexSelection.y = IDX_LAST_ROW-1;
			
		}
	}
	
	pvr_set_bg_color(0,0,.1);
	pvr_wait_ready();
	pvr_scene_begin();
	pvr_list_begin(PVR_LIST_TR_POLY);
	
	DrawGameList();
	
	const struct BMFCharacter *c = BMFFindCharacter(font, 'M');
	float charh = font->lineheight;
	float charwidth = c->xadvance;
	
	int winlines = IDX_ROWS+1;
	float winheight = winlines * charh;
	float topedge = (480 - winheight) / 2;
	
	float windowwidth = charwidth * (IDX_COLS * 2);
	float leftedge = (640 - windowwidth) / 2;
	float leftedgec = leftedge + charwidth / 2;
	
	//~ draw_rect_wh(100, 100, 640-200, 480 - 200, 2, 0xff000000, sqPrepare((void*)PVR_TA_INPUT));
	pvr_context bar_cxt;
	pvr_vertex64s bar_spr;
	pc_set_default_sprite(&bar_cxt);
	
	//Draw background
	//~ pv_setr_rect(bar_spr, leftedge-charwidth*1.1, topedge, leftedge+windowwidth+charwidth*1.1, topedge + winheight, 2);
	DrawRectAlphaWH(leftedge-charwidth*1., topedge-charh*.5, windowwidth+charwidth*1.2, winheight+charh, 2, pv_pack_argb(.8, .2, .2, .2));
	
	//Draw selection
	float selectionx = IndexSelection.x * charwidth * 2 + leftedge;
	float selectiony = IndexSelection.y * font->lineheight + topedge;
	DrawRectAlphaWH(selectionx - charwidth*.1, selectiony, charwidth*1.2, charh, 2.5, pv_pack_argb(.5, .8, .8, 0));
	
	//Draw letters
	int i;
	for(i = 0; i < IDX_SIZE; i++) {
		puts_center_x(font,
			(i%IDX_COLS) * charwidth*2 + leftedgec, (i/IDX_COLS) * font->lineheight + topedge, 3,
			gl->index[i] != -1 ? 0xffffffff : 0x80d0d0d0,
			IndexOptions[i]);
	}
	
	puts_center_x(font, 320,
		topedge + IDX_ROWS * font->lineheight, 3, -1,
		"(A) Jump   (B) Back");
	
	pvr_list_finish();
	pvr_scene_finish();
}

static void RunGame(const char *fname) {
	//Shut down loader renderer
	pvr_wait_ready();
	BMFFree(font);
	BMFFree(titlefont);
	pvr_shutdown();
	
	//Bring up Gens4All
	Init_Video_SubSystem(0,0,0);
	extern int Gens_Running;
	Gens_Running = 1;
	
	//~ printf("Playing \"%s\"\n", GetRomName(cursorpos));
	
	extern int run(const char *rom_filename);
	int runresult = run(fname);
	
	//Restore renderer
	init_loader_video();
	
	if (Has_SRAM_Changed()) {
		if (Prepare_Save_SRAM()) {
			loader_mode = &update_save;
			return;
		}
	}
}

static void game_list() {
	if (ListingDirMode) {
		LoadGameList(GAMELISTFNAME);
	}
		
	if (click.x)
		loader_mode = &GameListIndex;
	if (click.s)
		exit(0);
	
	UpdateListCursor(gl->game_cnt);
	
	//Start a game
	if (click.a) {
		extern void SetControls(int controlidx);
		SetControls(games[cursorpos].DefaultControls);
		GLCurGame = games + cursorpos;
		
		RunGame(GetRomName(cursorpos));
		return;
	}
	
	pvr_set_bg_color(0,0,.1);
	pvr_wait_ready();
	pvr_scene_begin();
	pvr_list_begin(PVR_LIST_TR_POLY);
	
	
	BMFprintf(font, 450, Disp.top, 1, "(X) Index");
	DrawGameList();
	
	pvr_list_finish();
	pvr_scene_finish();
}

static void navigate_dir() {
	if (!ListingDirMode)
		LoadDir(curdir);
	
	dirmarkwidth = BMFStringWidth(font, AsciiReader, "[D]", 3);
	
	if (click.s)
		exit(0);
	
	UpdateListCursor(direntrycnt);
	
	//Go up directory
	if (click.b) {
		char *lastslash = strrchr(curdir, '/');
		if (lastslash) {
			//Drop the last directory level, and it's slash, off
			int parentdirnamelen = lastslash - curdir;
			char parentdir[NAMEBUFSIZE];
			memcpy(parentdir, curdir, parentdirnamelen);
			parentdir[parentdirnamelen] = 0;
			
			//Are we moving into the root directory? If we dropped the rightmost slash off and have nothing left, we are moving into root.
			if (!LoadDir(parentdirnamelen ? parentdir : "/")) {
				//If we move into a new directory, reset the cursor and view position
				cursorpos = 0;
				viewtop = 0;
			}
		}
	}
	
	//Go into directory
	if (click.a && direntries[cursorpos].is_dir) {
		char newdir[NAMEBUFSIZE];
		//Don't double up the slash if we are moving out of root
		if (strcmp(curdir, "/") != 0)
			snprintf(newdir, NAMEBUFSIZE, "%s/%s", curdir, direntries[cursorpos].name);
		else
			snprintf(newdir, NAMEBUFSIZE, "/%s", direntries[cursorpos].name);
		
		if (!LoadDir(newdir)) {
			//If we move into a new directory, reset the cursor and view position
			cursorpos = 0;
			viewtop = 0;
		}
	}
	
	//Start a game
	if (click.a && !direntries[cursorpos].is_dir) {
		GLCurGame = NULL;
		RunGame(direntries[cursorpos].name);
		return;
	}
	
	pvr_set_bg_color(0,0,.1);
	pvr_wait_ready();
	pvr_scene_begin();
	pvr_list_begin(PVR_LIST_TR_POLY);
	
	
	DrawList(&PrintDirName, direntrycnt);
	
	
	pvr_list_finish();
	pvr_scene_finish();
	return;
}

int save_result;
static void display_save_result() {
	if (click.b || click.a || click.s)
		loader_mode = browser_mode;
	
	pvr_set_bg_color(0,0,.1);
	pvr_wait_ready();
	pvr_scene_begin();
	pvr_list_begin(PVR_LIST_TR_POLY);
	
	if (save_result == -1)
		puts_center(font, 240 - font->lineheight/2, "Not Saved");
	else if (save_result == 0)
		puts_center(font, 240 - font->lineheight/2, "Save Failed");
	else if (save_result == 1)
		puts_center(font, 240 - font->lineheight/2, "Save Completed");
	else
		puts_center(font, 240 - font->lineheight/2, "Unknown result");
	
	pvr_list_finish();
	pvr_scene_finish();
}

static void update_save() {
	if (Blocks_Required <= Destination_Free_Space) {
		if (click.b) {
			Abort_Save_SRAM();
			save_result = -1;
			loader_mode = &display_save_result;
		}
		if (click.a) {
			pvr_set_bg_color(0,0,.1);
			pvr_wait_ready();
			pvr_scene_begin();
			pvr_list_begin(PVR_LIST_TR_POLY);
			
			puts_center(font, 150, "Now Saving...");
			
			pvr_list_finish();
			pvr_scene_finish();
			
			save_result = Commit_Save_SRAM();
			loader_mode = &display_save_result;
			return;
		}
	} else {
		if (click.b || click.a || click.s) {
			Abort_Save_SRAM();
			loader_mode = LOADER_MODE;
		}
	}
	
	pvr_set_bg_color(0,0,.1);
	pvr_wait_ready();
	pvr_scene_begin();
	pvr_list_begin(PVR_LIST_TR_POLY);
	
	puts_center(font, 120, "Save Changed");
	
	puts_center(font, 180, "Saving to");
	puts_center(font, 180+font->lineheight, FriendlySaveDest);
	
	if (Blocks_Required)
		BMFprintf(font, 200, 300, 1, "Blocks Required: %i\nFree Space: %i", Blocks_Required, Destination_Free_Space);
	
	if (Blocks_Required <= Destination_Free_Space) {
		puts_center(font, 400, "(A) Save   (B) Don't Save");
	} else {
		puts_center(font, 400, "Not enough space to save");
	}
	//~ BMFprintf(font, left + dirmarkwidth, y, 1, " %s", direntries[i].name);
	
	pvr_list_finish();
	pvr_scene_finish();
}

static void go_to_filelist() {
	browser_mode = loader_mode = navigate_dir;
	strcpy(curdir, "/");
	LoadDir(curdir);
}
static void go_to_gamelist() {
	//~ printf("Using game list from: "GAMELISTFNAME"\n");
	//~ fflush(stdout);
	FILE *f = fopen(GAMELISTFNAME, "r");
	//~ perror("");
	//~ printf("F: %p\n", f);
	//~ fflush(stdout);
	if (f) {
		fclose(f);
		browser_mode = loader_mode = game_list;
		//~ chdir(GAMELISTDIR);
		LoadGameList(GAMELISTFNAME);
	} else {
		go_to_filelist();
	}
}

static void exit_title_screen() {
	if (GAMELISTLOADER) {
		go_to_gamelist();
	} else {
		go_to_filelist();
	}
}

static void title_screen() {
	static float anim = 0;
	int i;
	
	if (click.s) {
		if (!hold.a && GAMELISTLOADER) {
			exit_title_screen();
		} else {
			go_to_filelist();
		}
		return;
	}
	
	anim += 0.05;
	if (anim > (M_PI * 4))
		anim -= M_PI * 4;
	
	pvr_set_bg_color(.1,.1,.1);
	pvr_wait_ready();
	pvr_scene_begin();
	pvr_list_begin(PVR_LIST_TR_POLY);
	
	//Gens4All title
	struct BMFSprite logo[8];
	BMFprintfmem(titlefont, 320 - string_length_pixels(titlefont, "Gens4All") / 2, 140, 1, logo, "Gens4All");
	for(i = 0; i < 8; i++) {
		float s = sin(anim + (float)i/3) / 4 + 0.5;
		pc_set_sprite_color(&logo[i].cxt, 1, s+.25, s+.25, s);
	}
	pvr_prim(logo, sizeof(logo));
	
	//Version
	puts_center(font, 180, "Accelerated Renderer");
	puts_center(font, 180 + font->lineheight, "Release 4.0");
	
	//Press Start
	puts_center(font, 340, "Press Start");
	
	//Background grid
	int x,y, size = 40;
	float offsetx = -20 - anim / 2 / M_PI * 40, offsety = -20;
	pvr_context rectcxt;
	pvr_vertex64s rectv;
	
	pc_set_default_sprite(&rectcxt);
	pc_set_sprite_color(&rectcxt, 1, 0.15, 0.15, 0.15);
	pvr_prim(&rectcxt, sizeof(rectcxt));
	for(x = 0; x < 640/size+3; x++) {
		for(y = 0; y < 480/size+1; y++) {
			if ((x^y) & 1) {
				pv_setr_cmd_vertex_eos(rectv);
				pv_setr_rect(rectv, x*size+offsetx, y*size+offsety, x*size+size+offsetx, y*size+size+offsety, 0.1);
				pvr_prim(&rectv, sizeof(rectv));
			}
		}
	}
	
	pvr_list_finish();
	pvr_scene_finish();
}

void run_loader() {
	init_loader_video();
	
	if (SKIPTITLE) {
		exit_title_screen();
	} else {
		loader_mode = title_screen;
	}

	do {
		read_cont();
		
		loader_mode();
		
		watchdog_pet();
	} while(1);
	
	exit(0);	
}
