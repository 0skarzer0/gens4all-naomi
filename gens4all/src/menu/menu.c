/*****************************************************************************/
/* SDL based on-screen menu                                                  */
/* Main unit                                                                 */
/* Author: Oscar Orallo Pelaez                                               */
/* Date: 01-03-2008                                                          */
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "menu.h"
#include "filemenu.h"
#include "util/SFont.h"

#define STARTING_X    60
#define STARTING_Y    55
#define DEFAULT_LINE_SPACING 3
#define LINE_SPACING  menu_desc->line_spacing

#define MAX_ITEMS_ON_SCREEN  10

#define DOWN    1
#define UP   (-1)


static SDL_Surface *bg;
static SFont_Font *sfont;			// Normal text font
static SFont_Font *sfonti;			// Inactive text font
static SFont_Font *sfontl;			// Selected text font
static SFont_Font *sfonta;			// Actioned text font
static MenuType *menu_desc = NULL;	// Current menu description in use
static MenuType *item_menu = NULL;	// Item description menu
static MenuType *file_menu = NULL;	// File description menu
static SDL_Surface *surface;
static int font_height;
static unsigned selected_option=0;
static int menu_items = 0;				// Number of items on screen being displayed
static int menu_pag = 0;				// Page number being displayed


static void draw_item_bg(void);
static int menu_draw(void);
static void menu_update(void);
/* Clears the menu screen with the default background colour */
static void menu_clear(void);

static void DrawPixel(SDL_Surface *screen, int x, int y, Uint8 R, Uint8 G, Uint8 B)
{
    Uint32 color = SDL_MapRGB(screen->format, R, G, B);

    if ( SDL_MUSTLOCK(screen) ) {
        if ( SDL_LockSurface(screen) < 0 ) {
            return;
        }
    }
    switch (screen->format->BytesPerPixel) {
        case 1: { /* Assuming 8-bpp */
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
            *bufp = color;
        }
        break;

        case 2: { /* Probably 15-bpp or 16-bpp */
            Uint16 *bufp;

            bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
            *bufp = color;
        }
        break;

        case 3: { /* Slow 24-bpp mode, usually not used */
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
            *(bufp+screen->format->Rshift/8) = R;
            *(bufp+screen->format->Gshift/8) = G;
            *(bufp+screen->format->Bshift/8) = B;
        }
        break;

        case 4: { /* Probably 32-bpp */
            Uint32 *bufp;

            bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
            *bufp = color;
        }
        break;
    }
    if ( SDL_MUSTLOCK(screen) ) {
        SDL_UnlockSurface(screen);
    }
}

static Uint32 GetPixel(SDL_Surface *screen, int x, int y)
{
//     Uint32 color = SDL_MapRGB(screen->format, R, G, B);

/*    if ( SDL_MUSTLOCK(screen) ) {
        if ( SDL_LockSurface(screen) < 0 ) {
            return;
        }
    }*/
    switch (screen->format->BytesPerPixel) {
        case 1: { /* Assuming 8-bpp */
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
            return (Uint32)*bufp;
        }
        break;

        case 2: { /* Probably 15-bpp or 16-bpp */
            Uint16 *bufp;

            bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
            return (Uint32)*bufp;
        }
        break;

        case 3: { /* Slow 24-bpp mode, usually not used */
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
/*            *(bufp+screen->format->Rshift/8) = R;
            *(bufp+screen->format->Gshift/8) = G;
            *(bufp+screen->format->Bshift/8) = B;*/
			return 0;
        }
        break;

        case 4: { /* Probably 32-bpp */
            Uint32 *bufp;

            bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
            return *bufp;
        }
        break;
    }
/*    if ( SDL_MUSTLOCK(screen) ) {
        SDL_UnlockSurface(screen);
    }*/
}

static void menu_fade_in(void)
{
	int i, j;
	int iter;
	float factor = 0.2f;


	for(iter=0;iter<20;iter++)
	{
		for(i=0;i<surface->w;i++)
		{
			for(j=0;j<surface->h;j++)
			{
				Uint32 color;
				Uint8 r,g,b;
				Uint8 offset;
	
				color = GetPixel(surface,i,j);
				SDL_GetRGB(color, surface->format, &r, &g, &b);
	
				r = (int)((float)r * factor);
				g = (int)((float)g * factor);
				b = (int)((float)b * factor);
				DrawPixel(surface,i,j,r,g,b);
			}
		}
		factor += 0.04f;
		menu_update();
		SDL_Delay(20);
		menu_clear();
	}
}

