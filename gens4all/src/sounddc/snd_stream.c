/* KallistiOS 1.2.0

   snd_stream.c
   Copyright (c)2000,2001,2002,2003 Dan Potter
   Copyright (c)2002 Florian Schulze
   Modified for Genesis Plus DC by Stéphane Dallongeville

   SH-4 support routines for SPU streaming sound driver
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <arch/timer.h>
#include <dc/g2bus.h>
#include <dc/spu.h>
#include <dc/sound/sound.h>

#include "snd_stream.h"
#include "aica_cmd_iface.h"
//#include <dc/sound/aica_comm.h>

//CVSID("$Id: snd_stream.c,v 1.13 2003/03/09 01:26:37 bardtx Exp $");

/*

This module uses a nice circularly queued data stream in SPU RAM, which is
looped by a program running in the SPU itself.

Basically the poll routine checks to see if a certain minimum amount of
data is available to the SPU to be played, and if not, we ask the user
routine for more sound data and load it up. That's about it.

*/


/* The last write position in the playing buffer */
static int last_write_pos = 0;
static int curbuffer = 0;

/* the address of the sound ram from the SH4 side */
#define SPU_RAM_BASE    0xa0800000

/* buffer size in bytes */
#define BUFFER_SIZE     0xa00

/* Stream data location in AICA RAM */
static uint32			spu_ram_allocated, spu_ram_sch1, spu_ram_sch2;

/* "Get data" callback; we'll call this any time we want to get another
   buffer of output data. */
static void** (*str_get_data)(int requested_cnt, int * received_cnt) = NULL;

/* Stereo/mono flag for stream */
static int stereo;

/* Playback frequency */
static int frequency;

/* Have we been initialized yet? (and reserved a buffer, etc) */
static int initted = 0;

/* Set "get data" callback */
void gpdc_snd_stream_set_callback(void **(*func)(int, int*)) {
	str_get_data = func;
}

/* Clear buffers */
void gpdc_snd_stream_clear() {
	if (!str_get_data) return;

	spu_memset(spu_ram_sch1, 0, BUFFER_SIZE);
	spu_memset(spu_ram_sch2, 0, BUFFER_SIZE);
}

/* Prefill buffers -- do this before calling start() */
void gpdc_snd_stream_prefill() {
	void **buf;
	int got;

	if (!str_get_data) return;

	/* Load first buffer */
	/* XXX Note: This will not work if the full data size is less than
	   BUFFER_SIZE or BUFFER_SIZE/2. */
	buf = str_get_data(BUFFER_SIZE, &got);
	if (got < BUFFER_SIZE) gpdc_snd_stream_clear();
	else
	{
	   spu_memload(spu_ram_sch1, (uint8*)buf[0], BUFFER_SIZE);

        if ((stereo) && (buf[1] != NULL))
            spu_memload(spu_ram_sch2, (uint8*)buf[1], BUFFER_SIZE);
        else
            spu_memload(spu_ram_sch2, (uint8*)buf[0], BUFFER_SIZE);
    }

	/* Start with playing on buffer 0 */
	last_write_pos = 0;
	curbuffer = 0;
}

/* Initialize stream system */
int gpdc_snd_stream_init(void** (*callback)(int, int *)) {
	
	/* Setup the callback */
	gpdc_snd_stream_set_callback(callback);

	if (initted)
		return 0;

	/* Finish loading the stream driver */
	if (snd_init() < 0) {
		//dbglog(DBG_ERROR, "snd_stream_init(): snd_init() failed, giving up\n");
		return -1;
	}

  spu_ram_allocated = snd_mem_malloc((BUFFER_SIZE*2)+32);
	if (spu_ram_allocated & 31)
    spu_ram_sch1 = (spu_ram_allocated & ~31)+32; //(snd_mem_malloc(BUFFER_SIZE*2);
  else
    spu_ram_sch1 = spu_ram_allocated;

	spu_ram_sch2 = spu_ram_sch1 + BUFFER_SIZE;

	initted = 1;

	return 0;
}

/* Shut everything down and free mem */
void gpdc_snd_stream_shutdown() {
	if (!initted)
		return;

	gpdc_snd_stream_stop();
	snd_mem_free(spu_ram_allocated);
	initted = 0;
}

