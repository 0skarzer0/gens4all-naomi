#include <kos.h>
#include <dc/sound/sound.h>
#include <dc/sound/aica_comm.h>
#include "gens4all.h"

#include <time.h>
#include "psg.h"
#include "ym2612.h"
#include "vdp_io.h"
#include "pcm.h"
#include "sounddc/snd_stream.h"

#define MAX_AUDIOVAL ((1<<(16-1))-1)
#define MIN_AUDIOVAL -(1<<(16-1))

int Seg_L[882], Seg_R[882];
int Seg_Lenght, SBuffer_Lenght;
int Sound_Rate = 22050;
int Sound_Segs = 8;
int Bytes_Per_Unit = 4;
int Sound_Enable = 1;
int Sound_Stereo = 1;
int Sound_Is_Playing = 0;
int Sound_Initialised = 0;
int WAV_Dumping = 0;
int GYM_Playing = 0;
unsigned int Sound_Interpol[882];
unsigned int Sound_Extrapol[312][2];

char Dump_Dir[1024] = "";
char Dump_GYM_Dir[1024] = "";

#ifdef GYM_SUPPORT
int WP, RP;
#endif

//For SDL, this is bytes, for non-SDL, this is samples
static int audio_len = 0;

unsigned char *audiobuf = 0;
extern int over_time;

#define soundrecbytes	(128*1024)
char soundrec[soundrecbytes];
int soundrecpos;
int soundrecenabled = 0;

#ifndef NO_SOUND
void
proc (void *user, Uint8 * buffer, int len)
{
	prof_start(PROF_YMH);
	user=audiobuf;
	if(!user) {
		memset(buffer,len,0);
		return;
	}
	if (audio_len < len) {
		//printf("U"); fflush(stdout);
		//Audio buffer underflow. Copy what we have, pad out the rest of the buffer with zeroes
		//~ SDL_LockAudio ();
		memcpy (buffer, user, audio_len);
		//~ memset(buffer + audio_len, 0, len - audio_len);
		//~ SDL_UnlockAudio ();
		 
		if (soundrecenabled) {
			//If recording, make a copy of what was sent to SDL
			memcpy (soundrec + soundrecpos, user, audio_len);
			memset(soundrec + soundrecpos + audio_len, 0, len - audio_len);
			soundrecpos += len;
		}
		
		audio_len = 0;
	} else {
		//printf("f"); fflush(stdout);
		//We have more samples in the emulated buffer than SDL needs.
		//~ SDL_LockAudio ();
		memcpy (buffer, user, len);
		//~ SDL_UnlockAudio ();
		
		if (soundrecenabled) {
			//If recording, make a copy of what was sent to SDL
			memcpy (soundrec + soundrecpos, user, len);
			soundrecpos += len;
		}
		
		//Remove the part of the buffer that was sent to SDL
		audio_len -= len;
 		memmove (user, (unsigned char *) user + len, audio_len);
	}
	if (soundrecenabled && (soundrecpos > 500*1024)) {
		printf("Rec over\n"); fflush(stdout);
		soundrecenabled = 0;
	}
	prof_end(PROF_YMH);
}

#define AICA_BUF_SIZE	(8192)
int Use_SDL_Audio = 1;
uint32 aica_left_ch;
uint32 aica_right_ch;

static short lmixbuff[16*1024];
static short rmixbuff[16*1024];
static short lmixret[16*1024];
static short rmixret[16*1024];
static int lastmixused = 0;	//in samples
void * mixbuffs[2] = {lmixbuff, rmixbuff};
void * mixbuffs2[2] = {lmixret, rmixret};
//~ void * mixbuffs[2] = {Seg_L, Seg_R};
static void Write_Sound_Stereo_Split (short *LDest, short *RDest, const int length);

