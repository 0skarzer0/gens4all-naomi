
#define VDP_REGISTER_CNT	(0x18)
#define VDP_COLOR_CNT		(64)
#define VDP_PALETTE_SIZE	(16)	/* size in number of colors */
#define VDP_PALETTE_CNT		(4)
#define VDP_VRAM_SIZE		(64*1024)

#define TILEWIDTH	8	/* width in pixels */
#define TILEWIDTHF	8.0f	/* width in pixels */
#define TILEHEIGHT	8	/* height in pixels */
#define TILEHEIGHTF	8.0f	/* height in pixels */
#define TILESTRIDE	4	/* width in bytes */
#define TILESIZEOF	TILEHEIGHT*TILESTRIDE	/* size in bytes */

#define TILE_DEF_COMBOS	(1<<16)

#define VDP_TILEMASK	0x07FF
#define VDP_HFLIP	0x0800
#define VDP_VFLIP	0x1000
#define VDP_PALMASKHI	0x6000
#define VDP_PRIO	0x8000


#define VDP_PALSHIFT	13
#define VDP_PALMASKLO	0x3
#define VDP_GET_PAL(a)	(((a) >> VDP_PALSHIFT) & VDP_PALMASKLO)

// VDP PVR

#define TEXWIDTH	512	/* width in virtual pixels (not physical height reported to pvr) */
#define TEXWIDTHF	512.0f	/* width in virtual pixels (not physical height reported to pvr) */
#define TEXHEIGHT	256	/* height in pixels of vram area (not physical height reported to pvr) */
#define TEXHEIGHTF	256.0f	/* height in pixels of vram area (not physical height reported to pvr) */
#define TEXPVRWIDTH	1024	/* width reported to pvr */
#define TEXPVRWIDTHF	1024.0f	/* width reported to pvr */
#define TEXPVRHEIGHT	512	/* height reported to pvr */
#define TEXPVRHEIGHTF	512.0f	/* height reported to pvr */
#define TEXSTRIDE	256	/* width in bytes */

#define TEXTILEROWS	(TEXHEIGHT/TILEHEIGHT)
#define TEXTILECOLS	(TEXWIDTH/TILEWIDTH)





