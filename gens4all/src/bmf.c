#include <kos.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <png/png.h>
//~ #include "debug.h"
//~ #include "common_c.h"
#include "pvrvdp/pvr_cxt.h"
#include "pvrvdp/sq.h"
#include "pvrvdp/sh4float.h"
#include "bmf.h"


typedef struct {
	char	id[4];	// 'DTEX'
	short	width;
	short	height;
	int		type;
	int		size;
} vq_header_t;

static int bmfReadValidMagic(FILE *f) {
	char magic[4];
	fread(magic, 4, 1,f);
	return memcmp(magic, "BMF\003", sizeof(magic)) == 0;
}


//Fire format defines
#define BMF_CHAR_ELEMENT_SIZE		(20)
#define BMF_KERNING_ELEMENT_SIZE	(10)

#define BMF_COMMON_SIZE			(15)

enum {
	BMF_INFO = 1,
	BMF_COMMON,
	BMF_PAGES,
	BMF_CHARS,
	BMF_KERNING
};

static unsigned int Get1UByte(FILE *f) {
	return fgetc(f) & 0xff;
}

static unsigned int Get4UByte(FILE *f) {
	unsigned int val = 0;
	assert(fread(&val, sizeof(val), 1, f) == 1);
	return val;
}
static unsigned int Get2UByte(FILE *f) {
	unsigned short val = 0;
	assert(fread(&val, sizeof(val), 1, f) == 1);
	return val;
}
static int Get2SByte(FILE *f) {
	short val = 0;
	assert(fread(&val, sizeof(val), 1, f) == 1);
	return val;
}


static int CompareChars(const void* key, const void* elem) {
	int cp = (int)key;
	const struct BMFCharacter * c = (const struct BMFCharacter*)elem;
	
	return cp - c->codepoint;
}

static int CompareKerns(const void* key, const void* elem) {
	const struct BMFKerningKey * k = (const struct BMFKerningKey*)key;
	const struct BMFKerningKey * e = (const struct BMFKerningKey*)elem;
	
	return k->lastcp==e->lastcp ? k->curcp-e->curcp : k->lastcp-e->lastcp;
}

const struct BMFCharacter * BMFFindCharacter(const struct BMFFont *font, int codepoint) {
	assert(font);
	return bsearch((void*)codepoint, font->chars, font->charcnt,
		sizeof(*font->chars), CompareChars);
}

//static int enablekerning = 0;
void BMFToggleKerning(struct BMFFont *font) {
	if (font->kerns)
		font->kerningenabled = !font->kerningenabled;
}
void BMFEnableKerning(struct BMFFont *font) {
	if (font->kerns)
		font->kerningenabled = 1;
}
void BMFDisableKerning(struct BMFFont *font) {
	font->kerningenabled = 0;
}
int BMFIsKerningEnabled(const struct BMFFont *font) {
	return font->kerningenabled;
}

static int GetKerningOffset(const struct BMFFont *font, int lastcp, int curcp) {
	if (!BMFIsKerningEnabled(font))
		return 0;
	
	struct BMFKerningKey k = {lastcp, curcp};
	const struct BMFKerningPair *p;
	
	if (1) {
		p = bsearch(&k, font->kerns, font->kerncnt, sizeof(*font->kerns), CompareKerns);
	} else {
		int i;
		p = NULL;
		for(i = 0; i < font->kerncnt; i++) {
			if (memcmp(&k, &font->kerns[i].key, sizeof(k)) == 0) {
				p = font->kerns + i;
				break;
			}
		}
	}
	
	return p ? p->ofs : 0;
}

//This function converts a floating point number to PVR half float format
//while checking for rounding errors
static int halffloaterror;
static unsigned int HalfFloat(float f, const char *description, int num) {
	union {
		unsigned int i;
		float f;
	} fi;
	fi.f = f;
	if (fi.i & 0xffff) {
		unsigned int before = fi.i;
		fi.i &= ~0xffff;
		printf("Rounding %f (%08x) to %f (%08x) (location %s %i)\n", f, before, fi.f, fi.i, description, num);
		halffloaterror = 1;
	}
	return fi.i >> 16;
}

