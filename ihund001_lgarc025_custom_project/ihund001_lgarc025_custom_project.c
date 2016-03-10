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


#define NUM_TASKS 13
task tasks[NUM_TASKS];
const unsigned short numTasks = NUM_TASKS;

// Shots Propagator
enum Shot_Movement_Controller_States{SMC_Start, SMC_Process};
int SMC_tick(int state);

// Input for tank 1
enum T1_Left_Rotation_Input_Controller_States{T1_LRIC_Start, T1_LRIC_Wait, T1_LRIC_Hold};
int T1_LRIC_tick(int state);

enum T1_Right_Rotation_Input_Controller_States{T1_RRIC_Start, T1_RRIC_Wait, T1_RRIC_Hold};
int T1_RRIC_tick(int state);

enum T1_Shot_Input_Controller_States{T1_SIC_Start, T1_SIC_Wait, T1_SIC_Hold};
int T1_SIC_tick(int state);

enum T1_Movement_Input_Controller_States{T1_MIC_Start, T1_MIC_Process};
int T1_MIC_tick(int state);

// Tank 1 Hit Handler;
enum T1_Hit_Handler_States{T1_HH_Start, T1_HH_Wait, T1_HH_LOW, T1_HH_HIGH};
int T1_HH_tick(int state);

// Input for tank 2
enum T2_Left_Rotation_Input_Controller_States{T2_LRIC_Start, T2_LRIC_Wait, T2_LRIC_Hold};
int T2_LRIC_tick(int state);

enum T2_Right_Rotation_Input_Controller_States{T2_RRIC_Start, T2_RRIC_Wait, T2_RRIC_Hold};
int T2_RRIC_tick(int state);

enum T2_Shot_Input_Controller_States{T2_SIC_Start, T2_SIC_Wait, T2_SIC_Hold};
int T2_SIC_tick(int state);

enum T2_Movement_Input_Controller_States{T2_MIC_Start, T2_MIC_Process};
int T2_MIC_tick(int state);

// Tank 2 Hit Handler;
enum T2_Hit_Handler_States{T2_HH_Start, T2_HH_Wait, T2_HH_LOW, T2_HH_HIGH};
int T2_HH_tick(int state);

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
	tasks[i].state = SMC_Start;
	tasks[i].period = display_refresh_rate;
	tasks[i].elapsedTime = display_refresh_rate;
	tasks[i].TickFct = &SMC_tick;
	++i;
	tasks[i].state = T1_MIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T1_MIC_tick;
	++i;
	tasks[i].state = T1_LRIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T1_LRIC_tick;
	++i;
	tasks[i].state = T1_RRIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T1_RRIC_tick;
	++i;
	tasks[i].state = T1_SIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T1_SIC_tick;
	++i;
	tasks[i].state = T2_MIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T2_MIC_tick;
	++i;
	tasks[i].state = T2_LRIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T2_LRIC_tick;
	++i;
	tasks[i].state = T2_RRIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T2_RRIC_tick;
	++i;
	tasks[i].state = T2_SIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T2_SIC_tick;
	++i;
	tasks[i].state = TM_Start;
	tasks[i].period = display_refresh_rate;
	tasks[i].elapsedTime = display_refresh_rate;
	tasks[i].TickFct = &TM_tick;
	++i;
	tasks[i].state = T1_HH_Start;
	tasks[i].period = display_refresh_rate;
	tasks[i].elapsedTime = display_refresh_rate;
	tasks[i].TickFct = &T1_HH_tick;
	++i;
	tasks[i].state = T2_HH_Start;
	tasks[i].period = display_refresh_rate;
	tasks[i].elapsedTime = display_refresh_rate;
	tasks[i].TickFct = &T2_HH_tick;
	++i;
	tasks[i].state = TDH_Start;
	tasks[i].period = display_refresh_rate;
	tasks[i].elapsedTime = display_refresh_rate;
	tasks[i].TickFct = &TDH_tick;
	
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
	int moved = 0;
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
	t1->moved = moved;
}

