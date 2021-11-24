#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <assert.h>
#include "ntokenizer.h"

#define GAME_LIST_PARSER
#include "../gamelist.h"

typedef struct {
	GameListEntry gle;
	int namelen;
	int romlen;
	int savenamelen;
	int savetitlelen;
	int savetitlevmulen;
	const char *name;
	const char *rom;
	const char *savename;
	const char *savetitle;
	const char *savetitlevmu;
} ParserEntry;

GGL_Index listindices[GGL_NUM_INDICIES+1];
ParserEntry pes[1000];
int cur_entry;
char GameDir[GGL_MAX_DIR_LENGTH+1] = "/cd/";

#define ARR_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
typedef struct {
	const char *label;
	int value;
} SwitchEntry;

SwitchEntry sound_flags[] = {
	{"22050", SRAT_22050},
	{"8000", SRAT_8000},
	{"11025", SRAT_11025},
	{"16000", SRAT_16000},
	{"32000", SRAT_32000},
	{"44100", SRAT_44100},
};

SwitchEntry emu_flags[] = {
	{"sonic_smps_z80_hack", GH_SONIC_Z80_HACK},
	{"ristar_hack", GH_RISTAR_HACK},
	{"sonic2_auto_render_hack", GH_SONIC2_AUTO_RENDER_HACK},
	{"screen_off_render_hack", GH_SCREEN_OFF_RENDER_HACK},
};
SwitchEntry multitap[] = {
	{"none", DMT_NONE},
	{"teamplayer", DMT_TEAMPLAYER},
	{"4wayplay", DMT_4WAYPLAY},
};
SwitchEntry controls[] = {
	{"3button", DCON_3BUTTON},
	{"6button", DCON_6BUTTON},
	{"lostv6", DCON_6BUTTON_LV},
	{"sf26", DCON_6BUTTON_SF2},
};
SwitchEntry savetype[] = {
	{"none", SAV_NONE},
	{"auto", SAV_AUTO},
	{"odd", SAV_ODD_BYTES_SRAM},
	{"even", SAV_EVEN_BYTES_SRAM},
	{"all", SAV_WORD_SRAM},
	{"eeprom", SAV_EEPROM},
};
SwitchEntry savesize[] = {
	{"0", 0},
	{"none", 0},
	{"512", 10},
	{"1024", 11},
	{"1k", 11},
	{"2048", 12},
	{"2k", 12},
	{"4096", 13},
	{"4k", 13},
	{"8192", 14},
	{"8k", 14},
	{"16384", 15},
	{"16k", 15},
	{"32768", 16},
	{"32k", 16},
	{"65536", 17},
	{"64k", 17},
};

char * slurp(const char *fname, size_t *len) {
	FILE *src = fopen(fname, "rb");
	if (src == NULL) {
		*len = 0;
		return NULL;
	}
	
	//Get length
	fseek(src, 0, SEEK_END);
	size_t flen = ftell(src);
	fseek(src, 0, SEEK_SET);
	
	//Read string
	char *data = malloc(flen+1);
	fread(data, flen, 1, src);
	data[flen] = 0;
	
	fclose(src);
	
	*len = flen;
	return data;
}

void GetString(tokState *t, const char **dststr, int *dstlen) {
	tokNext(t);
	*dststr = tokToken(t);
	tokSkipTo(t, "\n\r");
	*dstlen = tokToken(t) - *dststr;
}

const SwitchEntry * GetTable(tokState *t, const SwitchEntry *list, int listsize) {
	int i;
	for(i = 0; i < listsize; i++) {
		if (tokEquals(t, list[i].label)) {
			return list + i;
		}
	}
	return NULL;
}
int GetTableDef0(tokState *t, const SwitchEntry *list, int listsize) {
	tokNext(t);
	const SwitchEntry * se = GetTable(t, list, listsize);
	if (!se) {
		printf("***Unknown switch: \"%.*s\"***\n", tokPrint(t));
	}
	return se ? se->value : 0;
}

void NextLine(tokState *t) {
	tokNext(t);
	while(!tokAtEnd(t) && !tokNextCharIsOneOf(t, " \n\r"))
		tokNext(t);
}