/* Start streaming (or if queueing is enabled, just get ready) */
void gpdc_snd_stream_start(uint32 freq, int st) {
	AICA_CMDSTR_CHANNEL(tmp, cmd, chan);
	
	if (!str_get_data) return;

	stereo = st;
	frequency = freq;

	/* Make sure these are sync'd (and/or delayed) */
	snd_sh4_to_aica_stop();
	
	/* Prefill buffers */
	gpdc_snd_stream_prefill();

	/* Channel 0 */
	cmd->cmd = AICA_CMD_CHAN;
	cmd->timestamp = 0;
	cmd->size = AICA_CMDSTR_CHANNEL_SIZE;
	cmd->cmd_id = 0;
	chan->cmd = AICA_CH_CMD_START;
	chan->base = spu_ram_sch1;
	chan->type = AICA_SM_16BIT;
	chan->length = (BUFFER_SIZE/2);
	chan->loop = 1;
	chan->loopstart = 0;
	chan->loopend = (BUFFER_SIZE/2)-1;
	chan->freq = freq;
	chan->vol = 240;
	chan->pan = 0;
	snd_sh4_to_aica(tmp, cmd->size);

	/* Channel 1 */
	cmd->cmd_id = 1;
	chan->base = spu_ram_sch2;
	chan->pan = 255;
	snd_sh4_to_aica(tmp, cmd->size);

	/* Process the changes */
    snd_sh4_to_aica_start();
}

/* Stop streaming */
void gpdc_snd_stream_stop() {
	AICA_CMDSTR_CHANNEL(tmp, cmd, chan);
	
	if (!str_get_data) return;

	/* Stop stream */
	/* Channel 0 */
	cmd->cmd = AICA_CMD_CHAN;
	cmd->timestamp = 0;
	cmd->size = AICA_CMDSTR_CHANNEL_SIZE;
	cmd->cmd_id = 0;
	chan->cmd = AICA_CH_CMD_STOP;
	snd_sh4_to_aica(tmp, cmd->size);

	/* Channel 1 */
	cmd->cmd_id = 1;
	snd_sh4_to_aica(tmp, AICA_CMDSTR_CHANNEL_SIZE);
}

#define G2_LOCK(OLD) \
	do { \
		if (!irq_inside_int()) \
			OLD = irq_disable(); \
			irq_enable(); \
		/* suspend any G2 DMA here... */ \
		while((*(volatile unsigned int *)0xa05f688c) & 0x20) \
			; \
	} while(0)

#define G2_UNLOCK(OLD) \
	do { \
		/* resume any G2 DMA here... */ \
		if (!irq_inside_int()) \
			irq_restore(OLD); \
	} while(0)
static void spu_memload_stereo16_dual(unsigned short *leftsrc, unsigned short *rightsrc, uint32 *leftdst, uint32 *rightdst, size_t size)
{
	//~ uint16 *src = src0;
	//~ uint32 *left  = (uint32*)(leftpos +SPU_RAM_BASE);
	//~ uint32 *right = (uint32*)(rightpos+SPU_RAM_BASE);
	//~ size = (size+7)/8;
	while(size--) {
		unsigned lval,rval,old;
		lval = *leftsrc++;
		rval = *rightsrc++;
		lval|= (*leftsrc++)<<16;
		rval|= (*rightsrc++)<<16;
		g2_fifo_wait();
		G2_LOCK(old);
		*leftdst++=lval;
		*rightdst++=rval;
		G2_UNLOCK(old);
	//	g2_write_32(*left++,lval);
	//	g2_write_32(*right++,rval);
	//	g2_fifo_wait();
	}
}

