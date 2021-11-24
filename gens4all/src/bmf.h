#ifndef BMF_H
#define BMF_H

#include "pvrvdp/pvr_cxt.h"

#define BMF_NO_LIMIT	(-1)
#define BMF_INVALID_GLYPH	(-1)

struct BMFKerningKey {
	int lastcp;
	int curcp;
};

struct BMFKerningPair {
	struct BMFKerningKey key;
	float ofs;
};

struct BMFSprite {
	pvr_context cxt;
	pvr_sprite_txr_t spr;
};

struct BMFCharacter {
	float xadvance;
	float xofs, yofs;
	float w, h;
	int codepoint;
	unsigned int topleftuv, bottomrightuv;
	int page;
};

#define BMFTP_MIP	(0x10)
#define BMFTP_VQ	(0x20)
#define BMFTP_NO_TWID	(0x40)
struct BMFTexturePage {
	uint32 w, h;
	uint32 flags;
	void *pvrptr;
	pvr_context cxt; //polygon
	pvr_context sprcxt; //sprite
};

struct BMFFont {
	float lineheight;
	float base;
	float scalex, scaley;
	int charcnt;
	int pagecnt;
	int kerncnt;
	struct BMFCharacter *chars;
	struct BMFTexturePage *pages;
	struct BMFKerningPair *kerns;
	int flags;
	int kerningenabled;
	char a,r,g,b;
};

//Reads a codepoint from **str and updates *str to point to the next codepoint
typedef int (*BMFStringReader)(const char **str);


struct BMFFont * BMFLoad(const char *fname, const char *extentionreplacement);
void BMFFree(struct BMFFont *font);


//Get the codepoint at str without advancing str to the next codepoint
static inline int PeekCodePoint(BMFStringReader sr, const char *str) {
	return sr(&str);
}

int AsciiReader(const char **str);
int UTF8Reader(const char **str);

//Returns number of code points between start of string and code point searched for
//If not found, returns -1
int BMFFindCodepoint(BMFStringReader sr, const char *str, int searchcodepoint, const char **locfound);
/*
	lengthcodepoints is length of str in codepoints. Used to find the length of a substring
*/
float BMFStringWidth(const struct BMFFont *font, BMFStringReader sr, const char *str, size_t lengthcodepoints);

const struct BMFCharacter * BMFFindCharacter(const struct BMFFont *font, int codepoint);

/*
	strlength is length of str in codepoints. Used to limit ammount to a substring
	
	returns number of sprites gnerated
	
	can write to memory or store queue
*/
int BMFcputsmem(const struct BMFFont *font, float x, float y, float z, int color, BMFStringReader sr, const char *str, size_t strlength, struct BMFSprite *dst);
int BMFputsmem(const struct BMFFont *font, float x, float y, float z, BMFStringReader sr, const char *str, size_t strlength, struct BMFSprite *dst);

void BMFcputs(const struct BMFFont *font, float x, float y, float z, int color, BMFStringReader sr, const char *str);
void BMFputs(const struct BMFFont *font, float x, float y, float z, BMFStringReader sr, const char *str);

//The following all assume ASCII encoding
void BMFprintf(const struct BMFFont *font, float x, float y, float z, const char *fmt, ...);
void BMFprintfmem(const struct BMFFont *font, float x, float y, float z, struct BMFSprite *dst, const char *fmt, ...);
void BMFvprintf(const struct BMFFont *font, float x, float y, float z, const char *fmt, va_list args);
void BMFvprintfmem(const struct BMFFont *font, float x, float y, float z, struct BMFSprite *dst, const char *fmt, va_list args);

void BMFDrawSpriteTexturedTo(float x, float y, float z, float tw, float th,
		uint32_t upperleftuv, uint32_t lowerrightuv, uint32_t light,
		uint32_t specular, pvr_context *cxt, struct BMFSprite *dst);

//Intended for testing. If you don't need kerning, you shouldn't have the host
//utility generate kerning data
void BMFToggleKerning(struct BMFFont *font);
void BMFEnableKerning(struct BMFFont *font);
void BMFDisableKerning(struct BMFFont *font);
int BMFIsKerningEnabled(const struct BMFFont *font);
		
#endif