static void ChangeExtension(const char *src, const char *ext, char *dst) {
	const char *endofname = strrchr(src, '.')+1;
	ptrdiff_t srclen = endofname - src;
	memcpy(dst, src, srclen);
	strcpy(dst + srclen, ext);
}

static int ExtentionEquals(const char *fname, const char *ext) {
	char *fnameextstart = strrchr(fname, '.')+1;
	return strcasecmp(fnameextstart, ext) == 0;
}


struct BMFFont * BMFLoad(const char *fname,const char *extentionreplacement) {
	struct BMFFont *dst = malloc(sizeof(struct BMFFont));
	assert(dst != NULL);
	memset(dst, 0, sizeof(*dst));
	
	//Open file
	FILE *f = fopen(fname, "rb");
	if (f == NULL)
		perror("Error opening .fnt");
	assert(f != NULL);
	
	//Check magic cookie
	int fontisvalid = bmfReadValidMagic(f);
	assert(fontisvalid);
	
	char pagenamebuf[200];
	halffloaterror = 0;
	
	while(!feof(f)) {
		int blocktype = fgetc(f);
		if (feof(f) || blocktype > BMF_KERNING)
			break;
		unsigned int size = Get4UByte(f);
		
		int i = 0, elements = 0, namelen = 0;
		
		switch(blocktype) {
		case BMF_INFO:
			//The info block just contains the export settings
			//and doesn't contain anything needed for rendering,
			//so skip the entire block.
			fseek(f, size, SEEK_CUR);
			break;
		case BMF_COMMON:
			dst->lineheight = Get2UByte(f);
			dst->base = Get2UByte(f);
			dst->scalex = Get2UByte(f);
			dst->scaley = Get2UByte(f);
			dst->pagecnt = Get2UByte(f);
			dst->flags = Get1UByte(f);
			dst->a = Get1UByte(f);
			dst->r = Get1UByte(f);
			dst->g = Get1UByte(f);
			dst->b = Get1UByte(f);
			break;
		case BMF_PAGES:
			dst->pages=malloc(sizeof(dst->pages[0])*dst->pagecnt);
			assert(dst->pages != NULL);
			
			namelen = size / dst->pagecnt;
			//Make sure texture filenames fit in our buffer
			assert(namelen < (int)sizeof(pagenamebuf));
			
			for(i = 0; i < dst->pagecnt; i++) {
				struct BMFTexturePage *p = dst->pages + i;
				
				fread(pagenamebuf, namelen, 1, f);
				if (extentionreplacement)
					ChangeExtension(pagenamebuf,
						extentionreplacement,
						pagenamebuf);
				
				//Load texture
				if (ExtentionEquals(pagenamebuf, "png")) {
					#if 0
					int r = png_load_texture(pagenamebuf,
						&p->pvrptr, PNG_FULL_ALPHA,
						&p->w, &p->h);
					if (r)
					#endif
						assert(0);	//Error reading texture
					p->flags = PC_ARGB4444;
				} else if (ExtentionEquals(pagenamebuf, "vq")) {
					vq_header_t head;
					int fileidx = open(pagenamebuf, O_RDONLY);
		
					//Error opening file
					assert(fileidx != -1);
					
					size_t bytesread = read(fileidx, &head, sizeof(head));
					
					assert(bytesread == sizeof(head));
					assert(memcmp(head.id, "DTEX", 4) == 0);
					
					void *readbuff = memalign(32, head.size);
					//~ npvr_texture_id_t texid = npvr_new_texture_file_mem(readbuff, extension, format, flags);
					
					p->pvrptr = pvr_mem_malloc(head.size);
					p->w = head.width;
					p->h = head.height;
					p->flags = ((head.type >> 27) & 0x7) | //format
						(((head.type >> 30) & 1) ? BMFTP_VQ : 0) | //vq
						(((head.type >> 31) & 1) ? BMFTP_MIP : 0) | //mip
						(((head.type >> 26) & 1) ? BMFTP_NO_TWID : 0); //twid
					
					bytesread = read(fileidx, readbuff, head.size);
					assert(bytesread == head.size);
					
					pvr_txr_load(readbuff, p->pvrptr, head.size);
					
					free(readbuff);
					
					close(fileidx);
				} else {
					//Unknown texture format
					assert(0);
				}
				printf("Texmode: %i\n", p->flags & 0x7);
				//Generate the rendering context
				pc_set_default_polygon(&p->cxt);
				pc_set_list(&p->cxt, PC_BLEND_POLY);
				pc_set_texture(&p->cxt, p->pvrptr,
					pc_convert_size(p->w),
					pc_convert_size(p->h), p->flags & 0x7,
					!(p->flags & BMFTP_NO_TWID),
					p->flags & BMFTP_MIP,
					p->flags & BMFTP_VQ);
				pc_set_color_format(&p->cxt, PC_PACKED);
				pc_set_blend_modes(&p->cxt,
					PC_SRC_ALPHA, PC_INV_SRC_ALPHA);
				
				pc_copy(&p->cxt, &p->sprcxt);
				pc_set_command(&p->sprcxt, PC_CMD_SPRITE);
				pc_set_max_strip_length(&p->sprcxt, PC_STRIP2);
			}
			break;
		case BMF_CHARS:
			//Make sure we have loaded the texture pages since we
			//read their sizes here
			assert(dst->pages != NULL);
			
			elements = size / BMF_CHAR_ELEMENT_SIZE;
			dst->charcnt = elements;
			dst->chars = malloc(sizeof(dst->chars[0]) * elements);
			printf("chars size: %i\n", sizeof(dst->chars[0]) * elements);
			assert(dst->chars != NULL);
			
			for(i = 0; i < elements; i++) {
				struct BMFCharacter *c = dst->chars + i;
				c->codepoint = Get4UByte(f);
				float x = Get2UByte(f);	//x
				float y = Get2UByte(f);	//y
				float w = Get2UByte(f);	//w
				float h = Get2UByte(f);	//h
				
				c->xofs = Get2SByte(f);	//xofs
				c->yofs = Get2SByte(f);	//yofs
				c->xadvance = Get2SByte(f);	//xAdvance
				c->page = Get1UByte(f);	//page
				Get1UByte(f);	//chnl (Not usable on DC)
				
				struct BMFTexturePage *t = dst->pages+c->page;
				float topleftu = x / t->w;
				float topleftv = y / t->h;
				float bottomrightu = topleftu + w / t->w;
				float bottomrightv = topleftv + h / t->h;
				c->w = w;
				c->h = h;
				c->topleftuv = (HalfFloat(topleftu, "leftu", c->codepoint)  << 16)
					| HalfFloat(topleftv, "topv", c->codepoint);
				c->bottomrightuv =
					(HalfFloat(bottomrightu, "rightu", c->codepoint) << 16)
					| HalfFloat(bottomrightv, "bottomv", c->codepoint);
			}
			break;
		case BMF_KERNING:
			elements = size / BMF_KERNING_ELEMENT_SIZE;
			
			dst->kerncnt = elements;
			dst->kerns = malloc(sizeof(*dst->kerns) * elements);
			assert(dst->kerns != NULL);
			
			BMFEnableKerning(dst);
			
			//Load each kerning pair
			for(i = 0; i < elements; i++) {
				struct BMFKerningPair *k = dst->kerns + i;
				k->key.lastcp = Get4UByte(f);
				k->key.curcp = Get4UByte(f);
				k->ofs = Get2SByte(f);
			}
			
			//Sort kerns so that we can binary search them when
			//drawing
			qsort(dst->kerns, dst->kerncnt, sizeof(*dst->kerns),
				CompareKerns);
			break;
		default:
			assert(0);
		};
	}
	
