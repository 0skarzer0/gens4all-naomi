
#include "gens4all.h"

#include <time.h>
#include "psg.h"
#include "ym2612.h"
#include "vdp_io.h"
#include "pcm.h"

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

static int audio_len = 0;

unsigned char *audiobuf = 0;
extern int over_time;

#ifndef NO_SOUND
void
proc (void *user, Uint8 * buffer, int len)
{
	user=audiobuf;
	if(!user) {
		memset(buffer,len,0);
		return;
	}
	if (audio_len < len) {
//		printf("S"); fflush(stdout);
		memcpy (buffer, user, audio_len);
		audio_len = 0;
	} else {
//		printf("s"); fflush(stdout);
		SDL_LockAudio ();
		memcpy (buffer, user, len);
		SDL_UnlockAudio ();
		
		audio_len -= len;
 		memcpy (user, (unsigned char *) user + len, audio_len);
	}
}

int
Init_Sound (void)
{
	SDL_AudioSpec spec;
	int i;
	int audio_factor;

	if (Sound_Initialised)
	{
		return 0;
	}

  End_Sound ();
printf("Sound Rate: %i\n", Sound_Rate);
  switch (Sound_Rate)
    {
    case 11025:
      if (CPU_Mode)
	Seg_Lenght = 220;
      else
	Seg_Lenght = 184;
      break;

    case 16000:
      if (CPU_Mode)
	Seg_Lenght = 320;
      else
 	Seg_Lenght = 266;
      break;

    case 22050:
      if (CPU_Mode)
	Seg_Lenght = 441;
      else
 	Seg_Lenght = 368;
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
	spec.callback = proc;
	#define oversizeratio 2
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
  SDL_PauseAudio (1);
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
	//if (audio_len > (1880*2)) {
	if (audio_len > (2500*2)) {
		return;
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

	return 1;
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