static int num_entries(const MenuEntry *menu)
{
	int i=0;

	if (menu == NULL) return 0;

	while(menu[i].text[0] != '\0')
		i++;

	return i;
}

static int num_pages(const MenuType *menu)
{
	return (menu->num_items/MAX_ITEMS_ON_SCREEN) + 1;
}

static int set_page(const int num)
{
	if ((num >= 0) && (num < num_pages(menu_desc)))
	{
		menu_pag = num;
		return 0;
	}
	else
		return -1;
}

static void set_menu(MenuType *menu)
{
	menu_desc = menu;
	menu_pag = 0;
	selected_option = 0;
	menu_draw();
}

static void set_sel_option(const int option, const int updown)
{
	if (menu_desc->entries[option].enabled)
		selected_option = option;
	else
	{
		int tmp_option = option;

		while(!menu_desc->entries[tmp_option].enabled)
		{
			tmp_option+=updown;
			tmp_option %= menu_items;
		}

		selected_option = tmp_option;
	}
}

static void switch_boolean(int *var)
{
	*var = (*var)?0:1;
}

static void inc_num_val(int *var, const int incr)
{
	*var += incr;
}

/* Writes an entry at the specified position */
static int menu_write_item(const char *str, const int pos, const int enabled)
{
	if (pos == selected_option)
	{
		draw_item_bg();
		SFont_Write(surface,sfontl,STARTING_X, STARTING_Y + pos*(font_height + LINE_SPACING),str);
	}
	else
	{
		SFont_Write(surface,(enabled)?sfont:sfonti,STARTING_X, STARTING_Y + pos*(font_height + LINE_SPACING),str);
	}
}

/* Writes an entry at the specified position */
static int exec_menu_item(void)
{
	SFont_Write(surface,sfonta,STARTING_X, STARTING_Y + selected_option*(font_height + LINE_SPACING),menu_desc->entries[selected_option + (menu_pag*menu_items)].text);
	SDL_UpdateRect(surface,0,STARTING_Y + selected_option*(font_height + LINE_SPACING),surface->w,selected_option*(font_height + LINE_SPACING) + font_height);
	SDL_Delay(100);

	if (menu_desc->entries[selected_option].type == FILESEL_ITEM)
	{
		char aux_text[MAX_ITEM_TEXT_LENGTH];
	
		strcpy(aux_text, menu_desc->entries[selected_option].info);
		if (file_menu != NULL) { free(file_menu); }
		file_menu = list_files(aux_text);
		if (file_menu != NULL)
		{
			set_menu(file_menu);
		}
	}
	else if (menu_desc->entries[selected_option + (menu_pag*menu_items)].action != NULL)
	{
		(*(int (*)())menu_desc->entries[selected_option + (menu_pag*menu_items)].action)(&menu_desc->entries[selected_option + (menu_pag*menu_items)]);
		menu_draw();
	}
	else
	{
		SFont_Write(surface,sfontl,STARTING_X, STARTING_Y + selected_option*(font_height + LINE_SPACING),menu_desc->entries[selected_option].text);
		SDL_UpdateRect(surface,0,STARTING_Y + selected_option*(font_height + LINE_SPACING),surface->w,selected_option*(font_height + LINE_SPACING) + font_height);
	}
}

static void draw_item_bg(void)
{
	SDL_Rect rect;
	int i,j;

	rect.x = 0;
	rect.y = STARTING_Y + selected_option*(font_height + LINE_SPACING);
	rect.w = surface->w;
	rect.h = font_height;

	for(i=rect.x;i<(rect.x + rect.w);i++)
	{
		for(j=rect.y;j<(rect.y + rect.h);j++)
		{
			Uint32 color;
			Uint8 r,g,b;
			Uint8 offset;

			color = GetPixel(surface,i,j);
			SDL_GetRGB(color, surface->format, &r, &g, &b);

			r/=2;
			g/=2;
			b/=2;
			DrawPixel(surface,i,j,r,g,b);
		}
	}
}

