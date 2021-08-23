#include <kos.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>

#include "bmf.h"
#include "pvrvdp/npvr_vertex.h"

extern void watchdog_pet();

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

static int ButtonA, ButtonB;
static int DpadUp, DpadDown, DpadLeft, DpadRight;
int Reset_Input_Lockout = 0;

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

char dirnamebuff[NAMEBUFSIZE];
int direntrycnt;
LoadDirEntry direntries[MAXDIRENTRIES];

#define MAX_DIRNAME	500
static char curdir[MAX_DIRNAME] = "/";

static int IsDir(const char *name) {
	struct stat dstat;
	memset(&dstat, 0, sizeof(dstat));
	stat(name, &dstat);
	
	return S_ISDIR(dstat.st_mode);
}

int string_length_pixels(struct BMFFont *font, const char * str) {
	int len = strlen(str);
	return BMFStringWidth(font, AsciiReader, str, len);
}

void puts_center(struct BMFFont *font, float y, const char * str) {
	BMFputs(font, 320 - string_length_pixels(font, str)/2, y, 1, AsciiReader, str);
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

static void (*loader_mode)();

static void navigate_dir() {
	static int cursorpos = 0;
	static int viewtop = 0;
	int linesvisible = 13;
	//~ int linesvisible = 480 / font->lineheight - 4;
	
	float dirmarkwidth = BMFStringWidth(font, AsciiReader, "[D]", 3);
	
	if (click.s)
		exit(0);
	
	//Move cursor
	if (click.d) cursorpos++;
	if (click.u) cursorpos--;
	if (click.r) cursorpos += linesvisible+1;
	if (click.l) cursorpos -= linesvisible+1;
	
	//Keep cursor in list
	if (cursorpos < 0) cursorpos = 0;
	if (cursorpos >= direntrycnt) cursorpos = direntrycnt-1;
	
	//Move view to show cursor
	if (cursorpos < viewtop) viewtop = cursorpos;
	if (cursorpos > (viewtop+linesvisible+1)) viewtop = cursorpos - linesvisible-1;
	if (viewtop < 0) viewtop = 0;
	
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
	if (click.x || (click.a && direntries[cursorpos].is_dir)) {
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
		//Shut down loader renderer
		BMFFree(font);
		BMFFree(titlefont);
		pvr_shutdown();
		
		//Bring up Gens4All
		Init_Video_SubSystem(0,0,0);
		extern int Gens_Running;
		Gens_Running = 1;
		
		extern int run(const char *rom_filename);
		int runresult = run(direntries[cursorpos].name);
		
		//Restore renderer
		init_loader_video();
	}
	
	pvr_set_bg_color(0,0,.1);
	pvr_wait_ready();
	pvr_scene_begin();
	pvr_list_begin(PVR_LIST_TR_POLY);
	
	float top = 30;
	float left = 40;
	
	//Draw current dir
	BMFprintf(font, left, top, 1, "\"%s\"", curdir);
	
	//Draw directory list
	int i;
	float y = top + font->lineheight;
	int linesdrawn = 0;
	for(i = viewtop; i < direntrycnt; i++) {
		if (direntries[i].is_dir)
			BMFprintf(font, left, y, 1, "[D]");
		
		BMFprintf(font, left + dirmarkwidth, y, 1, " %s", 
			direntries[i].name);
		
		y += font->lineheight;
		
		if (linesdrawn++ > linesvisible)
			break;
	}
	
	//Draw cursor
	float cursory = top + (cursorpos-viewtop+1) * font->lineheight;
	//BMFprintf(font, left-16, cursory, 1, ">");
	
	pvr_context bar_cxt;
	pvr_vertex64s bar_spr;
	pc_set_default_sprite(&bar_cxt);
	pc_set_sprite_color(&bar_cxt, 1, .4, .1, .1);
	pv_setr_cmd_vertex_eos(bar_spr);
	pv_setr_rect(bar_spr, 0, cursory, 640, cursory + font->lineheight, 0.5);
	pvr_prim(&bar_cxt, sizeof(bar_cxt));
	pvr_prim(&bar_spr, sizeof(bar_spr));
	
	//Scroll bar
	float scrollx = 570;
	float percentage = (float)cursorpos/(direntrycnt-1);
	float scrmargin = 40;
	float boxheight = 20;
	float scrhieght = 480 - scrmargin*2 - boxheight;
	float box_y = percentage * scrhieght + scrmargin;
	pc_set_sprite_color(&bar_cxt, .7, 0, 1, 0);
	pv_setr_rect(bar_spr, scrollx, box_y, scrollx+16, box_y+boxheight, 0.7);
	pvr_prim(&bar_cxt, sizeof(bar_cxt));
	pvr_prim(&bar_spr, sizeof(bar_spr));
	
	pc_set_sprite_color(&bar_cxt, 0.8, .3, .3, .3);
	pv_setr_rect(bar_spr, scrollx-8, scrmargin-8, scrollx+16+8, scrmargin+scrhieght+boxheight+8, 0.6);
	pvr_prim(&bar_cxt, sizeof(bar_cxt));
	pvr_prim(&bar_spr, sizeof(bar_spr));
	
	pvr_list_finish();
	pvr_scene_finish();
}

static void title_screen() {
	static float anim = 0;
	int i;
	
	if (click.s) {
		loader_mode = navigate_dir;
		LoadDir(curdir);
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
	puts_center(font, 180 + font->lineheight, "Release 2");
	
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
	
	loader_mode = title_screen;

	do {
		read_cont();
		
		loader_mode();
		
		watchdog_pet();
	} while(1);
	
	exit(0);	
}
