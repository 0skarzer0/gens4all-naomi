#include "gens4all.h"


int Debug;
int Frame_Skip;
int Frame_Number;
int DAC_Improv;
int RMax_Level;
int GMax_Level;
int BMax_Level;
int Contrast_Level;
int Brightness_Level;
int Greyscale;
int Invert_Color;

unsigned char CD_Data[1024];	// Used for hard reset to know the game name


void Init_Tab (void)
{
	int x, y, dep;

#if 0
  for (x = 0; x < 1024; x++)
    {
      for (y = 0; y < 64; y++)
	{
	  dep = (x & 3) + (((x & 0x3FC) >> 2) << 8);
	  dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
	  dep >>= 1;
	  Cell_Conv_Tab[(x >> 1) + (y << 9)] = (unsigned short) dep;
	}
    }

  for (x = 0; x < 512; x++)
    {
      for (y = 0; y < 64; y++)
	{
	  dep = (x & 3) + (((x & 0x1FC) >> 2) << 8);
	  dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
	  dep >>= 1;
	  Cell_Conv_Tab[(x >> 1) + (y << 8) + 0x8000] =
	    (unsigned short) (dep + 0x8000);
	}
    }

  for (x = 0; x < 256; x++)
    {
      for (y = 0; y < 64; y++)
	{
	  dep = (x & 3) + (((x & 0xFC) >> 2) << 8);
	  dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
	  dep >>= 1;
	  Cell_Conv_Tab[(x >> 1) + (y << 7) + 0xC000] =
	    (unsigned short) (dep + 0xC000);
	}
    }

  for (x = 0; x < 256; x++)
    {
      for (y = 0; y < 32; y++)
	{
	  dep = (x & 3) + (((x & 0xFC) >> 2) << 7);
	  dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
	  dep >>= 1;
	  Cell_Conv_Tab[(x >> 1) + (y << 7) + 0xE000] =
	    (unsigned short) (dep + 0xE000);
	  Cell_Conv_Tab[(x >> 1) + (y << 7) + 0xF000] =
	    (unsigned short) (dep + 0xF000);
	}
    }
#endif

	for (x = 0; x < CycleTabSize; x++)
		Z80_M68K_Cycle_Tab[x] = (int) ((double) x * 7.0 / 15.0);
}