/* Poll streamer to load more data if neccessary */
int gpdc_snd_stream_poll()
{
	int     current_play_pos;
	int     needed_samples;
	int     needed_samples1;
	int     needed_samples2;
	int     got_samples;
	void    **data;

	uint16 *bufL;
	uint16 *bufR;
	
	
	int total_needed_samples;

	if (!str_get_data) return -1;

	/* Get current read position */
	current_play_pos = (int)g2_read_32(SPU_RAM_BASE + AICA_CHANNEL(0) + offsetof(aica_channel_t, pos));
	current_play_pos &= ~1;

	if (current_play_pos >= (BUFFER_SIZE/2))
	{
		printf("snd_stream_poll: current_play_pos = %ld (%08lx)\n", current_play_pos, current_play_pos); fflush(stdout);
		return -1;
	}
	
	if (last_write_pos <= current_play_pos)
	{
		needed_samples = needed_samples1 = current_play_pos - last_write_pos;
		total_needed_samples = current_play_pos - last_write_pos;
	}
	else
	{
		needed_samples1 = (BUFFER_SIZE/2) - last_write_pos;
		needed_samples = needed_samples1 + current_play_pos;
		
		
		total_needed_samples = (BUFFER_SIZE/2) - last_write_pos + current_play_pos;
	}
	
	total_needed_samples &= ~7;
	//~ printf("%c [%3i]", (total_needed_samples < 300 || total_needed_samples > 400)?'!':' ', total_needed_samples);
	
	if ( 1 && total_needed_samples > 0) {
		int needed_last_write_samples;
		int needed_start_samples;
		
		data = str_get_data(total_needed_samples, &got_samples);
		
		if (got_samples < total_needed_samples)
			total_needed_samples = got_samples & ~1;	//Might lose a sample here...
		
		bufL = (uint16*)data[0];
		bufR = (uint16*)data[1];
		
		if (last_write_pos <= current_play_pos) {
			needed_last_write_samples = total_needed_samples;
			needed_start_samples = 0;
		} else {
			needed_last_write_samples = (BUFFER_SIZE/2) - last_write_pos;
			needed_start_samples = total_needed_samples - needed_last_write_samples;
		}
		//~ printf("(LW %4i, PP %4i)(%3i,%3i)", last_write_pos, current_play_pos, needed_start_samples,needed_last_write_samples);
		//Round down to even number of samples, KOS rounds up to 4 bytes
		//~ needed_last_write_samples &= ~7;
		//~ needed_start_samples &= ~7;
		
		//~ if (needed_last_write_samples < 0 || needed_start_samples < 0) {
			//~ printf("Sample error\n"); fflush(stdout);
			//~ exit(0);
		//~ }
		
		//~ spu_memload(spu_ram_sch1 + last_write_pos * 2, (uint8*)bufL, needed_last_write_samples * 2);
		//~ spu_memload(spu_ram_sch2 + last_write_pos * 2, (uint8*)bufR, needed_last_write_samples * 2);
		spu_memload_stereo16_dual(bufL, bufR, spu_ram_sch1 + last_write_pos * 2, spu_ram_sch2 + last_write_pos * 2, needed_last_write_samples * 2);
		last_write_pos = last_write_pos + needed_last_write_samples;
		
		if (needed_start_samples) {
			//~ spu_memload(spu_ram_sch1, ((short*)bufL) + needed_last_write_samples, needed_start_samples * 2);
			//~ spu_memload(spu_ram_sch2, ((short*)bufR) + needed_last_write_samples, needed_start_samples * 2);
			spu_memload_stereo16_dual(((short*)bufL) + needed_last_write_samples, ((short*)bufR) + needed_last_write_samples, spu_ram_sch1, spu_ram_sch2, needed_start_samples * 2);
			last_write_pos = needed_start_samples;
		}
		//~ printf("NW: %3i\n", last_write_pos); fflush(stdout);
	}
	
	//Old code
	if (0 && needed_samples > 0)
	{
		data = str_get_data(total_needed_samples, &got_samples);
		
		if (got_samples < needed_samples)
			needed_samples = got_samples & ~1;

		if (data == NULL)
		{
			/* Fill the "other" buffer with zeros */
			spu_memset(spu_ram_sch1 + (last_write_pos * 2), 0, needed_samples * 2);
			spu_memset(spu_ram_sch2 + (last_write_pos * 2), 0, needed_samples * 2);
			return -3;
		}

		bufL = (uint16*)data[0];

		if ((stereo) && (data[1] != NULL))
			bufR = (uint16*)(data[1]);
		else
			bufR = (uint16*)(data[0]);

		if (needed_samples >= needed_samples1)
		{
			#if defined(SPUDMA)
			spu_dma_transfer((void *)bufL,spu_ram_sch1 + (last_write_pos * 2), needed_samples1 * 2, 1, 0, 0);
			spu_dma_transfer((void *)bufR,spu_ram_sch2 + (last_write_pos * 2), needed_samples1 * 2, 1, 0, 0);
			last_write_pos += (needed_samples1 & ~15);
			#else
			spu_memload(spu_ram_sch1 + (last_write_pos * 2), (uint8*)bufL, needed_samples1 * 2);
			spu_memload(spu_ram_sch2 + (last_write_pos * 2), (uint8*)bufR, needed_samples1 * 2);
			last_write_pos += needed_samples1;
			#endif
			last_write_pos &= (BUFFER_SIZE/2) - 1;
			needed_samples -= needed_samples1;
		}

		if (needed_samples > 0)
		{
			#if defined(SPUDMA)
			spu_dma_transfer((void *)bufL, spu_ram_sch1 + (last_write_pos * 2), needed_samples * 2, 1, 0, 0);
			spu_dma_transfer((void *)bufR, spu_ram_sch2 + (last_write_pos * 2), needed_samples * 2, 1, 0, 0);
			last_write_pos += (needed_samples & ~15);
			#else
			spu_memload(spu_ram_sch1 + (last_write_pos * 2), (uint8*)bufL, needed_samples * 2);
			spu_memload(spu_ram_sch2 + (last_write_pos * 2), (uint8*)bufR, needed_samples * 2);
			last_write_pos += needed_samples;
			#endif
			last_write_pos &= (BUFFER_SIZE/2) - 1;
		}
	}
	
	return 0;
}

/* Set the volume on the streaming channels */
void gpdc_snd_stream_volume(int vol) {
	AICA_CMDSTR_CHANNEL(tmp, cmd, chan);

	cmd->cmd = AICA_CMD_CHAN;
	cmd->timestamp = 0;
	cmd->size = AICA_CMDSTR_CHANNEL_SIZE;
	cmd->cmd_id = 0;
	chan->cmd = AICA_CH_CMD_UPDATE | AICA_CH_UPDATE_SET_VOL;
	chan->vol = vol;
	snd_sh4_to_aica(tmp, cmd->size);

	cmd->cmd_id = 1;
	snd_sh4_to_aica(tmp, cmd->size);
}