	//Check that there was no halffloat rounding
	assert(!halffloaterror);
	
	fclose(f);
	return dst;
}

void BMFFree(struct BMFFont *font) {
	assert(font != NULL);
	
	if (font->chars != NULL)
		free(font->chars);
	if (font->kerns != NULL)
		free(font->kerns);
	if (font->pages != NULL) {
		int i;
		for(i = 0; i < font->pagecnt; i++)
			pvr_mem_free(font->pages[i].pvrptr);
		free(font->pages);
	}
	free(font);
}

int AsciiReader(const char **str) {
	int cp = (unsigned char)**str;
	*str += 1;
	return cp;
}

int UTF8Reader(const char **str) {
	const char *bytes = *str;
	int first = bytes[0];
	if (first >= 0) {
		//1 byte
		*str += 1;
		return first;
	} else if ((first & 0xe0) == 0xc0) {
		//2 bytes
		int b0 = first & 0x1f;
		int b1 = bytes[1] & 0x3f;
		*str += 2;
		return (b0<<6) | b1;
	} else if ((first & 0xf0) == 0xe0) {
		//3 bytes
		int b0 = first & 0xf;
		int b1 = bytes[1] & 0x3f;
		int b2 = bytes[2] & 0x3f;
		*str += 3;
		return (b0<<12) | (b1<<6) | b2;
	} else if ((first & 0xf8) == 0xf0) {
		//4 bytes
		int b0 = first & 0x7;
		int b1 = bytes[1] & 0x3f;
		int b2 = bytes[2] & 0x3f;
		int b3 = bytes[3] & 0x3f;
		*str += 4;
		return (b0<<16) | (b1<<12) | (b2<<6) | b3;
	}
	return -1;
}


