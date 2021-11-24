#ifndef _GENS4ALL_PROFILER_H_
#define _GENS4ALL_PROFILER_H_

#include "pvrvdp/profile.h"

enum{
	PROF_TOTAL = 0,
	PROF_M68K,
	PROF_Z80,
	PROF_YMH,
	PROF_BLIT,
	PROF_VRENDER,
	PROF_SRENDER,
	PROF_MAINLOOP,
	PROF_PVRWAIT,
	PROF_MAPA,
	PROF_MAPB,
	PROF_SPR,
	PROF_TEX,
	PROF_R6,
	PROF_R7,
};

#define GENS4ALL_PROFILER_MAX 64
extern unsigned long long prof_initial[GENS4ALL_PROFILER_MAX];
extern unsigned long long prof_sum[GENS4ALL_PROFILER_MAX];
extern unsigned long long prof_executed[GENS4ALL_PROFILER_MAX];
extern int prof_started[GENS4ALL_PROFILER_MAX];
extern char *prof_msg[GENS4ALL_PROFILER_MAX];

extern unsigned long long prof_frame_sum[GENS4ALL_PROFILER_MAX];
extern unsigned long long prof_last_frame_time[GENS4ALL_PROFILER_MAX];

extern float frame_busy_avrg;
extern float last_frame_busy;
extern unsigned long long render_total_us;

#ifndef PROFILER_GENS4ALL

#define prof_start(A)
#define prof_end(A)
#define prof_show()
#define prof_init()
#define prof_reset()

#define prof_frame_start()
#define prof_frame_end()

#define prof_inc_frame_count()

#else

#ifdef PROFILER_SDL
#include <SDL.h>
#define PROFILER__REDUCTION__ 0
#define PROFILER__ADJUST__ 0
#define get_time_in_micros() (unsigned long long)SDL_GetTicks()
#else
#ifndef DREAMCAST
#include <sys/time.h>

//#ifdef WIN32
#if 0
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <time.h>

#ifndef __GNUC__
#define EPOCHFILETIME (116444736000000000i64)
#else
#define EPOCHFILETIME (116444736000000000LL)
#endif

static __inline__ int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    FILETIME        ft;
    LARGE_INTEGER   li;
    __int64         t;
    static int      tzflag;

    if (tv)
    {
        GetSystemTimeAsFileTime(&ft);
        li.LowPart  = ft.dwLowDateTime;
        li.HighPart = ft.dwHighDateTime;
        t  = li.QuadPart;       /* In 100-nanosecond intervals */
        t -= EPOCHFILETIME;     /* Offset to the Epoch time */
        t /= 10;                /* In microseconds */
        tv->tv_sec  = (long)(t / 1000000);
        tv->tv_usec = (long)(t % 1000000);
    }

    return 0;
}

#endif

static __inline__ unsigned long long get_time_in_micros(void)
{
	unsigned long long ret;
	struct timeval tm;
	gettimeofday(&tm,NULL);
	ret=tm.tv_sec;
	ret*=1000000LL;
	ret+=tm.tv_usec;
	return ret;
}
#define PROFILER__REDUCTION__ 0
#define PROFILER__ADJUST__ 0
#else
#include <kos.h>
#define PROFILER__REDUCTION__ 2
#define PROFILER__ADJUST__ 1
#define get_time_in_micros() timer_us_gettime64()
#endif
#endif

#define PROFILE_STACK_SIZE	20

typedef struct {
	int start_cycle;	//Cycle count of last prof_start or prof_end call
	int depth;	//Position in stack
	int stack[PROFILE_STACK_SIZE];	//Background tasks
} G4AProfiler;

extern G4AProfiler g4a_profiler;

//extern unsigned long long prof_last_frame_peak[GENS4ALL_PROFILER_MAX];
//extern unsigned long long prof_last_frame_peak_time[GENS4ALL_PROFILER_MAX];

static __inline__ void prof_frame_start() {
	int i;
	for(i = 0; i < GENS4ALL_PROFILER_MAX; i++) {
		prof_frame_sum[i] = 0;
	}
	
	extern long long last_frame_start_time_us;
	last_frame_start_time_us = timer_us_gettime64();
	
	g4a_profiler.depth = 0;
	g4a_profiler.start_cycle = 0;
	pfcStart(0, PE_CYCLE_COUNT);
}