static void **audio_callback(int samples_requested, int *samples_returned)
{
	if (samples_requested > audio_len) {
		*samples_returned = samples_requested;
		return mixbuffs;
	}
	//~ int origaudio_len = audio_len;
	//~ int origlastmixused = lastmixused;
	//Use up old samples
	int keep_amt = (audio_len - lastmixused)*2;
	if (keep_amt < 0) keep_amt = 0;
 	memmove (lmixbuff, lmixbuff + lastmixused, keep_amt);
 	memmove (rmixbuff, rmixbuff + lastmixused, keep_amt);
	audio_len -= lastmixused;
	if (audio_len < 0)
		audio_len = 0;
 	
 	//How much to remove from next callback
	//~ lastmixused = audio_len > samples_requested ? samples_requested : audio_len; //samples_requested;
	lastmixused = samples_requested;
	//~ printf("(%i)", samples_requested);
	//~ printf("(%i,%i,%i|%i,%i)", samples_requested,origaudio_len,origlastmixused, keep_amt/2,lastmixused); fflush(stdout);
	
	
	
	//Return samples
	*samples_returned = samples_requested;
	//~ *samples_returned = lastmixused;
	
	if (soundrecenabled) {
		//If recording, make a copy of what was sent to SDL
		memcpy (soundrec + soundrecpos, lmixbuff, *samples_returned * 2);
		soundrecpos += samples_requested*2;
		if (soundrecenabled && (soundrecpos > 500*1024)) {
			printf("Rec over\n"); fflush(stdout);
			soundrecenabled = 0;
		}
	}
	
	return mixbuffs;
}

static void **audio_callback2(int samples_requested, int *samples_returned)
{
	//~ printf("(%i)", samples_requested); fflush(stdout);
	if (samples_requested > audio_len) {
		memset(lmixret, 0, samples_requested*2);
		memset(rmixret, 0, samples_requested*2);
		/*
		memcpy(lmixret, lmixbuff, audio_len*2);
		memcpy(rmixret, rmixbuff, audio_len*2);
		int i, lastl = lmixbuff[audio_len-1], lastr = lmixbuff[audio_len-1];
		for(i = audio_len; i < samples_requested; i++) {
			lmixret[i] = lastl;
			rmixret[i] = lastr;
		}*/
		
		*samples_returned = samples_requested;
	} else {
		memcpy(lmixret, lmixbuff, samples_requested*2);
		memcpy(rmixret, rmixbuff, samples_requested*2);
		
		audio_len -= samples_requested;
		memmove(lmixbuff, lmixbuff + samples_requested, audio_len*2);
		memmove(rmixbuff, rmixbuff + samples_requested, audio_len*2);
		
		*samples_returned = samples_requested;
	}
	
	if (soundrecenabled) {
		memcpy (soundrec + soundrecpos, lmixret, *samples_returned * 2);
		soundrecpos += samples_requested*2;
		if (soundrecenabled && (soundrecpos > 500*1024)) {
			printf("Rec over\n"); fflush(stdout);
			soundrecenabled = 0;
		}
	}
	
	return mixbuffs2;
}