inline void BMFDrawSpriteTexturedTo(float x, float y, float z, float tw, float th,
		uint32_t upperleftuv, uint32_t lowerrightuv, uint32_t light,
		uint32_t specular, pvr_context *cxt, struct BMFSprite *dst) {
	
	//char *dstc = dst;
	struct BMFSprite *cs = dst;
	pc_copy(cxt, &cs->cxt);
	pc_set_sprite_color_packed(&cs->cxt, light);
	pc_set_sprite_offset_packed(&cs->cxt, specular);
	_sqSubmit32Offset(cs, 0);
	
	cs->spr.flags = PVR_CMD_VERTEX_EOL;
	cs->spr.ax = x;		cs->spr.ay = y; 	cs->spr.az = z;
	cs->spr.bx = x + tw; 	cs->spr.by = y; 	cs->spr.bz = z;
	cs->spr.cx = x + tw;	
	_sqSubmit32Offset(cs, 32);
				cs->spr.cy = y + th;	cs->spr.cz = z;
	cs->spr.dx = x; 	cs->spr.dy = y + th;
	cs->spr.auv = upperleftuv;
	cs->spr.cuv = lowerrightuv;
	cs->spr.buv = sh4Xtrct(lowerrightuv >> 16, upperleftuv << 16);
	_sqSubmit32Offset(cs, 64);
}

//Returns number of code points between start of string and code point searched for
//If not found, returns -1
int BMFFindCodepoint(BMFStringReader sr, const char *str, int searchcodepoint, const char **locfound) {
	int cp, curcpidx = 0;
	do {
		//Get code point
		cp = sr(&str);
		
		//Did we find what we are looking for?
		if (cp == searchcodepoint) {
			if (locfound)
				*locfound = str;
			return curcpidx;
		}
		
		//Did we hit end of string?
		if (cp == 0) {
			return -1;
		}
		
		curcpidx++;
	} while(cp);
	
	return -1;
}

/*
	lengthcodepoints is length of str in codepoints. Used to find the length of a substring
*/
float BMFStringWidth(const struct BMFFont *font, BMFStringReader sr, const char *str, size_t lengthcodepoints) {
	assert(font != NULL);
	assert(str != NULL);
	
	float len = 0;
	int lastcp = -1;	//Last codepoint, used to preform kerning
	while(lengthcodepoints--) {
		//Get codepoint
		int cp = sr(&str);
		
		//Exit on zero terminator
		if (cp == 0)
			break;
		
		//Find character
		const struct BMFCharacter *c = BMFFindCharacter(font, cp);
		if (c == NULL) {
			//We couldn't find the character, so look up the
			//invalid character glyph
			c = BMFFindCharacter(font, BMF_INVALID_GLYPH);
			if (c == NULL) {
				//Couldn't find the invalid char glyph, so
				//just skip the entire character.
				lastcp = -1;
				continue;
			}
		}
		
		len += GetKerningOffset(font, lastcp, cp) + c->xadvance;
		lastcp = cp;
	}
	
	return len;
}