static SFont_Font* get_coloured_font(const SFont_Font *font, const Uint8 r, const Uint8 g, const Uint8 b)
{
	SFont_Font *tmp;
	Uint32 color;
	int x,y;

	tmp = (SFont_Font *)malloc(sizeof(SFont_Font));
	if (tmp == NULL)
		return NULL;

	memcpy(tmp,font,sizeof(SFont_Font));

	tmp->Surface = SDL_DisplayFormat(font->Surface);
	if (tmp->Surface == NULL)
		return NULL;

	color = SDL_MapRGB(tmp->Surface->format, 255, 255, 255);

	for(x=0;x<tmp->Surface->w;x++)
	{
		for(y=0;y<tmp->Surface->h;y++)
		{
 			if (GetPixel(tmp->Surface, x, y) == color)
			{
  				DrawPixel(tmp->Surface,x,y,r,g,b);
			}

		}
	}

	return tmp;
}


int menu_init(SDL_Surface *sfc, MenuType *menu, const char *font_path)
{
	/* Font loading */
	sfont = SFont_InitFont(SDL_LoadBMP(DATA_PREFIX "/sfonts.bmp"));

	if (sfont == NULL)
		return -1;

	font_height = SFont_TextHeight(sfont);

	/* Load inactive & selected colour font */
 	sfonti = get_coloured_font(sfont, 0x84,0x84,0x84);
 	sfontl = get_coloured_font(sfont, 0xff,0xff,0x00);
 	sfonta = get_coloured_font(sfont, 0xff,0x00,0x00);

	if ((sfonti == NULL) || (sfontl == NULL) || (sfonta == NULL))
		return -1;

	/* Cargamos el archivo BMP en la superficie */
	bg = SDL_LoadBMP(DATA_PREFIX "bg.bmp");

	if (bg == NULL)
		return -1;

	/* Save the number of items */
	menu->num_items = num_entries(menu->entries);
	menu->line_spacing = DEFAULT_LINE_SPACING;

	item_menu = menu;	// Save the item description menu
	menu_desc = menu;	// Set it up to be the current
	surface = sfc;		// Rendering surface

	return 0;
}

/* Clears the menu screen with the default background colour */
static void menu_clear(void)
{
    SDL_BlitSurface(bg, NULL, surface, NULL);
}

/* Updates menu screen */
static void menu_update(void)
{
	SDL_UpdateRect(surface,0,0,0,0);
}

static int menu_draw(void)
{
	int idx = menu_pag * MAX_ITEMS_ON_SCREEN;
	static int first_draw=1;
	int i = 0;

	menu_clear();

#if 0
	if (first_draw)		// Initial effect, showing only the BG
	{
  		menu_fade_in();
		SDL_Delay(1000);
		first_draw = 0;
	}
#endif

	while((menu_desc->entries[idx].text[0] != '\0') && (i<MAX_ITEMS_ON_SCREEN))
	{
		char tmp_text[MAX_ITEM_TEXT_LENGTH + 5];

		switch(menu_desc->entries[idx].type)
		{
			case BOOLEAN_ITEM:
				sprintf(tmp_text, "%s  <%s>", menu_desc->entries[idx].text,(*((int *)menu_desc->entries[idx].handled_var))?"ON":"OFF");
				menu_write_item(tmp_text, i, menu_desc->entries[idx].enabled);
				break;

			case NUM_VALUE_ITEM:
				sprintf(tmp_text, "%s  <%d>", menu_desc->entries[idx].text,(*((int *)menu_desc->entries[idx].handled_var)));
				menu_write_item(tmp_text, i, menu_desc->entries[idx].enabled);
				break;

			default:
				menu_write_item(menu_desc->entries[idx].text, i, menu_desc->entries[idx].enabled);
		}
		i++;
		idx++;
	}

	/* Fetch the number of menu items */
	menu_items = i;
	menu_update();

	return 0;
}


