#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#define GAME_LIST_PARSER
#include "../gamelist.h"

#define MAX_GAMES	5000
#define TITLE_SIZE	48
#define SERIAL_SIZE	14

static const char *SaveTypes[] = {"auto", "none", "odd", "even", "all", "eeprom"};

typedef struct {
	uint32_t vectors[64];
	char system_type[16];
	char release[16];
	char title_domestic[TITLE_SIZE];
	char title_world[TITLE_SIZE];
	char serial[SERIAL_SIZE];
	uint16_t checksum;
	char peripherals[16];
	uint32_t rom_start;
	uint32_t rom_end;
	uint32_t ram_start;
	uint32_t ram_end;
	unsigned char sram[4];
	uint32_t sram_start;
	uint32_t sram_end;
	char modem[12];
	char pad0[40];
	char region[3];
	char pad1[13];
} GenesisHeader;

static int gamecnt;
static GameListEntry headers[MAX_GAMES];
static char gamenames[MAX_GAMES][TITLE_SIZE+1];
static char filenames[MAX_GAMES][2000];
static int gameorder[MAX_GAMES];

#define GetBE32(src) GetBE32P((unsigned char *)(&(src)))
uint32_t GetBE32P(const unsigned char *src) {
	uint32_t v = src[0] << 24;
	v |= src[1] << 15;
	v |= src[2] << 8;
	v |= src[3];
	
	return v;
}



void AddGame(const char *title, const char *fname, const GameListEntry *entry) {
	//~ printf("%s\n", title);
	strcpy(gamenames[gamecnt], title);
	strcpy(filenames[gamecnt], fname);
	memcpy(headers + gamecnt, entry, sizeof(*headers));
	gamecnt++;
}

void WriteGame(FILE *out, int gameidx) {
	GameListEntry *g = headers + gameidx;
	
	fprintf(out, "#File %s\n", filenames[gameidx]);
	if (g->SRAMSize)
		fprintf(out, "#Has save\n");
	fprintf(out, "Game: %s\n", gamenames[gameidx]);
	//~ fprintf(out, "#Serial: \"%.*s\"\n", SERIAL_SIZE, header[gameidx].serial);
	
	const char *fn = strrchr(filenames[gameidx], '/');
	fprintf(out, "Rom: %s\n", fn ? fn+1 : filenames[gameidx]);
	//~ fprintf(out, "Emu: %s\n", );
	fprintf(out, "Savename: %.3s\n", g->SaveExtension);
	fprintf(out, "Savetitle: %.32s\n", gamenames[gameidx]);
	fprintf(out, "Savevmu: %.16s\n", gamenames[gameidx]);
	fprintf(out, "Savetype: %s\n", SaveTypes[g->SRAM]);
	
	int savesize = g->SRAMSize ? 1<<(g->SRAMSize-1) : 0;
	fprintf(out, "Savesize: %i\n", savesize);
	
	if (g->Hacks) {
		fprintf(out, "Emu: ");
		if (g->Hacks & (1<<GH_SONIC_Z80_HACK))
			fprintf(out, "sonic_smps_z80_hack ");
		if (g->Hacks & (1<<GH_RISTAR_HACK))
			fprintf(out, "ristar_hack ");
		if (g->Hacks & (1<<GH_SONIC2_AUTO_RENDER_HACK))
			fprintf(out, "sonic2_auto_render_hack ");
	}
	
	fprintf(out, "\n\n");
}

const char *GetSortTitle(int gameidx) {
	if (memcmp(gamenames[gameidx], "The ", 4) == 0)
		return gamenames[gameidx] + 4;
	else
		return gamenames[gameidx];
}