int Init_SoundDC(void) {
	int audio_factor, i;
	
	if (Sound_Initialised) {
		gpdc_snd_stream_start(Sound_Rate, Sound_Stereo);
		return 1;
	}
	
	switch (Sound_Rate)
	{
	case 8000:
		if (CPU_Mode)
		Seg_Lenght = 160;
		else
		Seg_Lenght = 134;
		break;

	case 11025:
		if (CPU_Mode)
		Seg_Lenght = 221;	//was 220
		else
		Seg_Lenght = 185;	//was 184
		break;

	case 16000:
		if (CPU_Mode)
		Seg_Lenght = 320;
		else
		Seg_Lenght = 267;
		break;

	case 22050:
		if (CPU_Mode)
		Seg_Lenght = 441;
		else
		Seg_Lenght = 368;
		break;

	case 32000:
		if (CPU_Mode)
		Seg_Lenght = 640;
		else
		Seg_Lenght = 534;
		break;

	case 44100:
		if (CPU_Mode)
		Seg_Lenght = 882;
		else
		Seg_Lenght = 735;
		break;
	}
	
	audio_factor = (CPU_Mode)?312:262;

	for (i = 0; i < audio_factor; i++) {
		Sound_Extrapol[i][0] = ((Seg_Lenght * i) / audio_factor);
		Sound_Extrapol[i][1] =
		(((Seg_Lenght * (i + 1)) / audio_factor) - Sound_Extrapol[i][0]);
	}
	
	for (i = 0; i < Seg_Lenght; i++)
		Sound_Interpol[i] = ((audio_factor * i) / Seg_Lenght);

	memset (Seg_L, 0, Seg_Lenght << 2);
	memset (Seg_R, 0, Seg_Lenght << 2);
	
	int channels = Sound_Stereo ? 2 : 1;
	#define oversizeratio 1
	printf("Audiobuf size: %i\n", (int)((Sound_Rate * channels * 2 * 4 * oversizeratio) * sizeof (short)));
	audiobuf = malloc ((Sound_Rate * channels * 2 * 4 * oversizeratio) * sizeof (short));
	
	
	if (Use_SDL_Audio == 2) {
		aica_left_ch = snd_mem_malloc(AICA_BUF_SIZE);
		aica_right_ch = snd_mem_malloc(AICA_BUF_SIZE);
	} else {
		int ret;
		ret = gpdc_snd_stream_init(audio_callback2);
		printf("Init ret: %i\n", ret);
		gpdc_snd_stream_clear();
		gpdc_snd_stream_start(Sound_Rate, Sound_Stereo);
	}
	//~ game_stream = snd_stream_alloc(DCStreamCB, 8*1024);
	//~ printf("Game stream: %i\n", game_stream);
	//~ snd_stream_queue_enable(game_stream);
	//~ snd_stream_start(game_stream, Sound_Rate, Sound_Stereo);
	//~ snd_stream_queue_go(game_stream);
	
	return (Sound_Initialised = 1);
}

int
Init_Sound (void)
{
	if (!Use_SDL_Audio)
		return Init_SoundDC();
	
	SDL_AudioSpec spec;
	int i;
	int audio_factor;

	if (Sound_Initialised)
	{
		printf("Sound already running\n"); fflush(stdout);
		SDL_PauseAudio (0);
		return 1;
	}

  End_Sound ();
printf("Sound Rate: %i\n", Sound_Rate);
  switch (Sound_Rate)
    {
   case 8000:
      if (CPU_Mode)
	Seg_Lenght = 160;
      else
 	Seg_Lenght = 134;
      break;
      
    case 11025:
      if (CPU_Mode)
	Seg_Lenght = 221;
      else
	Seg_Lenght = 185;
      break;

    case 16000:
      if (CPU_Mode)
	Seg_Lenght = 320;
      else
 	Seg_Lenght = 267;
      break;

    case 22050:
      if (CPU_Mode)
	Seg_Lenght = 441;
      else
 	Seg_Lenght = 368;
      break;
      
    case 32000:
      if (CPU_Mode)
	Seg_Lenght = 640;
      else
 	Seg_Lenght = 534;
      break;

    case 44100:
      if (CPU_Mode)
	Seg_Lenght = 882;
      else
	Seg_Lenght = 735;
      break;
    }

	audio_factor = (CPU_Mode)?312:262;

	for (i = 0; i < audio_factor; i++)
	{
		Sound_Extrapol[i][0] = ((Seg_Lenght * i) / audio_factor);
		Sound_Extrapol[i][1] =
		(((Seg_Lenght * (i + 1)) / audio_factor) - Sound_Extrapol[i][0]);
	}
	
	for (i = 0; i < Seg_Lenght; i++)
		Sound_Interpol[i] = ((audio_factor * i) / Seg_Lenght);

	memset (Seg_L, 0, Seg_Lenght << 2);
	memset (Seg_R, 0, Seg_Lenght << 2);

#ifdef GYM_SUPPORT
	WP = 0;
	RP = 0;
#endif

#if 0
  if (-1 == SDL_InitSubSystem (SDL_INIT_AUDIO))
    {
      return 0;
    }
#endif

	/* Setting up SDL stuff */
	spec.freq = Sound_Rate;
	spec.format = AUDIO_S16SYS;
	spec.channels = (Sound_Stereo)? 2 : 1;
	spec.samples = 1024;
	//~ spec.samples = 800;
	spec.callback = proc;
	#define oversizeratio 1
	printf("Audiobuf size: %i\n", (int)(spec.samples * spec.channels * 2 * 4 * oversizeratio * sizeof (short)));
	audiobuf =
	(unsigned char *) malloc ((spec.samples * spec.channels * 2 * 4 * oversizeratio) *
					sizeof (short));
	spec.userdata = audiobuf;

//   memset (audiobuf, 0,
// 	  (spec.samples * spec.channels * 2 * 4) * sizeof (short));

#ifdef _PSP
  static int audioopen=0;
  if (audioopen==0 && SDL_OpenAudio (&spec, 0) != 0)
#else
  if (SDL_OpenAudio (&spec, 0) != 0)
#endif
    {
      return 0;
    }
#ifdef _PSP
  audioopen=1;
#endif
  SDL_PauseAudio (0);
  return (Sound_Initialised = 1);
}


