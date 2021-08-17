#include <SDL.h>

#ifndef DEBUG_GENS4ALL_H
#define DEBUG_GENS4ALL_H

#ifdef DEBUG_GENS4ALL

#ifndef START_DEBUG
#define START_DEBUG 0
#endif

extern int debuggingnow;
#ifdef DEBUG_FILE
extern FILE *DEBUG_STR_FILE;
#else
#define DEBUG_STR_FILE stdout
#endif

#ifdef DEBUG_GENS4ALL_FFLUSH
#define GENS4ALL_FFLUSH fflush(DEBUG_STR_FILE);
#else
#define GENS4ALL_FFLUSH
#endif

#ifdef DEBUG_FILE
#define dbg(TEXTO) \
	if (debuggingnow) \
	{ \
		fprintf(DEBUG_STR_FILE,"%s\n",TEXTO); \
		GENS4ALL_FFLUSH \
	}


#define dbgf(FORMATO, RESTO...) \
	if (debuggingnow) \
	{ \
		fprintf (DEBUG_STR_FILE,FORMATO , ## RESTO); \
		GENS4ALL_FFLUSH \
	}
#else
#define dbg(TEXTO) \
	if (debuggingnow) \
	{ \
		puts(TEXTO); \
		GENS4ALL_FFLUSH \
	}


#define dbgf(FORMATO, RESTO...) \
	if (debuggingnow) \
	{ \
		printf(FORMATO , ## RESTO); \
		GENS4ALL_FFLUSH \
	}
#endif


static __inline__ void dbgsum(char *str, void *buff, unsigned len)
{
	if (debuggingnow)
	{
		unsigned char *p=(unsigned char *)buff;
		unsigned i;
		unsigned long long ret=0;
		for(i=0;i<len;i++,p++)
			ret+=((unsigned long long)(*p))*((unsigned long long)(i+1));
		fprintf(DEBUG_STR_FILE,"%s : %.8X%.8X\n",str,ret);
#ifdef DEBUG_GENS4ALL_FFLUSH
		fflush(DEBUG_STR_FILE);
#endif
	}
}


#else

#define dbg(TEXTO)
#define dbgf(FORMATO, RESTO...)
#define debsum(STR)


#endif

#endif