void ParseList(const char *list, size_t listsize) {
	tokState t;
	tokInit(&t, list, listsize, " \t", " \t\n\r");
	
	ParserEntry *curpe = pes+0;
	
	//Mark list indicies as unused
	int i;
	for(i = 0; i < GGL_NUM_INDICIES+1; i++)
		listindices[i] = -1;
	
	do {
		tokNext(&t);
		
		if (tokToken(&t)[0] == '#') {
			//Comment
			tokSkipTo(&t, "\n\r");
		} else if (tokEqualsChar(&t, '\n')) {
		} else if (tokEqualsChar(&t, '\r')) {
		} else if (tokEquals(&t, "Letter:")) {
			tokNext(&t);
			if (tokTokenLen(&t) == 1) {
				int l = tokToken(&t)[0];
				if (!isalpha(l) && l != '#') {
					printf("Didn't get a letter\n");
					exit(-1);
				}
				
				l = tolower(l);
				if (l >= 'a' && l <= 'z')
					l = l - 'a' + 1;
				else
					l = 0;
				
				if (listindices[l] != -1) {
					printf("Letter: %c is already specified\n", tokToken(&t)[0]);
					exit(-1);
				}
				
				listindices[l] = cur_entry;	//Point to next game
			} else {
				printf("Didn't get a letter\n");
				exit(-1);
			}
		} else if (tokEquals(&t, "Dir:")) {
			tokNext(&t);
			const char *dirnamestart = tokToken(&t);
			tokSkipTo(&t, "\n\r");
			const char *dirnameend = tokToken(&t);
			size_t dirlen = dirnameend - dirnamestart;
			
			if (dirlen > GGL_MAX_DIR_LENGTH) {
				printf("Directory name too long\n");
				exit(1);
			}
			
			memcpy(GameDir, dirnamestart, dirlen);
			GameDir[dirlen] = 0;
			
			printf("Directory: \"%s\"\n", GameDir);
		} else if (tokEquals(&t, "Game:")) {
			//Game name
			
			//New entry
			cur_entry++;
			curpe++;
			
			GetString(&t, &curpe->name, &curpe->namelen);
			
			printf("Game %i: \"%.*s\"\n", cur_entry, curpe->namelen, curpe->name);
		} else if (tokEquals(&t, "Rom:")) {
			//ROM file name
			GetString(&t, &curpe->rom, &curpe->romlen);
			
			//~ printf("ROM file: \"%.*s\"\n", curpe->romlen, curpe->rom);
		} else if (tokEquals(&t, "Savename:")) {
			//Save filename
			GetString(&t, &curpe->savename, &curpe->savenamelen);
			//~ printf("Savename %i: \"%.*s\"\n", cur_entry, curpe->savenamelen, curpe->savename);
		} else if (tokEquals(&t, "Savetitle:")) {
			//Save title
			GetString(&t, &curpe->savetitle, &curpe->savetitlelen);
		} else if (tokEquals(&t, "Savevmu:")) {
			//VMU save title
			GetString(&t, &curpe->savetitlevmu, &curpe->savetitlevmulen);
			//TODO check length
		} else if (tokEquals(&t, "Emu:")) {
			//Emu switches
			tokNext(&t);
			while(!tokEqualsOneOfChar(&t, " \n\r")) {
				const SwitchEntry * se = GetTable(&t, emu_flags, ARR_SIZE(emu_flags));
				if (se) {
					//~ printf("  --%s -> %i\n", se->label, se->value);
					curpe->gle.Hacks |= 1 << se->value;
				} else {
					printf("  --Unknown emu switch: \"%.*s\"\n", tokPrint(&t));
				}
				tokNext(&t);
			}
		} else if (tokEquals(&t, "Sound:")) {
			//Sound
			printf("Per-game sample rates not supported yet\n");
			exit(-1);
			curpe->gle.SampleRate = GetTableDef0(&t, sound_flags, ARR_SIZE(sound_flags));
		} else if (tokEquals(&t, "Savetype:")) {
			//Save type
			curpe->gle.SRAM = GetTableDef0(&t, savetype, ARR_SIZE(savetype));
		} else if (tokEquals(&t, "Savesize:")) {
			//Save size
			curpe->gle.SRAMSize = GetTableDef0(&t, savesize, ARR_SIZE(savesize));
			//printf("%.*s -> %i bytes\n", tokPrint(&t), curpe->gle.SRAMSize ? 1<<(curpe->gle.SRAMSize-1) : 0);
		} else if (tokEquals(&t, "Multitap:")) {
			//Multitap
			printf("Multitap not supported yet\n");
			exit(-1);
			curpe->gle.Multitap = GetTableDef0(&t, multitap, ARR_SIZE(multitap));
		} else if (tokEquals(&t, "Controls:")) {
			//Controls
			curpe->gle.DefaultControls = GetTableDef0(&t, controls, ARR_SIZE(controls));
		} else if (!tokAtEnd(&t)) {
			printf("****Unknown token: \"%.*s\"****\n", tokPrint(&t));
		}
		
		
	} while(!tokAtEnd(&t));
	
	//Make sure no indicies off the end
	for(i = 0; i < GGL_NUM_INDICIES+1; i++) {
		if (listindices[i] >= cur_entry)
			listindices[i] = cur_entry-1;
	}
}

