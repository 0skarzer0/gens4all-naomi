/*****************************************************************************/
/* SDL based on-screen menu                                                  */
/* Main unit                                                                 */
/* Author: Oscar Orallo Pelaez                                               */
/* Date: 01-03-2008                                                          */
/*****************************************************************************/

#ifndef __MENU_H__
#define __MENU_H__

#include "SDL.h"

#define MAX_ITEM_TEXT_LENGTH 40


typedef enum {ACTION_ITEM, FILESEL_ITEM, NUM_VALUE_ITEM, BOOLEAN_ITEM} MenuEntryType;

typedef struct
{
	char text[MAX_ITEM_TEXT_LENGTH];	/* Displayed text on screen */
	MenuEntryType type;					/* Kind of menu item */
	void *action;						/* Action to be taken */
	char *info;							/* Additional item entry information (flags, etc.) */
	void *handled_var;					/* Variable handled internally by the system menu */
	int enabled;						/* Menu entry enabled */
} MenuEntry;

typedef struct
{
	MenuEntry *entries;
	int num_items;
	int line_spacing;
} MenuType;


int menu_init(SDL_Surface *sfc, MenuType *menu, const char *font_path);

void menu_free(void);


#endif