void
End_Sound ()
{
	if (Use_SDL_Audio) {
		SDL_PauseAudio (1);
	} else {
		gpdc_snd_stream_stop();
	}
  return;	//Keep sound system up for switching between games
//*
//	Commented out so it keeps working when switching games
  if (audiobuf)
  	free (audiobuf);
  audiobuf = NULL;
  if (Sound_Initialised)
    {
      Sound_Is_Playing = 0;
      Sound_Initialised = 0;
    }
#ifndef _PSP
  SDL_QuitSubSystem (SDL_INIT_AUDIO);
#endif
//*/
}

int
Get_Current_Seg (void)
{
  return 0;
}


int
Check_Sound_Timing (void)
{
  return 0;
}


static void Write_Sound_Stereo (short *Dest, const int length)
{
  int i, out_L, out_R;
  short *dest = Dest;

  for (i = 0; i < length; i++)
    {
      out_L = Seg_L[i];
      Seg_L[i] = 0;

      if (out_L < MIN_AUDIOVAL)
	*dest++ = MIN_AUDIOVAL;
      else if (out_L > MAX_AUDIOVAL)
	*dest++ = MAX_AUDIOVAL;
      else
	*dest++ = (short) out_L;

      out_R = Seg_R[i];
      Seg_R[i] = 0;

      if (out_R < MIN_AUDIOVAL)
	*dest++ = MIN_AUDIOVAL;
      else if (out_R > MAX_AUDIOVAL)
	*dest++ = MAX_AUDIOVAL;
      else
	*dest++ = (short) out_R;
    }
}

static void Write_Sound_Stereo_Split (short *LDest, short *RDest, const int length)
{
  int i, out_L, out_R;

  for (i = 0; i < length; i++)
    {
      out_L = Seg_L[i];
      Seg_L[i] = 0;

      if (out_L < MIN_AUDIOVAL)
	*LDest++ = MIN_AUDIOVAL;
      else if (out_L > MAX_AUDIOVAL)
	*LDest++ = MAX_AUDIOVAL;
      else
	*LDest++ = (short) out_L;

      out_R = Seg_R[i];
      Seg_R[i] = 0;

      if (out_R < MIN_AUDIOVAL)
	*RDest++ = MIN_AUDIOVAL;
      else if (out_R > MAX_AUDIOVAL)
	*RDest++ = MAX_AUDIOVAL;
      else
	*RDest++ = (short) out_R;
    }
}


static void Dump_Sound_Stereo (short *Dest, const int length)
{
  int i, out_L, out_R;
  short *dest = Dest;

  for (i = 0; i < length; i++)
    {
      out_L = Seg_L[i];

      if (out_L < MIN_AUDIOVAL)
	*dest++ = MIN_AUDIOVAL;
      else if (out_L > MAX_AUDIOVAL)
	*dest++ = MAX_AUDIOVAL;
      else
	*dest++ = (short) out_L;

      out_R = Seg_R[i];

      if (out_R < MIN_AUDIOVAL)
	*dest++ = MIN_AUDIOVAL;
      else if (out_R > MAX_AUDIOVAL)
	*dest++ = MAX_AUDIOVAL;
      else
	*dest++ = (short) out_R;
    }
}