void Recalculate_Palettes (void)
{
	int i;
	int r, g, b;
	int rf, gf, bf;
	int bright, cont;

	for (r = 0; r < 0x10; r++)
    {
		for (g = 0; g < 0x10; g++)
		{
			for (b = 0; b < 0x10; b++)
			{
				rf = (r & 0xE) << 2;
				gf = (g & 0xE) << 2;
				bf = (b & 0xE) << 2;

				rf = (int) (double) (rf) * (int) ((double) (RMax_Level) / 224.0);
				gf = (int) (double) (gf) * (int) ((double) (GMax_Level) / 224.0);
				bf = (int) (double) (bf) * (int) ((double) (BMax_Level) / 224.0);

				// Compute colors here (64 levels)
				bright = Brightness_Level;
				bright -= 100;
				bright *= 32;
				bright /= 100;

				rf += bright;
				gf += bright;
				bf += bright;

				if (rf < 0)
					rf = 0;
				else if (rf > 0x3F)
					rf = 0x3F;
				if (gf < 0)
					gf = 0;
				else if (gf > 0x3F)
					gf = 0x3F;
				if (bf < 0)
					bf = 0;
				else if (bf > 0x3F)
					bf = 0x3F;

				cont = Contrast_Level;
				
				rf = (rf * cont) / 100;
				gf = (gf * cont) / 100;
				bf = (bf * cont) / 100;

				if (rf < 0)
					rf = 0;
				else if (rf > 0x3F)
					rf = 0x3F;
				if (gf < 0)
					gf = 0;
				else if (gf > 0x3F)
					gf = 0x3F;
				if (bf < 0)
					bf = 0;
				else if (bf > 0x3F)
					bf = 0x3F;

				if (Mode_555 & 1)
				{
					rf = (rf >> 1) << 10;
					gf = (gf >> 1) << 5;
				}
				else
				{
					rf = (rf >> 1) << 11;
					gf = (gf >> 0) << 5;
				}

				bf = (bf >> 1) << 0;
				Palette[(b << 8) | (g << 4) | r] = rf | gf | bf;
			}
		}
	}

#ifdef SEGA_32X
	for (i = 0; i < 0x10000; i++)
	{
		b = ((i >> 10) & 0x1F) << 1;
		g = ((i >> 5) & 0x1F) << 1;
		r = ((i >> 0) & 0x1F) << 1;
		
		r = (int) (double) (r) * (int) ((double) (RMax_Level) / 248.0);
		g = (int) (double) (g) * (int) ((double) (GMax_Level) / 248.0);
		b = (int) (double) (b) * (int) ((double) (BMax_Level) / 248.0);

		// Compute colors here (64 levels)
		bright = Brightness_Level;
		bright -= 100;
		bright *= 32;
		bright /= 100;

		r += bright;
		g += bright;
		b += bright;

		if (r < 0)
			r = 0;
		else if (r > 0x3F)
			r = 0x3F;
		if (g < 0)
			g = 0;
		else if (g > 0x3F)
			g = 0x3F;
		if (b < 0)
			b = 0;
		else if (b > 0x3F)
			b = 0x3F;

		cont = Contrast_Level;

		r = (r * cont) / 100;
		g = (g * cont) / 100;
		b = (b * cont) / 100;

		if (r < 0)
			r = 0;
		else if (r > 0x3F)
			r = 0x3F;
		if (g < 0)
			g = 0;
		else if (g > 0x3F)
			g = 0x3F;
		if (b < 0)
			b = 0;
		else if (b > 0x3F)
			b = 0x3F;

		if (Mode_555 & 1)
		{
			r = (r >> 1) << 10;
			g = (g >> 1) << 5;
		}
		else
		{
			r = (r >> 1) << 11;
			g = (g >> 0) << 5;
		}

        b = (b >> 1) << 0;
        _32X_Palette_16B[i] = r | g | b;
    }
#endif

    if (Greyscale)
    {
        for (i = 0; i < 0x1000; i++)
        {
            if (Mode_555 & 1)
            {
                r = ((Palette[i] >> 10) & 0x1F) << 1;
                g = ((Palette[i] >> 5) & 0x1F) << 1;
            }
            else
            {
                r = ((Palette[i] >> 11) & 0x1F) << 1;
                g = (Palette[i] >> 5) & 0x3F;
            }

            b = ((Palette[i] >> 0) & 0x1F) << 1;

            r = (r * (unsigned int) (0.30 * 65536.0)) >> 16;
            g = (g * (unsigned int) (0.59 * 65536.0)) >> 16;
            b = (b * (unsigned int) (0.11 * 65536.0)) >> 16;

            r = g = b = r + g + b;

            if (Mode_555 & 1)
            {
                r = (r >> 1) << 10;
                g = (g >> 1) << 5;
            }
            else
            {
                r = (r >> 1) << 11;
                g = (g >> 0) << 5;
            }

            b = (b >> 1) << 0;
            Palette[i] = r | g | b;
        }

#ifdef SEGA_32X
        for (i = 0; i < 0x10000; i++)
        {
            if (Mode_555 & 1)
            {
                r = ((_32X_Palette_16B[i] >> 10) & 0x1F) << 1;
                g = ((_32X_Palette_16B[i] >> 5) & 0x1F) << 1;
            }
            else
            {
                r = ((_32X_Palette_16B[i] >> 11) & 0x1F) << 1;
                g = (_32X_Palette_16B[i] >> 5) & 0x3F;
            }

            b = ((_32X_Palette_16B[i] >> 0) & 0x1F) << 1;

            r = (r * (unsigned int) (0.30 * 65536.0)) >> 16;
            g = (g * (unsigned int) (0.59 * 65536.0)) >> 16;
            b = (b * (unsigned int) (0.11 * 65536.0)) >> 16;

            r = g = b = r + g + b;

            if (Mode_555 & 1)
            {
                r = (r >> 1) << 10;
                g = (g >> 1) << 5;
            }
            else
            {
                r = (r >> 1) << 11;
                g = (g >> 0) << 5;
            }

            b = (b >> 1) << 0;
            _32X_Palette_16B[i] = r | g | b;
    	}
#endif
    }

    if (Invert_Color)
    {
        for (i = 0; i < 0x1000; i++)
        {
        Palette[i] ^= 0xFFFF;
        }

#ifdef SEGA_32X
        for (i = 0; i < 0x10000; i++)
        {
        _32X_Palette_16B[i] ^= 0xFFFF;
        }
#endif
    }

#ifdef SEGA_32X
	for (i = 0; i < 0x100; i++)
	{
		_32X_VDP_CRam_Ajusted[i] = _32X_Palette_16B[_32X_VDP_CRam[i]];
	}
#endif
}