/*
	strlength is length of str in codepoints. Used to limit ammount to a substring
	
	returns number of sprites gnerated
*/

int BMFcputsmem(const struct BMFFont *font, float x, float y, float z, int color, BMFStringReader sr, const char *str, size_t strlength, struct BMFSprite *dst) {
	if (strlength == 0)
		return 0;	
	
	assert(font != NULL);
	assert(str != NULL);
	assert(sr != NULL);
	assert(dst != NULL);
	
	int spritecnt = 0;
	float leftedge = x;
	int cp, lastcp = BMF_INVALID_GLYPH;	//Last codepoint, used to preform kerning
	do {
		//Get codepoint
		cp = sr(&str);
		
		//Exit on zero terminator
		if (cp == 0)
			break;
		
		//Handle new line
		if (cp == '\n') {
			x = leftedge;
			y += font->lineheight;
			lastcp = BMF_INVALID_GLYPH;
			continue;
		}
		
		//Find character
		const struct BMFCharacter *c = BMFFindCharacter(font, cp);
		if (c == NULL) {
			//We couldn't find the character, so look up the
			//invalid character glyph
			c = BMFFindCharacter(font, BMF_INVALID_GLYPH);
			if (c == NULL) {
				//Couldn't find the invalid char glyph, so
				//just skip the entire character.
				lastcp = BMF_INVALID_GLYPH;
				continue;
			}
		}
		
		x += GetKerningOffset(font, lastcp, cp);
		
		//Don't bother drawing spaces
		if (cp != ' ') {
			BMFDrawSpriteTexturedTo(x + c->xofs, y + c->yofs, z, c->w,
				c->h, c->topleftuv, c->bottomrightuv, color, 0,
				&font->pages[c->page].sprcxt, dst);
			dst++;
			spritecnt++;
		}
		
		x += c->xadvance;
		lastcp = cp;
	} while(cp && --strlength);
	
	return spritecnt;
}
int BMFputsmem(const struct BMFFont *font, float x, float y, float z, BMFStringReader sr, const char *str, size_t strlength, struct BMFSprite *dst) {
	return BMFcputsmem(font, x, y, z, -1, sr, str, strlength, dst);
}
static inline void TargetTA() {
	sqSetTargetAddress((void*)PVR_TA_INPUT);
}
void BMFputs(const struct BMFFont *font, float x, float y, float z, BMFStringReader sr, const char *str) {
	TargetTA();
	BMFputsmem(font, x, y, z, sr, str, BMF_NO_LIMIT, sqGetWritePointer((struct BMFSprite*)PVR_TA_INPUT));
}
void BMFcputs(const struct BMFFont *font, float x, float y, float z, int color, BMFStringReader sr, const char *str) {
	TargetTA();
	BMFcputsmem(font, x, y, z, color, sr, str, BMF_NO_LIMIT, sqGetWritePointer((struct BMFSprite*)PVR_TA_INPUT));
}

void BMFvprintfmem(const struct BMFFont *font, float x, float y, float z, struct BMFSprite *dst, const char *fmt, va_list args) {
	assert(font != NULL);
	assert(fmt != NULL);
	
	char buff[4000];	//TODO buffer overflow	
	int buffused = vsprintf(buff, fmt, args);
	assert(buffused < (int)sizeof(buff));
	
	//voidBMFputsmem(const struct BMFFont *font, float x, float y, float z, BMFStringReader sr, const char *str, size_t strlength, struct BMFSprite *dst)
	BMFputsmem(font, x, y, z, AsciiReader, buff, BMF_NO_LIMIT, dst);
}

void BMFvprintf(const struct BMFFont *font, float x, float y, float z, const char *fmt, va_list args) {
	TargetTA();
	BMFvprintfmem(font, x, y, z, sqGetWritePointer((struct BMFSprite*)PVR_TA_INPUT), fmt, args);
}

void BMFprintfmem(const struct BMFFont *font, float x, float y, float z, struct BMFSprite *dst, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	BMFvprintfmem(font, x, y, z, dst, fmt, args);
	va_end(args);
}

void BMFprintf(const struct BMFFont *font, float x, float y, float z, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	BMFvprintf(font, x, y, z, fmt, args);
	va_end(args);
}