char *strbuf;
char *strbufnext;

uint32_t AppendStr(const char *src, size_t srclen) {
	uint32_t ofs = strbufnext - strbuf;
	
	if (src)
		memcpy(strbufnext, src, srclen);
	strbufnext[srclen] = 0;
	strbufnext += srclen + 1;
	
	return ofs;
}

void WriteList(FILE *out) {
	if (out == NULL)
		return;
	
	uint32_t entrycnt = cur_entry;
	uint32_t str_area_ofs = entrycnt * sizeof(GameListEntry) + sizeof(GameListHeader);
	size_t written;
	
	GameListHeader head;
	
	memcpy(head.fourcc, "G4AL", 4);
	head.version = 1;
	head.game_cnt = entrycnt;
	memcpy(head.index, listindices, sizeof(GGL_Index) * GGL_NUM_INDICIES);
	strncpy(head.dir, GameDir, GGL_MAX_DIR_LENGTH);
	written = fwrite(&head, sizeof(head), 1, out);
	if (written != 1) {
		printf("Error writing header\n");
		exit(-1);
	}
	strbuf = malloc(1024*1024);
	strbufnext = strbuf;
	
	int i;
	ParserEntry *curpe = pes+1;
	for(i = 1; i <= cur_entry; i++) {
		curpe->gle.strings_ofs = AppendStr(curpe->name, curpe->namelen) + str_area_ofs;
		AppendStr(curpe->rom, curpe->romlen);
		AppendStr(curpe->savetitle, curpe->savetitlelen);
		AppendStr(curpe->savetitlevmu, curpe->savetitlevmulen);
		
		if (curpe->savename) {
			memcpy(curpe->gle.SaveExtension, curpe->savename, 3);
			if (curpe->savenamelen != 3) {
				
				printf("%i %li\n", i, strlen(curpe->savename));
				assert(curpe->savenamelen == 3);
			}
		} else {
			memcpy(curpe->gle.SaveExtension, "000", 3);
		}
		
		written = fwrite(&curpe->gle, sizeof(GameListEntry), 1, out);
		if (written != 1) {
			printf("Error writing entry %i\n", i);
			exit(-1);
		}
		
		curpe++;
	}
	
	/* printf("String region length: %i\n", (int)(strbufnext - strbuf));
	printf("Entry array length: %li\n", entrycnt*sizeof(GameListEntry));
	
	printf("Header size: %li\n", sizeof(GameListHeader));
	printf("Game entry: %li\n", sizeof(GameListEntry)); */
	
	written = fwrite(strbuf, (size_t)(strbufnext - strbuf), 1, out);
	if (written != 1) {
		printf("Error writing strings\n");
		exit(-1);
	}
		
}

void PrintUsage() {
	printf(
		"Gens4All Game List Creator\n" \
		"Usage:\n" \
		"	glc -g [OUTPUT.TXT] [ROMS]...\n"
		"		Generate game list template from list of ROMs\n" \
		"	glc -c [INPUT.TXT]\n" \
		"		Convert a text game list to binary. Result is save in \"gamelist.bin\".\n");
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		PrintUsage();
		return 1;
	}
	
	setbuf(stdout, 0);
	
	if (strcasecmp(argv[1], "-g") == 0) {
		if (argc < 3) {
			printf("No output file specified\n");
			exit(1);
		}
		
		char * ext = strrchr(argv[2], '.');
		if (!ext || strcasecmp(ext, ".txt")!=0) {
			printf("Output file is not a text file. This doesn't seem right.\nProgram stopped. We don't want to accidently overwrite a ROM, do we?\n");
			exit(1);
		}
		
		//TODO check argv[2] is a text file
		extern int MakeList(const char *outfile, int argc, char *argv[]);
		return MakeList(argv[2], argc-3, argv+3);
	} else if (strcasecmp(argv[1], "-h") == 0) {
		PrintUsage();
		exit(1);
	} else if (strcasecmp(argv[1], "-c") == 0) {
		if (argc != 3) {
			printf("No input file specified\n");
			exit(1);
		}
			
		size_t len = 0;
		char *in = slurp(argv[2], &len);
		
		printf("Read %li bytes\n", len);
		ParseList(in, len);
		
		FILE *out = fopen("gamelist.bin", "wb");
		WriteList(out);
		fclose(out);
		printf("Success\n");
		
		return 0;
	}
	
	
	
}