static void Write_Sound_Mono (short *Dest, const int length)
{
  int i, out;
  short *dest = Dest;

  for (i = 0; i < length; i++)
    {
      out = Seg_L[i] + Seg_R[i];
      Seg_L[i] = Seg_R[i] = 0;

      if (out < -0xFFFF)
	*dest++ = -0x7FFF;
      else if (out > 0xFFFF)
	*dest++ = 0x7FFF;
      else
	*dest++ = (short) (out >> 1);
    }
}


static void Dump_Sound_Mono (short *Dest, const int length)
{
  int i, out;
  short *dest = Dest;

  for (i = 0; i < length; i++)
    {
      out = Seg_L[i] + Seg_R[i];

      if (out < -0xFFFF)
	*dest++ = -0x7FFF;
      else if (out > 0xFFFF)
	*dest++ = 0x7FFF;
      else
	*dest++ = (short) (out >> 1);
    }
}


int Write_Sound_Buffer (void *Dump_Buf)
{
	if (!Sound_Enable)
	{
		return 0;
	}
	
	prof_start(PROF_YMH);
	//~ if (audio_len > (1880*2)) {
	//~ if (audio_len > (2500*2)) {
	if (!Use_SDL_Audio) {
		//~ printf("B"); fflush(stdout);
		if (1) {
			if (audio_len < 4000) {
				Write_Sound_Stereo_Split(lmixbuff+audio_len, rmixbuff+audio_len, Seg_Lenght);
				audio_len += Seg_Lenght;
			} else {
				printf("o");
			}
			goto ret0;
		}
		if (0 && audio_len > (2000*2)) {
			AICA_CMDSTR_CHANNEL(tmp, cmd, chan);
			/* Channel 0 */
			cmd->cmd = AICA_CMD_CHAN;
			cmd->timestamp = 0;
			cmd->size = AICA_CMDSTR_CHANNEL_SIZE;
			cmd->cmd_id = 0;
			chan->cmd = AICA_CH_CMD_START | AICA_CH_START_DELAY;
			chan->base = aica_left_ch;
			chan->type = AICA_SM_16BIT;
			chan->length = AICA_BUF_SIZE/2;
			chan->loop = 1;
			chan->loopstart = 0;
			chan->loopend = AICA_BUF_SIZE/2;
			chan->freq = Sound_Rate;
			chan->vol = 255;
			chan->pan = 0;
			snd_sh4_to_aica(tmp, cmd->size);

			/* Channel 1 */
			cmd->cmd_id = 1;
			chan->base = aica_right_ch;
			chan->pan = 255;
			snd_sh4_to_aica(tmp, cmd->size);

			/* Start both channels simultaneously */
			//~ cmd->cmd_id = (1 << streams[hnd].ch[0]) | (1 << streams[hnd].ch[1]);
			cmd->cmd_id = 0x3;
			chan->cmd = AICA_CH_CMD_START | AICA_CH_START_SYNC;
			snd_sh4_to_aica(tmp, cmd->size);
			
			spu_memload(aica_left_ch, Seg_L, 2000*2);
			spu_memload(aica_right_ch, Seg_R, 2000*2); //Seg_Lenght
			
			audio_len -= 4000;
			
			snd_sh4_to_aica_start();
		}
		goto ret1;
		//~ return 1;
	} else {
		if (audio_len > (3000*2)) {
			//~ printf("Too full\n"); fflush(stdout);
			goto ret0;
			//~ return 0;	//Does anything use this return value?
		}
	}
	
	if (Sound_Stereo)
	{
#if (TARGET_SYS,x86)
		if (Have_MMX)
			Write_Sound_Stereo_MMX (Seg_L, Seg_R, (short *) &audiobuf[audio_len], Seg_Lenght);
		else
#endif
 		Write_Sound_Stereo ((short *) &audiobuf[audio_len], Seg_Lenght);
		audio_len += Seg_Lenght * sizeof(short) * 2;
	}
	else
	{
#if (TARGET_SYS,x86)
		if (Have_MMX)
			Write_Sound_Mono_MMX (Seg_L, Seg_R, (short *) &audiobuf[audio_len], Seg_Lenght);
		else
#endif
		Write_Sound_Mono ((short *) &audiobuf[audio_len], Seg_Lenght);
		audio_len += Seg_Lenght * sizeof(short);
	}

	/*
		TODO: the threshold should be calculated from the amount required 
		by the hardware/sound generator
	*/
/*
	if (audio_len > (1880*2))
		over_time=1;
	else
		over_time=0;
*/
ret1:
	prof_end(PROF_YMH);
	return 1;
ret0:
	prof_end(PROF_YMH);
	return 0;
}


