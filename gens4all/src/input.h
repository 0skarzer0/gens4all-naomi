#ifndef G_INPUT_H
#define G_INPUT_H

struct K_Def {
	unsigned int Start, Mode, A, B, C, X, Y, Z, Up, Down, Left, Right;
};

extern struct K_Def Keys_Def[8];
extern unsigned char Kaillera_Keys[16];

int Check_Key_Pressed(unsigned int key);

#define MAX_CONTROLLERS	(4)

enum {
	GENC_NULL,
	GENC_A,
	GENC_B,
	GENC_C,
	GENC_X,
	GENC_Y,
	GENC_Z,
	GENC_START,
	GENC_MODE,
	GENC_UP,
	GENC_DOWN,
	GENC_LEFT,
	GENC_RIGHT,
	GENC_COUNT,
};

#define CONT_4BUTTON 0
#define CONT_6BUTTON 1

typedef struct {
	int a,b,c,x,y,z;
	int s, md;	//start
	int u,d,l,r;	//D-pad directions
	int au,ad,al,ar;	//Analog directions
	int sl, sr;	//Shoulder buttons
} EmuInputButtons;
#define DC_BUTTON_COUNT	(sizeof(EmuInputButtons) / sizeof(int))

typedef struct {
	//click, hold, last (0 = not pressed, 1 = pressed)
	EmuInputButtons c, h, l;
	//holdtime (value is time remaining until click)
	EmuInputButtons ht;
	//Prevents button from clicking or holding until released
	EmuInputButtons block;
	
	float ax, ay;	//Analog stick
	float sl, sr;	//Shoulder buttons
	int type;
} EmuInput;

extern EmuInput PlayerCont[4];
extern EmuInput *UICont;
void Block_Hold(void);
void Block_Click(void);

int Init_Input(void);
void End_Input(void);
void Update_Input(void);
void Update_Controllers(void);
void Scan_Player_Net(int Player);
void Update_Controllers_Net(int num_player);

void open_joystick();
void close_joystick();

extern SDL_Joystick* joy[2];

#define CHECK_BUTTON(player, ctrl, button)				\
	if (Check_Key_Pressed(Keys_Def[player].button)) 	\
		Controller_ ## ctrl ## _ ## button = 0; 		\
	else Controller_ ## ctrl ## _ ## button = 1;

#define CHECK_DIR(player, ctrl)							\
	if (Check_Key_Pressed(Keys_Def[player].Up)) 		\
	{									   				\
		Controller_ ## ctrl ## _Up = 0;			   		\
		Controller_ ## ctrl ## _Down = 1;				\
	}													\
	else												\
	{													\
		Controller_ ## ctrl ## _Up = 1;					\
		CHECK_BUTTON(player, ctrl, Down)				\
	}													\
	if (Check_Key_Pressed(Keys_Def[player].Left)) 		\
	{									   				\
		Controller_ ## ctrl ## _Left = 0;			   	\
		Controller_ ## ctrl ## _Right = 1;				\
	}													\
	else												\
	{													\
		Controller_ ## ctrl ## _Left = 1;				\
		CHECK_BUTTON(player,ctrl, Right)				\
	}
		
#define CHECK_ALL_BUTTONS(player, ctrl)					\
	CHECK_BUTTON(player, ctrl, Start)					\
	CHECK_BUTTON(player, ctrl, A)						\
	CHECK_BUTTON(player, ctrl, B)						\
	CHECK_BUTTON(player, ctrl, C)						\
														\
	if (Controller_ ## ctrl ## _Type & 1)				\
	{													\
		CHECK_BUTTON(player, ctrl, Mode)				\
		CHECK_BUTTON(player, ctrl, X)					\
		CHECK_BUTTON(player, ctrl, Y)					\
		CHECK_BUTTON(player, ctrl, Z)					\
	}
	
#define CHECK_PLAYER_PAD(player, ctrl)	\
	CHECK_DIR(player, ctrl)				\
	CHECK_ALL_BUTTONS(player, ctrl)
#endif