static __inline__ void prof_frame_end() {
	extern long long last_frame_start_time_us;
	extern long long last_frame_duration_us;
	
	last_frame_duration_us = timer_us_gettime64() - last_frame_start_time_us;
	int i;
	float sum = 0;
	for(i = 0; i < GENS4ALL_PROFILER_MAX; i++) {
		const int cycles_per_microsecond = 200;
		prof_last_frame_time[i] = prof_frame_sum[i] / cycles_per_microsecond;
		
		sum += prof_last_frame_time[i] / 1000.0f;
		
		
/*		if (prof_last_frame_peak_time[i]) {
			prof_last_frame_peak_time[i]--;
		} else {
			prof_last_frame_peak[i] -= 50;
		}
		
		if (prof_last_frame_peak[i] < prof_last_frame_time[i]) {
			prof_last_frame_peak[i] = prof_last_frame_time[i];
			prof_last_frame_peak_time[i] = 60;
		}*/
	}
	//~ last_frame_busy = sum/1000.0f;
	//~ last_frame_busy = sum-(prof_last_frame_time[PROF_VRENDER]+prof_last_frame_time[PROF_PVRWAIT])/1000.0f;
	//~ last_frame_busy = sum-(prof_last_frame_time[PROF_PVRWAIT])/1000.0f;
	last_frame_busy = (last_frame_duration_us - prof_last_frame_time[PROF_PVRWAIT])/1000.0f;
	frame_busy_avrg = frame_busy_avrg * 0.985 + last_frame_busy * 0.015;
	render_total_us = prof_last_frame_time[PROF_MAPA]
		+ prof_last_frame_time[PROF_MAPB]
		+ prof_last_frame_time[PROF_SPR]
		+ prof_last_frame_time[PROF_VRENDER];
	
}

#if 0
static __inline__ void prof_start(unsigned a)
{
//	printf("%s restart\n", prof_msg[a]);
//	fflush(stdout);
//	if (prof_started[a])
//		return;
	prof_executed[a]++;
	prof_started[a]=1;
	prof_initial[a]=get_time_in_micros();
	
	
}


static __inline__ void prof_end(unsigned a)
{
	unsigned long long timeus = get_time_in_micros();
	prof_frame_sum[a] += timeus-prof_initial[a];
	if (!prof_started[a]) {
		//printf("%s not running\n", prof_msg[a]);
		//fflush(stdout);
		return;
	}
/*
	extern unsigned prof_total;
	unsigned i;
	for(i=0;i<prof_total;i++)
		prof_initial[i]+=PROFILER__REDUCTION__;
	prof_sum[a]+= timeus-prof_initial[a]+PROFILER__ADJUST__;
	prof_started[a]=0;
*/
}

#else

#define G4A_PROF_ERRORCHECK	0

static __inline__ void prof_start(unsigned a) {
	if (G4A_PROF_ERRORCHECK && g4a_profiler.depth >= GENS4ALL_PROFILER_MAX) {
		printf("Overflow\n");
		fflush(stdout);
		return;
	}
	
	int curprof = g4a_profiler.stack[g4a_profiler.depth];
	prof_frame_sum[curprof] += pfcReadLow(0) - g4a_profiler.start_cycle;	
	g4a_profiler.stack[++g4a_profiler.depth] = a;
	g4a_profiler.start_cycle = pfcReadLow(0);
}


static __inline__ void prof_end(unsigned a) {
	if (G4A_PROF_ERRORCHECK && g4a_profiler.depth <= 0) {
		printf("Underflow\n");
		fflush(stdout);
		return;
	}
	prof_frame_sum[a] += pfcReadLow(0) - g4a_profiler.start_cycle;
	g4a_profiler.depth--;
	g4a_profiler.start_cycle = pfcReadLow(0);
}
#endif



#undef PROFILER__REDUCTION__
#undef PROFILER__ADJUST__

void prof_init(void);
void prof_add(char *msg);
void prof_show(void);
void prof_setmsg(int n, char *msg);
void prof_reset(void);

#define prof_inc_frame_count() do { extern int prof_frames; prof_frames++;  } while(0)


#endif





#endif


