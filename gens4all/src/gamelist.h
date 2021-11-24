#ifndef GAMELIST_H
#define GAMELIST_H

#include <stdint.h>

#define GGL_MAX_DIR_LENGTH	(100)

#define GGL_NUM_INDICIES	(27)	//1 per letter, plus one for numbers
typedef short GGL_Index;

enum GL_SampleRate {
	SRAT_8000,
	SRAT_11025,
	SRAT_16000,
	SRAT_22050,
	SRAT_32000,
	SRAT_44100,
};
enum GL_SRAMType {
	SAV_AUTO,
	SAV_NONE,
	SAV_ODD_BYTES_SRAM,
	SAV_EVEN_BYTES_SRAM,
	SAV_WORD_SRAM,
	SAV_EEPROM,
};
enum GL_DefControls {
	DCON_3BUTTON,
	DCON_3BUTTON_REV,
	DCON_6BUTTON,
	DCON_6BUTTON_LV,
	DCON_6BUTTON_SF2,
	DCON_TEAMPLAYER,
	DCON_4WAYPLAY,
	DCON_MENACER,
	DCON_JUSTIFIER,
};
enum GL_Multitap {
	DMT_NONE,
	DMT_TEAMPLAYER,
	DMT_4WAYPLAY,
	DMT_JCART,
};
enum GL_Hacks {
	GH_SONIC_Z80_HACK,
	GH_RISTAR_HACK,
	GH_SONIC2_AUTO_RENDER_HACK,
	GH_SCREEN_OFF_RENDER_HACK,
	GH_TOTAL_HACKS
};

typedef struct {
	uint32_t SampleRate : 3;	//GL_SampleRate
	uint32_t SRAM : 3;	//GL_SRAMType
	uint32_t SRAMSize : 5;	//This is minimum save size, 0 is zero, otheriwse size is (1<<(SRAMSize-1)). Does not include skipped bytes for odd/even SRAM.
	uint32_t Hacks : GH_TOTAL_HACKS;
	uint32_t Multitap : 2;	//GL_Multitap
	uint32_t DefaultControls : 5;	//GL_DefControls
	//~ uint32_t Unselectable : 1;
	uint32_t  : 0;
	char SaveExtension[3];
#ifdef GAME_LIST_PARSER
	uint32_t strings_ofs;
#else
	const char *strings;	//Order: Title, ROM filename, Savelong, Saveshort
#endif
} GameListEntry;

#define GL_CalcSRAMSize(sizeval) ((sizeval) ? 1<<((sizeval)-1) : 0)

typedef struct {
	char fourcc[4];
	uint32_t version;
	uint32_t game_cnt;
	GGL_Index index[GGL_NUM_INDICIES];
	char dir[GGL_MAX_DIR_LENGTH];
	GameListEntry games[];
} GameListHeader;

#define GameListHeaderFourCC	"G4AL"

#ifndef GAME_LIST_PARSER
extern GameListEntry *GLCurGame;

extern const char * GetCurGameSaveTitle();
extern const char * GetCurGameSaveVMUTitle();
#endif

#endif