int T1_MIC_tick(int state){
	
	unsigned char us_pina = ~PINA;
	unsigned char up = GetBit(us_pina, T1_MOVE_UP_BIT);
	unsigned char down = GetBit(us_pina, T1_MOVE_DOWN_BIT);
	unsigned char left = GetBit(us_pina, T1_MOVE_LEFT_BIT);
	unsigned char right = GetBit(us_pina, T1_MOVE_RIGHT_BIT);
	
	switch(state){
		case T1_MIC_Start:
			state = T1_MIC_Process;
			break;
	}
	
	switch(state){
		case T1_MIC_Process:
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
		case T2_MIC_Start:
		state = T2_MIC_Process;
		break;
	}
	
	switch(state){
		case T2_MIC_Process:
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
		case T1_SIC_Start:
			state = T1_SIC_Wait;
			break;
		case T1_SIC_Wait:
			if(shoot){
				makeShot(&t1, shots_arr);
				state = T1_SIC_Hold;
			}
			break;
		case T1_SIC_Hold:
			if(!shoot){
				state = T1_SIC_Wait;
			}
			break;
	}
	return state;
}

int T2_SIC_tick(int state){
	unsigned char us_pind = ~PIND;
	unsigned char shoot = GetBit(us_pind, T2_SHOT_BIT);
	
	switch(state){
		case T2_SIC_Start:
		state = T2_SIC_Wait;
		break;
		case T2_SIC_Wait:
		if(shoot){
			makeShot(&t2, shots_arr);
			state = T2_SIC_Hold;
		}
		break;
		case T2_SIC_Hold:
		if(!shoot){
			state = T2_SIC_Wait;
		}
		break;
	}
	return state;
}

int T1_LRIC_tick(int state){
	
	unsigned char us_pina = ~PINA;
	unsigned char rotate_left = GetBit(us_pina, T1_ROTATE_LEFT_BIT);
	
	switch(state){
		case T1_LRIC_Start:
			state = T1_LRIC_Wait;
			break;
		case T1_LRIC_Wait:
			if(rotate_left){
				t1_controls[TANK_LR] = 1;
				state = T1_LRIC_Hold;
			}
			break;
		case T1_LRIC_Hold:
			if(!rotate_left){
				state = T1_LRIC_Wait;
			}
			break;
	}
	return state;
}

int T2_LRIC_tick(int state){
	
	unsigned char us_pind = ~PIND;
	unsigned char rotate_left = GetBit(us_pind, T2_ROTATE_LEFT_BIT);
	
	switch(state){
		case T2_LRIC_Start:
		state = T2_LRIC_Wait;
		break;
		case T2_LRIC_Wait:
		if(rotate_left){
			t2_controls[TANK_LR] = 1;
			state = T2_LRIC_Hold;
		}
		break;
		case T2_LRIC_Hold:
		if(!rotate_left){
			state = T2_LRIC_Wait;
		}
		break;
	}
	return state;
}

int T1_RRIC_tick(int state){
	
	unsigned char us_pina = ~PINA;
	unsigned char rotate_right = GetBit(us_pina, T1_ROTATE_RIGHT_BIT);
	
	switch(state){	
		case T1_RRIC_Start:
			state = T1_RRIC_Wait;
			break;
		case T1_RRIC_Wait:
			if(rotate_right){
				t1_controls[TANK_RR] = 1;
				state = T1_RRIC_Hold;
			}
			break;
		case T1_RRIC_Hold:
			if(!rotate_right){
				state = T1_RRIC_Wait;
			}
			break;
	}
	return state;
}

int T2_RRIC_tick(int state){
	
	unsigned char us_pind = ~PIND;
	unsigned char rotate_right = GetBit(us_pind, T2_ROTATE_RIGHT_BIT);
	
	switch(state){
		case T2_RRIC_Start:
		state = T2_RRIC_Wait;
		break;
		case T2_RRIC_Wait:
		if(rotate_right){
			t2_controls[TANK_RR] = 1;
			state = T2_RRIC_Hold;
		}
		break;
		case T2_RRIC_Hold:
		if(!rotate_right){
			state = T2_RRIC_Wait;
		}
		break;
	}
	return state;
}