/* Move to the next menu item up/downwards */
static void menu_move_updown(const int updown)
{
	switch(updown)
	{
		case UP:
			if (selected_option == 0)
			{
				set_page(menu_pag - 1);
				set_sel_option(menu_items - 1,updown);
			}
			else
			{
				set_sel_option(selected_option - 1, updown);
			}
			break;
		case DOWN:
			if ((selected_option + 1) == menu_items)
			{
				set_page(menu_pag + 1);
				set_sel_option(0,updown);
			}
			else
			{
				set_sel_option(selected_option + 1, updown);
			}
			break;
	}
}

void menu_run(void)
{
	SDL_Event event;

	menu_draw();

	while(1)
	{
		SDL_WaitEvent(&event);

	    switch (event.type) {
	        case SDL_KEYDOWN:
				switch(event.key.keysym.sym)
				{
					case SDLK_LEFT:
						switch(menu_desc->entries[selected_option].type)
						{
							case BOOLEAN_ITEM:
								switch_boolean((int *)menu_desc->entries[selected_option].handled_var);
								menu_draw();
								break;

							case NUM_VALUE_ITEM:
								inc_num_val((int *)menu_desc->entries[selected_option].handled_var, -1);
								menu_draw();
								break;

							default:
								if (set_page(menu_pag - 1) == 0)
									menu_draw();
								break;
						}
						break;
			        case SDLK_RIGHT:
						switch(menu_desc->entries[selected_option].type)
						{
							case BOOLEAN_ITEM:
								switch_boolean((int *)menu_desc->entries[selected_option].handled_var);
								menu_draw();
								break;

							case NUM_VALUE_ITEM:
								inc_num_val((int *)menu_desc->entries[selected_option].handled_var, 1);
								menu_draw();
								break;

							default:
								if (set_page(menu_pag + 1) == 0)
								{
									menu_draw();
								}
								break;
						}
						break;
					case SDLK_UP:
						menu_move_updown(UP);
					   	menu_draw();
						break;
					case SDLK_DOWN:
						menu_move_updown(DOWN);
						menu_draw();
						break;
					case SDLK_a:
					case SDLK_LCTRL:
					case SDLK_LALT:
					case SDLK_SPACE:
					case SDLK_LSHIFT:
						exec_menu_item();
						break;
					case SDLK_ESCAPE:
					case SDLK_BACKSPACE:
						set_menu(item_menu);
						break;
			        default:
						exit(0);
						break;
				}
				break;
#ifdef _PSP
				case SDL_JOYBUTTONDOWN:
					switch(event.jbutton.button) {
					case 7:	// Left
						switch(menu_desc->entries[selected_option].type)
						{
							case BOOLEAN_ITEM:
								switch_boolean((int *)menu_desc->entries[selected_option].handled_var);
								menu_draw();
								break;

							case NUM_VALUE_ITEM:
								inc_num_val((int *)menu_desc->entries[selected_option].handled_var, -1);
								menu_draw();
								break;

							default:
								if (set_page(menu_pag - 1) == 0)
									menu_draw();
								break;
						}
						break;
			        case 9:	// Right
						switch(menu_desc->entries[selected_option].type)
						{
							case BOOLEAN_ITEM:
								switch_boolean((int *)menu_desc->entries[selected_option].handled_var);
								menu_draw();
								break;

							case NUM_VALUE_ITEM:
								inc_num_val((int *)menu_desc->entries[selected_option].handled_var, 1);
								menu_draw();
								break;

							default:
								if (set_page(menu_pag + 1) == 0)
								{
									menu_draw();
								}
								break;
						}
						break;
					case 8:	// Up
						menu_move_updown(UP);
					   	menu_draw();
						break;
					case 6:	// Down
						menu_move_updown(DOWN);
						menu_draw();
						break;
					case 2:	// Cross
						exec_menu_item();
						break;
					case 1: // Circle
						set_menu(item_menu);
						break;
			        default:
						exit(0);
						break;
				}
#endif
				break;
			case SDL_QUIT:
				exit(0);
	    }
	}
}

void menu_free(void)
{
	SFont_FreeFont(sfont);
	SFont_FreeFont(sfontl);
	SFont_FreeFont(sfonti);
    SDL_FreeSurface(bg);

	if (file_menu != NULL)
	{
		free(file_menu->entries);
		free(file_menu);
	}
}
