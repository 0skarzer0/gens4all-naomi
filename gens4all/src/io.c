/*****************************************************************************/
/* Modulo controlador IO                                                     */
/* Autor: Oscar Orallo Pelaez                                                */
/* Fecha: 18-02-2007                                                         */
/*****************************************************************************/

#include "io.h"

#define u32 unsigned int
#define u16 unsigned short
#define u8 unsigned char


u32 Controller_1_State = 0x40;
u32 Controller_1_COM = 0x0;
u32 Controller_1_Counter = 0;
u32 Controller_1_Delay = 0;
u32 Controller_1_Type = 0;
u32 Controller_1_Up = 1;
u32 Controller_1_Down = 1;
u32 Controller_1_Left = 1;
u32 Controller_1_Right = 1;
u32 Controller_1_Start = 1;
u32 Controller_1_Mode = 1;
u32 Controller_1_A = 1;
u32 Controller_1_B = 1;
u32 Controller_1_C = 1;
u32 Controller_1_X = 1;
u32 Controller_1_Y = 1;
u32 Controller_1_Z = 1;

u32 Controller_2_State = 0x40;
u32 Controller_2_COM = 0x0;
u32 Controller_2_Counter = 0;
u32 Controller_2_Delay = 0;
u32 Controller_2_Type = 0;
u32 Controller_2_Up = 1;
u32 Controller_2_Down = 1;
u32 Controller_2_Left = 1;
u32 Controller_2_Right = 1;
u32 Controller_2_Start = 1;
u32 Controller_2_Mode = 1;
u32 Controller_2_A = 1;
u32 Controller_2_B = 1;
u32 Controller_2_C = 1;
u32 Controller_2_X = 1;
u32 Controller_2_Y = 1;
u32 Controller_2_Z = 1;

u32 Controller_1B_Type = 0;
u32 Controller_1B_Up = 1;
u32 Controller_1B_Down = 1;
u32 Controller_1B_Left = 1;
u32 Controller_1B_Right = 1;
u32 Controller_1B_Start = 1;
u32 Controller_1B_Mode = 1;
u32 Controller_1B_A = 1;
u32 Controller_1B_B = 1;
u32 Controller_1B_C = 1;
u32 Controller_1B_X = 1;
u32 Controller_1B_Y = 1;
u32 Controller_1B_Z = 1;

u32 Controller_1C_Type = 0;
u32 Controller_1C_Up = 1;
u32 Controller_1C_Down = 1;
u32 Controller_1C_Left = 1;
u32 Controller_1C_Right = 1;
u32 Controller_1C_Start = 1;
u32 Controller_1C_Mode = 1;
u32 Controller_1C_A = 1;
u32 Controller_1C_B = 1;
u32 Controller_1C_C = 1;
u32 Controller_1C_X = 1;
u32 Controller_1C_Y = 1;
u32 Controller_1C_Z = 1;

u32 Controller_1D_Type = 0;
u32 Controller_1D_Up = 1;
u32 Controller_1D_Down = 1;
u32 Controller_1D_Left = 1;
u32 Controller_1D_Right = 1;
u32 Controller_1D_Start = 1;
u32 Controller_1D_Mode = 1;
u32 Controller_1D_A = 1;
u32 Controller_1D_B = 1;
u32 Controller_1D_C = 1;
u32 Controller_1D_X = 1;
u32 Controller_1D_Y = 1;
u32 Controller_1D_Z = 1;

u32 Controller_2B_Type = 0;
u32 Controller_2B_Up = 1;
u32 Controller_2B_Down = 1;
u32 Controller_2B_Left = 1;
u32 Controller_2B_Right = 1;
u32 Controller_2B_Start = 1;
u32 Controller_2B_Mode = 1;
u32 Controller_2B_A = 1;
u32 Controller_2B_B = 1;
u32 Controller_2B_C = 1;
u32 Controller_2B_X = 1;
u32 Controller_2B_Y = 1;
u32 Controller_2B_Z = 1;

u32 Controller_2C_Type = 0;
u32 Controller_2C_Up = 1;
u32 Controller_2C_Down = 1;
u32 Controller_2C_Left = 1;
u32 Controller_2C_Right = 1;
u32 Controller_2C_Start = 1;
u32 Controller_2C_Mode = 1;
u32 Controller_2C_A = 1;
u32 Controller_2C_B = 1;
u32 Controller_2C_C = 1;
u32 Controller_2C_X = 1;
u32 Controller_2C_Y = 1;
u32 Controller_2C_Z = 1;

u32 Controller_2D_Type = 0;
u32 Controller_2D_Up = 1;
u32 Controller_2D_Down = 1;
u32 Controller_2D_Left = 1;
u32 Controller_2D_Right = 1;
u32 Controller_2D_Start = 1;
u32 Controller_2D_Mode = 1;
u32 Controller_2D_A = 1;
u32 Controller_2D_B = 1;
u32 Controller_2D_C = 1;
u32 Controller_2D_X = 1;
u32 Controller_2D_Y = 1;
u32 Controller_2D_Z = 1;