int SMC_tick(int state){
	switch(state){
		case SMC_Start:
			state = SMC_Process;
			break;
	}
	
	switch(state){
		case SMC_Process:
			moveAllShots(shots_arr, &t1, &t2);
			break;
	}
	
	return state;
}

int TM_tick(int state){
	
	switch(state){
		case TM_Start:
			state = TM_Process;
			break;
	}
	
	switch(state){
		case TM_Process:
			moveTankFromInput(&t1, &t2, t1_controls[TANK_UP], t1_controls[TANK_DOWN], t1_controls[TANK_LEFT], t1_controls[TANK_RIGHT], t1_controls[TANK_LR], t1_controls[TANK_RR] );
			t1_controls[TANK_LR] = 0;
			t1_controls[TANK_RR] = 0;
			moveTankFromInput(&t2, &t1, t2_controls[TANK_UP], t2_controls[TANK_DOWN], t2_controls[TANK_LEFT], t2_controls[TANK_RIGHT], t2_controls[TANK_LR], t2_controls[TANK_RR] );
			t2_controls[TANK_LR] = 0;
			t2_controls[TANK_RR] = 0;
	}
	
	return state;
}

int TDH_tick(int state){
	switch(state){
		case TDH_Start:
			state = TDH_Process;
			break;
	}
	
	switch(state){
		case TDH_Process:
			if(t1.moved){
				printTank(&t1);
				t1.moved = 0;
			}
			if(t2.moved){
				printTank(&t2);
				t2.moved = 0;
			}
			break;
	}
	
	return state;
}

int T1_HH_tick(int state){
	
	static int flashes = 0;
	static int count_int = 0;
	
	switch(state){
		case T1_HH_Start:
			state = T1_HH_Wait;
			break;
			
		case T1_HH_Wait:
			if(t1.hit == 1){
				state = T1_HH_HIGH;
				t1.hit = 0;
				count_int = 0;
			}
			break;
		
		case T1_HH_HIGH:
			++count_int;
			if(count_int == 1){
				t1.color = HIT_COLOR;
				t1.moved = 1;
			}
			else if(count_int == 10){
				count_int = 0;
				state = T1_HH_LOW;
			}
			break;
			
		case T1_HH_LOW:
			++count_int;
			if(count_int == 1){
				t1.color = NORMAL_COLOR;
				t1.moved = 1;
			}
			else if(count_int == 10){
				++flashes;
				count_int = 0;
				if(flashes == 3){
					flashes = 0;
					state = T1_HH_Wait;
				}
				else
					state = T1_HH_HIGH;
			}
			break;	
	}
	
	return state;
}

int T2_HH_tick(int state){
	
	static int flashes = 0;
	static int count_int = 0;
	
	switch(state){
		case T2_HH_Start:
		state = T2_HH_Wait;
		break;
		
		case T2_HH_Wait:
		if(t2.hit == 1){
			state = T2_HH_HIGH;
			t2.hit = 0;
			count_int = 0;
		}
		break;
		
		case T2_HH_HIGH:
		++count_int;
		if(count_int == 1){
			t2.color = HIT_COLOR;
			t2.moved = 1;
		}
		else if(count_int == 10){
			count_int = 0;
			state = T2_HH_LOW;
		}
		break;
		
		case T2_HH_LOW:
		++count_int;
		if(count_int == 1){
			t2.color = NORMAL_COLOR;
			t2.moved = 1;
		}
		else if(count_int == 10){
			++flashes;
			count_int = 0;
			if(flashes == 3){
				flashes = 0;
				state = T2_HH_Wait;
			}
			else
			state = T2_HH_HIGH;
		}
		break;
	}
	
	return state;
}