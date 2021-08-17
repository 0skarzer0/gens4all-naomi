/*****************************************************************************/
/* SDL based on-screen menu                                                  */
/* File manager unit                                                         */
/* Author: Oscar Orallo Pelaez                                               */
/* Date: 01-03-2008                                                          */
/*****************************************************************************/


#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include "menu.h"

#define MAX_FILES_LIST_SIZE  100
#define FILEMENU_LINE_SPACING 2

// #define DEBUG_FILEMENU

int run(const char *rom_filename);


/* List the directory content */
MenuType * list_files(const char *dir)
{
	MenuType *tmp_menu_desc;
	DIR *dp;
	struct dirent *ep;
	int i=0;

#ifdef DEBUG_FILEMENU
	printf("DIR=%s\n",dir);
#endif

	tmp_menu_desc = (MenuType *) malloc(sizeof(MenuType));
	
	if (tmp_menu_desc == NULL)
		return NULL;

	tmp_menu_desc->entries = (MenuEntry *) malloc(sizeof(MenuEntry) * (MAX_FILES_LIST_SIZE + 1));

	if (tmp_menu_desc->entries == NULL)
		return NULL;

	chdir(dir);

#ifdef DEBUG_FILEMENU
	printf("CUR_DIR=%s\n",get_current_dir_name());
#endif

	dp = opendir (".");
	if (dp != NULL)
	{
		while ((ep = readdir (dp)) && (i < MAX_FILES_LIST_SIZE))
		{
			if (strcmp(ep->d_name,".") == 0) continue;
 			strncpy(tmp_menu_desc->entries[i].text,ep->d_name, MAX_ITEM_TEXT_LENGTH);
			if (opendir (tmp_menu_desc->entries[i].text) != NULL)
			{
				tmp_menu_desc->entries[i].type = FILESEL_ITEM;
				tmp_menu_desc->entries[i].info = tmp_menu_desc->entries[i].text;
#ifdef DEBUG_FILEMENU
	printf("text=%s - info=%s\n",tmp_menu_desc->entries[i].text,tmp_menu_desc->entries[i].info);
#endif
				tmp_menu_desc->entries[i].action = NULL;
			}
			else
			{
				tmp_menu_desc->entries[i].type = ACTION_ITEM;
				tmp_menu_desc->entries[i].info = NULL;
				tmp_menu_desc->entries[i].action = run;
			}

			tmp_menu_desc->entries[i].enabled = 1;
			i++;
		}
		(void) closedir (dp);
	}

#ifdef DEBUG_FILEMENU
	printf("TOTAL_FILES=%d\n",i);
#endif

	strcpy(tmp_menu_desc->entries[i].text, "");
	tmp_menu_desc->entries[i].info = "";
	tmp_menu_desc->entries[i].action = NULL;

	/* Save the number of items */
	tmp_menu_desc->num_items = i;

	tmp_menu_desc->line_spacing = FILEMENU_LINE_SPACING;

	return tmp_menu_desc;
}