void Check_Country_Order (void)
{
	if ((Country_Order[0] == Country_Order[1])
		|| (Country_Order[0] == Country_Order[2])
		|| (Country_Order[1] == Country_Order[2])
		|| (Country_Order[0] == Country_Order[2]) || (Country_Order[0] > 2)
		|| (Country_Order[0] < 0) || (Country_Order[1] > 2)
		|| (Country_Order[1] < 0) || (Country_Order[2] > 2)
		|| (Country_Order[2] < 0))
	{
		Country_Order[0] = 0;
		Country_Order[1] = 1;
		Country_Order[2] = 2;
	}
}

#ifdef SEGA_CD
char * Detect_Country_SegaCD (void)
{
	if (CD_Data[0x10B] == 0x64)
	{
		Game_Mode = 1;
		CPU_Mode = 1;
		return EU_CD_Bios;
	}
	else if (CD_Data[0x10B] == 0xA1)
	{
		Game_Mode = 0;
		CPU_Mode = 0;
		return JA_CD_Bios;
	}
	else
	{
		Game_Mode = 1;
		CPU_Mode = 0;
		return US_CD_Bios;
	}
}
#endif

void Detect_Country_Genesis (void)
{
	int c_tab[3] = { 4, 1, 8 };
	int gm_tab[3] = { 1, 0, 1 };
	int cm_tab[3] = { 0, 0, 1 };
	int i, coun = 0;
	char c;

	if (!strnicmp ((char *) &Rom_Data[0x1F0], "eur", 3))
		coun |= 8;
	else if (!strnicmp ((char *) &Rom_Data[0x1F0], "usa", 3))
		coun |= 4;
	else if (!strnicmp ((char *) &Rom_Data[0x1F0], "jap", 3))
		coun |= 1;
	else
		for (i = 0; i < 4; i++)
		{
			c = toupper (Rom_Data[0x1F0 + i]);

			if (c == 'U')
				coun |= 4;
			else if (c == 'J')
				coun |= 1;
			else if (c == 'E')
				coun |= 8;
			else if (c < 16)
				coun |= c;
			else if ((c >= '0') && (c <= '9'))
				coun |= c - '0';
			else if ((c >= 'A') && (c <= 'F'))
				coun |= c - 'A' + 10;
		}

	if (coun & c_tab[Country_Order[0]])
	{
		Game_Mode = gm_tab[Country_Order[0]];
		CPU_Mode = cm_tab[Country_Order[0]];
	}
	else if (coun & c_tab[Country_Order[1]])
	{
		Game_Mode = gm_tab[Country_Order[1]];
		CPU_Mode = cm_tab[Country_Order[1]];
	}
	else if (coun & c_tab[Country_Order[2]])
	{
		Game_Mode = gm_tab[Country_Order[2]];
		CPU_Mode = cm_tab[Country_Order[2]];
	}
	else if (coun & 2)
	{
		Game_Mode = 0;
		CPU_Mode = 1;
	}
	else
	{
		Game_Mode = 1;
		CPU_Mode = 0;
	}
	
	if (Game_Mode)
	{
		if (CPU_Mode)
			Put_Info ("Europe system (50 FPS)", 1500);
		else
			Put_Info ("USA system (60 FPS)", 1500);
	}
	else
	{
		if (CPU_Mode)
			Put_Info ("Japan system (50 FPS)", 1500);
		else
			Put_Info ("Japan system (60 FPS)", 1500);
	}

	if (CPU_Mode)
	{
		VDP_Status |= 0x0001;
#ifdef SEGA_32X
		_32X_VDP.Mode &= ~0x8000;
#endif
	}
	else
	{
		VDP_Status &= 0xFFFE;
#ifdef SEGA_32X
		_32X_VDP.Mode |= 0x8000;
#endif
	}
}