int gamecmp (const void *a, const void *b) {
	const char *at = GetSortTitle(*(int*)a);
	const char *bt = GetSortTitle(*(int*)b);
	return strcmp(at, bt);
}
void WriteAllGames(FILE *out) {
	int i;
	for(i = 0; i < gamecnt; i++)
		gameorder[i] = i;
	
	qsort(gameorder, gamecnt, sizeof(*gameorder), &gamecmp);
	
	fprintf(out, "#ROMs loaded file this directory:\n");
	fprintf(out, "Dir: /cd/\n\n");
	
	if (!isalpha(*GetSortTitle(gameorder[0]))) {
		fprintf(out, "Letter: #\n");
	}
	int lastchar = 0;
	for(i = 0; i < gamecnt; i++) {
		char fc = *GetSortTitle(gameorder[i]);
		if (fc != lastchar && isalpha(fc)) {
			fprintf(out, "\n##############################\nLetter: %c\n", fc);
			lastchar = fc;
		}
		WriteGame(out, gameorder[i]);
	}
}

int MakeList(const char *outfile, int argc, char *argv[]) {
	int i;
	char title_buf[TITLE_SIZE+1];
	
	//~ if (argc <= 1) {
		//~ printf("Got nothing\n");
		//~ return 1;
	//~ }
	
	FILE *out = fopen(outfile, "wb");
	if (out == NULL) {
		printf("Couldn't open file for write\n");
		exit(1);
	}
	
	for(i = 0; i < argc; i++) {
		printf("Reading file %i: %s\n", i, argv[i]);
		
		GenesisHeader head;
		int j;
		
		int savesize = 0;
		size_t filesize = 0;
		GameListEntry entry;
		memset(&entry, 0, sizeof(entry));
		
		FILE *f = fopen(argv[i], "rb");
		if (f == NULL) {
			printf("Couldn't open %s\n", argv[i]);
			continue;
		}
		
		if (fread(&head, sizeof(GenesisHeader), 1, f) != 1) {
			printf("Error reading %s\n", argv[i]);
			fclose(f);
			continue;
		}
		
		fseek(f, 0, SEEK_END);
		filesize = ftell(f);
		//printf("File size: %i bytes\n", (int)filesize);
		
		fclose(f);
		
		//Create default entry;
		entry.SampleRate = SRAT_22050;
		entry.SRAM = SAV_AUTO;
		entry.SRAMSize = 0;
		entry.Hacks = 0;
		entry.Multitap = 0;
		entry.DefaultControls = 0;
		entry.SaveExtension[0] = '0';
		entry.SaveExtension[1] = '0';
		entry.SaveExtension[2] = '0';
		
		memset(title_buf, 0, sizeof(title_buf));
		
		//Use world title unless it's blank, then use domestic
		for(j = 0; j < TITLE_SIZE; j++) {
			if (head.title_world[j] != ' ' && head.title_world[j] != 0)
				break;
		}
		memcpy(title_buf, j != TITLE_SIZE ? head.title_world : head.title_domestic, TITLE_SIZE);
		
		//     Remove Initial And      Duplicate Spaces And Convert To Initial Caps
		char *dst = title_buf;
		int had_space = 1;
		int next_caps = 1;
		for(j = 0; j < TITLE_SIZE-2; j++) {
			if (had_space) {
				if (title_buf[j] != ' ')
					*dst++ = toupper(title_buf[j]);
			} else {
				*dst++ = next_caps ? toupper(title_buf[j]) : tolower(title_buf[j]);
			}
			had_space = title_buf[j] == ' ';
			next_caps = had_space || ispunct(title_buf[j]);
			if (title_buf[j] == '\'')
				next_caps = 0;
		}
		
		//Make sure title is null terminated
		*dst = 0;
		
		//Remove trailing space if there is one
		if (dst != title_buf && dst[-1] == ' ')
			dst[-1] = 0;
		
		//Lock-On Technology
		if (filesize == 3.25*1024*1024 && strcmp(title_buf, "Sonic & Knuckles") == 0) {
			strcpy(title_buf, "Knuckles in Sonic the Hedgehog 2");
		} else if (filesize == 4*1024*1024 && strcmp(title_buf, "Sonic & Knuckles") == 0) {
			strcpy(title_buf, "Sonic 3 & Knuckles");
		}
		
		//Check for SRAM
		if (head.sram[0] == 'R' && head.sram[1] == 'A') {
		HaveSRAM:
			//Have SRAM
			switch(head.sram[2]) {
			case 0xE0:
				entry.SRAM = SAV_WORD_SRAM;
				break;
			case 0xF0:
				entry.SRAM = SAV_EVEN_BYTES_SRAM;
				break;
			case 0xF8:
				entry.SRAM = SAV_ODD_BYTES_SRAM;
				break;
			case 0xE8:
				printf("EEPROM not supported yet, no saves available\n");
				goto EndSRAM;
			default:
				printf("Unknown save type (0x%02X), no saves available\n", (unsigned char)head.sram[1]);
				goto EndSRAM;
			}
			
			//Get size
			savesize = GetBE32(head.sram_end) - GetBE32(head.sram_start);
			if (entry.SRAM != SAV_WORD_SRAM)
				savesize = (savesize/2) + 1;
			//printf("%i byte save\n", savesize);
		}
		//Some games have SRAM but don't list it in the header
		if (strcmp(title_buf, "Sonic 3 & Knuckles") == 0) {
			entry.SRAM = SAV_ODD_BYTES_SRAM;
			savesize = 512;
		} else if (strcmp(title_buf, "Buck Rodgers Countdown To Doomsd") == 0) {
			entry.SRAM = SAV_ODD_BYTES_SRAM;
			savesize = 8192;
		}
		
	EndSRAM:;
		//Set save size
		entry.SRAMSize = __builtin_ffs(savesize);
		
		//Hash the ROM serial number for the end of the filename
		unsigned int hash = 5381;
		for(j = 2; j < SERIAL_SIZE; j++) {
			hash = (hash <<5) + hash + head.serial[j];
		}
		hash = (hash <<5) + hash + 0;
		hash ^= hash>>16;
		
		//Set default save name
		for(j = 0; j < 3; j++) { 
			char letter = hash % 52;
			hash /= 52;
			
			entry.SaveExtension[j] = (letter > 26) ? ('a' + letter - 26) : (letter + 'A');
		}
		
		//Check for hacks
		if (strcmp(title_buf, "Sonic The Hedgehog 2") == 0) {
			entry.Hacks = (1 << GH_SONIC2_AUTO_RENDER_HACK) | (1 << GH_SONIC_Z80_HACK);
		} else if (strcmp(title_buf, "Knuckles in Sonic the Hedgehog 2") == 0) {
			entry.Hacks = (1 << GH_SONIC2_AUTO_RENDER_HACK) | (1 << GH_SONIC_Z80_HACK);
		} else if (strcmp(title_buf, "Sonic The Hedgehog 3") == 0) {
			entry.Hacks = 1 << GH_SONIC_Z80_HACK;
		} else if (strcmp(title_buf, "Sonic & Knuckles") == 0) {
			entry.Hacks = 1 << GH_SONIC_Z80_HACK;
		} else if (strcmp(title_buf, "- Ristar -") == 0) {
			entry.Hacks = 1 << GH_RISTAR_HACK;
		} else if (strcmp(title_buf, "- Ristar The Shooting Star -") == 0) {
			entry.Hacks = 1 << GH_RISTAR_HACK;
		} else if (strcmp(title_buf, "Sonic 3d Flickies' Island") == 0) {
			entry.Hacks = 1 << GH_SONIC_Z80_HACK;
		} else if (strcmp(title_buf, "Land Stalker") == 0) {
			entry.Hacks = 1 << GH_SCREEN_OFF_RENDER_HACK;
		} else if (strcmp(title_buf, "Landstalker") == 0) {
			entry.Hacks = 1 << GH_SCREEN_OFF_RENDER_HACK;
		}
		
		AddGame(title_buf, argv[i], &entry);
		
		
		//~ printf("Game: %.48s\n", head.title_domestic);
		//~ printf("Game: %.48s\n", head.title_world);
		//~ WriteGame(out, i-1);
		printf("Title: \"%s\"\n", title_buf);
		//~ printf("Save type: %i\n", entry.SRAM);
	}
	
	WriteAllGames(out);
	
	fclose(out);
	
	return 0;
}