int
Clear_Sound_Buffer (void)
{
#if 0
  LPVOID lpvPtr1;
  DWORD dwBytes1;
  HRESULT rval;
  int i;

  if (!Sound_Initialised)
    return 0;

  rval =
    lpDSBuffer->Lock (0, Seg_Lenght * Sound_Segs * Bytes_Per_Unit, &lpvPtr1,
		      &dwBytes1, NULL, NULL, 0);

  if (rval == DSERR_BUFFERLOST)
    {
      lpDSBuffer->Restore ();
      rval =
	lpDSBuffer->Lock (0, Seg_Lenght * Sound_Segs * Bytes_Per_Unit,
			  &lpvPtr1, &dwBytes1, NULL, NULL, 0);

    }

  if (rval == DS_OK)
    {
      signed short *w = (signed short *) lpvPtr1;

      for (i = 0; i < Seg_Lenght * Sound_Segs * Bytes_Per_Unit; i += 2)
	*w++ = (signed short) 0;

      rval = lpDSBuffer->Unlock (lpvPtr1, dwBytes1, NULL, NULL);

      if (rval == DS_OK)
	return 1;
    }
#endif
  return 0;
}


int
Play_Sound (void)
{
#if 0
  HRESULT rval;

  if (Sound_Is_Playing)
    return 1;

  rval = lpDSBuffer->Play (0, 0, DSBPLAY_LOOPING);

  Clear_Sound_Buffer ();

  if (rval != DS_OK)
    return 0;
#endif
  return (Sound_Is_Playing = 1);
}


int
Stop_Sound (void)
{
#if 0
  HRESULT rval;

  rval = lpDSBuffer->Stop ();

  if (rval != DS_OK)
    return 0;

  Sound_Is_Playing = 0;
#endif
  return 1;
}


int
Start_WAV_Dump (void)
{
#if 0
  char Name[1024] = "";

  if (!(Sound_Is_Playing) || !(Game))
    return (0);

  if (WAV_Dumping)
    {
      Put_Info ("WAV sound is already dumping", 1000);
      return (0);
    }

  strcpy (Name, Dump_Dir);
  strcat (Name, Rom_Name);

  if (WaveCreateFile (Name, &MMIOOut, &MainWfx, &CkOut, &CkRIFF))
    {
      Put_Info ("Error in WAV dumping", 1000);
      return (0);
    }

  if (WaveStartDataWrite (&MMIOOut, &CkOut, &MMIOInfoOut))
    {
      Put_Info ("Error in WAV dumping", 1000);
      return (0);
    }

  Put_Info ("Starting to dump WAV sound", 1000);
  WAV_Dumping = 1;
#endif
  return 1;
}


int
Update_WAV_Dump (void)
{
#if 0
  unsigned char Buf_Tmp[882 * 4 + 16];
  unsigned int lenght, Writted;

  if (!WAV_Dumping)
    return 0;

  Write_Sound_Buffer (Buf_Tmp);

  lenght = Seg_Lenght << 1;

  if (Sound_Stereo)
    lenght *= 2;

  if (WaveWriteFile
      (MMIOOut, lenght, &Buf_Tmp[0], &CkOut, &Writted, &MMIOInfoOut))
    {
      Put_Info ("Error in WAV dumping", 1000);
      return 0;
    }
#endif
  return (1);
}


int
Stop_WAV_Dump (void)
{
#if 0
  if (!WAV_Dumping)
    {
      Put_Info ("Already stopped", 1000);
      return 0;
    }

  if (WaveCloseWriteFile (&MMIOOut, &CkOut, &CkRIFF, &MMIOInfoOut, 0))
    return 0;

  Put_Info ("WAV dump stopped", 1000);
  WAV_Dumping = 0;
#endif
  return 1;
}
#endif