u8 RD_Controller_1(void)
{
	// TEAM PLAYER on port 1 ?
	if (Controller_1_Type & 0x10)
	{
		// TODO
		return 0;
	}
	else
	{
		u8 ret_val1, ret_val2;

		u32 sel = ((Controller_1_State >> 4) & 0x04) >> 2;

		// 6-button controller
		if (Controller_1_Type & 1)
			sel += (Controller_1_Counter & 0x03) << 1;

		switch(sel)
		{
			case 1:	// first high
			case 3:	// second high
			case 5:	// third high
				ret_val1 = (((Controller_1_B << 2) | Controller_1_Left) << 2) | Controller_1_Up;
				ret_val2 = (((Controller_1_C << 2) | Controller_1_Right) << 2) | Controller_1_Down;

				return ret_val1 + 0x40 + (ret_val2 << 1);

			case 0:	// first low
			case 2:	// second low
				ret_val1 = (Controller_1_Start << 4) | Controller_1_Down;
				ret_val2 = (Controller_1_A << 4) | Controller_1_Up;

				return (ret_val1 << 1) + ret_val2;

			case 4:	// third low
				return ((Controller_1_Start << 1) + Controller_1_A) << 4;

			case 6:	// fourth low
				return (((Controller_1_Start << 1) + Controller_1_A) << 4) | 0x0F;

			case 7:	// fourth high
				ret_val1 = (((Controller_1_C << 2) | Controller_1_Mode) << 2) | Controller_1_Y;
				ret_val2 = (((Controller_1_B << 2) | Controller_1_X) << 2) | Controller_1_Z;

				return (ret_val1 << 1) + ret_val2 + 0x40;
		}
	}
}

u8 RD_Controller_2(void)
{
	// TEAM PLAYER on port 2 ?
	if (Controller_2_Type & 0x10)
	{
		// TODO
		return 0;
	}
	else
	{
		u8 ret_val1, ret_val2;

		u32 sel = ((Controller_2_State >> 4) & 0x04) >> 2;

		// 6-button controller
		if (Controller_2_Type & 1)
			sel += (Controller_2_Counter & 0x03) << 1;

		switch(sel)
		{
			case 1:	// first high
			case 3:	// second high
			case 5:	// third high
				ret_val1 = (((Controller_2_B << 2) | Controller_2_Left) << 2) | Controller_2_Up;
				ret_val2 = (((Controller_2_C << 2) | Controller_2_Right) << 2) | Controller_2_Down;

				return (ret_val1 + 0x40) + (ret_val2 << 1);

			case 0:	// first low
			case 2:	// second low
				ret_val1 = (Controller_2_Start << 4) | Controller_2_Down;
				ret_val2 = (Controller_2_A << 4) | Controller_2_Up;

				return (ret_val1 << 1) + ret_val2;

			case 4:	// third low
				return ((Controller_2_Start << 1) + Controller_2_A) << 4;

			case 6:	// fourth low
				return (((Controller_2_Start << 1) + Controller_2_A) << 4) | 0x0F;

			case 7:	// fourth high
				ret_val1 = (((Controller_2_C << 2) | Controller_2_Mode) << 2) | Controller_2_Y;
				ret_val2 = (((Controller_2_B << 2) | Controller_2_X) << 2) | Controller_2_Z;

				return (ret_val1 << 1) + ret_val2 + 0x40;
		}
	}
}

u8 WR_Controller_1(u8 data)
{
	if ((!(Controller_1_State & 0x40)) && (data & 0x40))
	{
		Controller_1_Counter++;
	}

	// line 1 down
	Controller_1_Delay = 0;

	if (Controller_1_Type & 0x10)
	{
		// Team player
		if (!(Controller_1_State & 0x20) && (data & 0x20))
		{
			Controller_1_Counter += 0x0100;
		}

		// line 2 down
	}

	Controller_1_State = data;
}

u8 WR_Controller_2(unsigned char data)
{
	if ((!(Controller_2_State & 0x40)) && (data & 0x40))
	{
		Controller_2_Counter++;
	}

	// line 1 down
	if ((Controller_1_Type & 0x10) && (data & 0xC))
	{
		Controller_1_Counter = 0;
	}

	// No TP1
	Controller_2_Delay = 0;

	if (Controller_2_Type & 0x10)
	{
		// Team player
		if (!(Controller_2_State & 0x20) && (data & 0x20))
		{
			Controller_2_Counter += 0x0100;
		}

		// line 2 down
		Controller_2_State = data;
	}
	else
	{
		Controller_2_State = data;
	}
}

void Fix_Controllers(void)
{
	if (Controller_1_Delay > 25)
	{
		// delay 1 expired
		Controller_1_Counter = 0;
	}
	else
	{
		Controller_1_Delay++;
	}

	if (Controller_2_Delay > 25)
	{
		// delay 2 expired
		Controller_2_Counter = 0;
	}
	else
	{
		Controller_2_Delay++;
	}
}

// TODO
void Make_IO_Table(void)
{
}
