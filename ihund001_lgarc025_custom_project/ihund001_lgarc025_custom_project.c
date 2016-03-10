/*
 * ihund001_lgarc025_custom_project.c
 *
 * Created: 2/29/2016 10:49:57 PM
 *  Author: IEUser
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include "lib\HX8357_1284.h"
#include "tank.h"
#include "timer.h"
#include "task.h"
#include "utilities.h"

#define T1_MOVE_UP_BIT 3
#define T1_MOVE_DOWN_BIT 1
#define T1_MOVE_LEFT_BIT 0
#define T1_MOVE_RIGHT_BIT 2
#define T1_ROTATE_LEFT_BIT 4
#define T1_ROTATE_RIGHT_BIT 5
#define T1_SHOT_BIT 6

#define T2_MOVE_UP_BIT 5
#define T2_MOVE_DOWN_BIT 2
#define T2_MOVE_LEFT_BIT 1
#define T2_MOVE_RIGHT_BIT 3
#define T2_ROTATE_LEFT_BIT 4
#define T2_ROTATE_RIGHT_BIT 6
#define T2_SHOT_BIT 0


#define NUM_TASKS 11
task tasks[NUM_TASKS];
const unsigned short numTasks = NUM_TASKS;

// Tank Input Handler Tasks States
enum Left_Rotation_Input_Controller_States{LRIC_Start, LRIC_Wait, LRIC_Hold};
enum Right_Rotation_Input_Controller_States{RRIC_Start, RRIC_Wait, RRIC_Hold};
enum Shot_Input_Controller_States{SIC_Start, SIC_Wait, SIC_Hold};
enum Movement_Input_Controller_States{MIC_Start, MIC_Process};

// Tank 1 Input Handlers
int T1_LRIC_tick(int state);
int T1_RRIC_tick(int state);
int T1_SIC_tick(int state);
int T1_MIC_tick(int state);

// Tank 2 Input Handlers
int T2_LRIC_tick(int state);
int T2_RRIC_tick(int state);
int T2_SIC_tick(int state);
int T2_MIC_tick(int state);


// Tank Flasher States
enum Tank_Flasher_States{Flasher_Start, Flasher_Wait, Flasher_LOW, Flasher_HIGH};
int T1_Flasher_tick(int state);
int T2_Flasher_tick(int state);


// Game Engine
enum Game_Engine{GE_Start, GE_Process};
int GE_tick(int state);

// Shots Propagator
enum Shot_Movement_Controller_States{SMC_Start, SMC_Process};
int SMC_tick(int state);

// Tank mover
enum Tank_Movement_States{TM_Start, TM_Process};
int TM_tick(int state);

// Tank Displayer
enum Tank_Display_Handler_States{TDH_Start, TDH_Process};
int TDH_tick(int state);

tank t1;
tank t2;

#define TANK_UP 0
#define TANK_DOWN 1
#define TANK_LEFT 2
#define TANK_RIGHT 3
#define TANK_LR 4
#define TANK_RR 5

int t1_controls[6];
int t2_controls[6];

bullet* shots_arr[MAX_CONCURRENT_SHOTS];

// Helper functions
void game_engine_move_tanks_helper();
void refresh_tanks();

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRD = 0x00; PORTD = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	
	unsigned int display_refresh_rate = 20;
	unsigned int input_rate = 40;
	unsigned long TimePeriodGCD = 20;
	
	memset(shots_arr, 0, 4* sizeof(bullet *));
	memset(t1_controls, 0, 6* sizeof(int));
	memset(t2_controls, 0, 6* sizeof(int));
	
	unsigned char i = 0;
	tasks[i].state = MIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T1_MIC_tick;
	++i;
	tasks[i].state = LRIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T1_LRIC_tick;
	++i;
	tasks[i].state = RRIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T1_RRIC_tick;
	++i;
	tasks[i].state = SIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T1_SIC_tick;
	++i;
	tasks[i].state = MIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T2_MIC_tick;
	++i;
	tasks[i].state = LRIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T2_LRIC_tick;
	++i;
	tasks[i].state = RRIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T2_RRIC_tick;
	++i;
	tasks[i].state = SIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T2_SIC_tick;
	++i;
	tasks[i].state = Flasher_Start;
	tasks[i].period = display_refresh_rate;
	tasks[i].elapsedTime = display_refresh_rate;
	tasks[i].TickFct = &T1_Flasher_tick;
	++i;
	tasks[i].state = Flasher_Start;
	tasks[i].period = display_refresh_rate;
	tasks[i].elapsedTime = display_refresh_rate;
	tasks[i].TickFct = &T2_Flasher_tick;
	++i;
	tasks[i].state = GE_Start;
	tasks[i].period = display_refresh_rate;
	tasks[i].elapsedTime = display_refresh_rate;
	tasks[i].TickFct = &GE_tick;
	
	TimerFlag = 0;
	TimerSet(TimePeriodGCD);
	TimerOn();
	
	SPI_MasterInit();
	displayInit();
	fillScreen(0xFFFF);
	
	initTank(&t1, 100, 50, 'N');
	initTank(&t2, 100, 400, 'S');
	
	printTank(&t1);
	printTank(&t2);
	
	while(1)
	{	
		for ( unsigned char i = 0; i < numTasks; i++ ) {
			if ( tasks[i].elapsedTime >= tasks[i].period ) {
				tasks[i].state = tasks[i].TickFct(tasks[i].state);
				tasks[i].elapsedTime = 0;
			}
			tasks[i].elapsedTime += TimePeriodGCD;
		}
		while(!TimerFlag);
		TimerFlag = 0;	
	}
}

void moveTankFromInput(tank * t1, tank* t2, int up, int down, int left, int right, int lr, int rr){
	int moved = t1->refresh;
	if(up && !down)
		moved |= moveTank(t1, t2, 0, t1->tank_speed);
	else if(down && !up)
		moved |= moveTank(t1, t2, 0, -t1->tank_speed);
	if(left && !right)
		moved |= moveTank(t1, t2, t1->tank_speed, 0);
	else if(right && !left)
		moved |= moveTank(t1, t2, -t1->tank_speed, 0);
	if(lr && !rr)
		moved |= rotateTankLeft(t1, t2);
	else if(rr && !lr)
		moved |= rotateTankRight(t1, t2);
	t1->refresh = moved;
}

int T1_MIC_tick(int state){
	
	unsigned char us_pina = ~PINA;
	unsigned char up = GetBit(us_pina, T1_MOVE_UP_BIT);
	unsigned char down = GetBit(us_pina, T1_MOVE_DOWN_BIT);
	unsigned char left = GetBit(us_pina, T1_MOVE_LEFT_BIT);
	unsigned char right = GetBit(us_pina, T1_MOVE_RIGHT_BIT);
	
	switch(state){
		case MIC_Start:
			state = MIC_Process;
			break;
	}
	
	switch(state){
		case MIC_Process:
			t1_controls[TANK_UP] = up;
			t1_controls[TANK_DOWN] = down;
			t1_controls[TANK_LEFT] = left;
			t1_controls[TANK_RIGHT] = right;
			break;
	}

	return state;
}

int T2_MIC_tick(int state){
	
	unsigned char us_pind = ~PIND;
	unsigned char up = GetBit(us_pind, T2_MOVE_UP_BIT);
	unsigned char down = GetBit(us_pind, T2_MOVE_DOWN_BIT);
	unsigned char left = GetBit(us_pind, T2_MOVE_LEFT_BIT);
	unsigned char right = GetBit(us_pind, T2_MOVE_RIGHT_BIT);
	
	switch(state){
		case MIC_Start:
		state = MIC_Process;
		break;
	}
	
	switch(state){
		case MIC_Process:
		t2_controls[TANK_UP] = up;
		t2_controls[TANK_DOWN] = down;
		t2_controls[TANK_LEFT] = left;
		t2_controls[TANK_RIGHT] = right;
		break;
	}

	return state;
}

int T1_SIC_tick(int state){
	unsigned char us_pina = ~PINA;
	unsigned char shoot = GetBit(us_pina, T1_SHOT_BIT);
	
	switch(state){
		case SIC_Start:
			state = SIC_Wait;
			break;
		case SIC_Wait:
			if(shoot){
				makeShot(&t1, shots_arr);
				state = SIC_Hold;
			}
			break;
		case SIC_Hold:
			if(!shoot){
				state = SIC_Wait;
			}
			break;
	}
	return state;
}

int T2_SIC_tick(int state){
	unsigned char us_pind = ~PIND;
	unsigned char shoot = GetBit(us_pind, T2_SHOT_BIT);
	
	switch(state){
		case SIC_Start:
		state = SIC_Wait;
		break;
		case SIC_Wait:
		if(shoot){
			makeShot(&t2, shots_arr);
			state = SIC_Hold;
		}
		break;
		case SIC_Hold:
		if(!shoot){
			state = SIC_Wait;
		}
		break;
	}
	return state;
}

int T1_LRIC_tick(int state){
	
	unsigned char us_pina = ~PINA;
	unsigned char rotate_left = GetBit(us_pina, T1_ROTATE_LEFT_BIT);
	
	switch(state){
		case LRIC_Start:
			state = LRIC_Wait;
			break;
		case LRIC_Wait:
			if(rotate_left){
				t1_controls[TANK_LR] = 1;
				state = LRIC_Hold;
			}
			break;
		case LRIC_Hold:
			if(!rotate_left){
				state = LRIC_Wait;
			}
			break;
	}
	return state;
}

int T2_LRIC_tick(int state){
	
	unsigned char us_pind = ~PIND;
	unsigned char rotate_left = GetBit(us_pind, T2_ROTATE_LEFT_BIT);
	
	switch(state){
		case LRIC_Start:
		state = LRIC_Wait;
		break;
		case LRIC_Wait:
		if(rotate_left){
			t2_controls[TANK_LR] = 1;
			state = LRIC_Hold;
		}
		break;
		case LRIC_Hold:
		if(!rotate_left){
			state = LRIC_Wait;
		}
		break;
	}
	return state;
}

int T1_RRIC_tick(int state){
	
	unsigned char us_pina = ~PINA;
	unsigned char rotate_right = GetBit(us_pina, T1_ROTATE_RIGHT_BIT);
	
	switch(state){	
		case RRIC_Start:
			state = RRIC_Wait;
			break;
		case RRIC_Wait:
			if(rotate_right){
				t1_controls[TANK_RR] = 1;
				state = RRIC_Hold;
			}
			break;
		case RRIC_Hold:
			if(!rotate_right){
				state = RRIC_Wait;
			}
			break;
	}
	return state;
}

int T2_RRIC_tick(int state){
	
	unsigned char us_pind = ~PIND;
	unsigned char rotate_right = GetBit(us_pind, T2_ROTATE_RIGHT_BIT);
	
	switch(state){
		case RRIC_Start:
		state = RRIC_Wait;
		break;
		case RRIC_Wait:
		if(rotate_right){
			t2_controls[TANK_RR] = 1;
			state = RRIC_Hold;
		}
		break;
		case RRIC_Hold:
		if(!rotate_right){
			state = RRIC_Wait;
		}
		break;
	}
	return state;
}

int T1_Flasher_tick(int state){
	
	static int flashes = 0;
	static int count_int = 0;
	
	switch(state){
		case Flasher_Start:
			state = Flasher_Wait;
			break;
			
		case Flasher_Wait:
			if(t1.flash == 1){
				state = Flasher_HIGH;
				t1.flash = 0;
				count_int = 0;
			}
			break;
		
		case Flasher_HIGH:
			++count_int;
			if(count_int == 1){
				t1.color = HIT_COLOR;
				t1.refresh = 1;
			}
			else if(count_int == 10){
				count_int = 0;
				state = Flasher_LOW;
			}
			break;
			
		case Flasher_LOW:
			++count_int;
			if(count_int == 1){
				t1.color = NORMAL_COLOR;
				t1.refresh = 1;
			}
			else if(count_int == 10){
				++flashes;
				count_int = 0;
				if(flashes == 3){
					flashes = 0;
					state = Flasher_Wait;
				}
				else
					state = Flasher_HIGH;
			}
			break;	
	}
	
	return state;
}

int T2_Flasher_tick(int state){
	static int flashes = 0;
	static int count_int = 0;
	
	switch(state){
		case Flasher_Start:
			state = Flasher_Wait;
			break;
		
		case Flasher_Wait:
		if(t2.flash == 1){
			state = Flasher_HIGH;
			t2.flash = 0;
			count_int = 0;
		}
		break;
		
		case Flasher_HIGH:
		++count_int;
		if(count_int == 1){
			t2.color = HIT_COLOR;
			t2.refresh = 1;
		}
		else if(count_int == 10){
			count_int = 0;
			state = Flasher_LOW;
		}
		break;
		
		case Flasher_LOW:
		++count_int;
		if(count_int == 1){
			t2.color = NORMAL_COLOR;
			t2.refresh = 1;
		}
		else if(count_int == 10){
			++flashes;
			count_int = 0;
			if(flashes == 3){
				flashes = 0;
				state = Flasher_Wait;
			}
			else
			state = Flasher_HIGH;
		}
		break;
	}
	
	return state;
}


int GE_tick(int state){
	switch(state){
		case GE_Start:
			state = GE_Process;
			break;
	}
	
	switch(state){
		case GE_Process:
			// Move bullets
			moveAllShots(shots_arr, &t1, &t2);
			
			// Move tanks
			game_engine_move_tanks_helper();
			
			// Todo-Check if tank got power up
	
			// Redisplay Tanks if needed
			refresh_tanks();
			
			break;
	}
	
	return state;
}

void game_engine_move_tanks_helper(){
	moveTankFromInput(&t1, &t2, t1_controls[TANK_UP], t1_controls[TANK_DOWN], t1_controls[TANK_LEFT], t1_controls[TANK_RIGHT], t1_controls[TANK_LR], t1_controls[TANK_RR] );
	t1_controls[TANK_LR] = 0;
	t1_controls[TANK_RR] = 0;
	moveTankFromInput(&t2, &t1, t2_controls[TANK_UP], t2_controls[TANK_DOWN], t2_controls[TANK_LEFT], t2_controls[TANK_RIGHT], t2_controls[TANK_LR], t2_controls[TANK_RR] );
	t2_controls[TANK_LR] = 0;
	t2_controls[TANK_RR] = 0;
}

void refresh_tanks(){
	if(t1.refresh){
		printTank(&t1);
		t1.refresh = 0;
	}
	if(t2.refresh){
		printTank(&t2);
		t2.refresh = 0;
	}
}