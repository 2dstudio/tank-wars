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

#define MOVE_UP_BIT 3
#define MOVE_DOWN_BIT 1
#define MOVE_LEFT_BIT 0
#define MOVE_RIGHT_BIT 2

#define ROTATE_LEFT_BIT 4
#define ROTATE_RIGHT_BIT 5
#define SHOT_BIT 6

#define TANK_MOVE_RATE 5


#define NUM_TASKS 6
task tasks[NUM_TASKS];
const unsigned short numTasks = NUM_TASKS;

enum Shot_Movement_Controller_States{SMC_Start, SMC_Process};
int SMC_tick(int state);

enum Left_Rotation_Input_Controller_States{LRIC_Start, LRIC_Wait, LRIC_Hold};
int LRIC_tick(int state);

enum Right_Rotation_Input_Controller_States{RRIC_Start, RRIC_Wait, RRIC_Hold};
int RRIC_tick(int state);

enum Shot_Input_Controller_States{SIC_Start, SIC_Wait, SIC_Hold};
int SIC_tick(int state);

enum Movement_Input_Controller_States{MIC_Start, MIC_Process};
int MIC_tick(int state);

enum Tank_Movement_States{TM_Start, TM_Process};
int TM_tick(int state);

tank t1;
tank t2;

#define TANK_UP 0
#define TANK_DOWN 1
#define TANK_LEFT 2
#define TANK_RIGHT 3
#define TANK_LR 4
#define TANK_RR 5

int controls[6];

bullet* shots_arr[MAX_CONCURRENT_SHOTS];

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	
	unsigned int display_refresh_rate = 20;
	unsigned int input_rate = 40;
	unsigned long TimePeriodGCD = 20;
	
	memset(shots_arr, 0, 4* sizeof(bullet *));
	memset(controls, 0, 6* sizeof(int));
	
	unsigned char i = 0;
	tasks[i].state = MIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &MIC_tick;
	++i;
	tasks[i].state = SIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &SIC_tick;
	++i;
	tasks[i].state = LRIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &LRIC_tick;
	++i;
	tasks[i].state = RRIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &RRIC_tick;
	++i;
	tasks[i].state = SMC_Start;
	tasks[i].period = display_refresh_rate;
	tasks[i].elapsedTime = display_refresh_rate;
	tasks[i].TickFct = &SMC_tick;
	++i;
	tasks[i].state = TM_Start;
	tasks[i].period = display_refresh_rate;
	tasks[i].elapsedTime = display_refresh_rate;
	tasks[i].TickFct = &TM_tick;
	
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
		moved |= moveTank(t1, t2, 0, TANK_MOVE_RATE);
	else if(down && !up)
		moved |= moveTank(t1, t2, 0, -TANK_MOVE_RATE);
	if(left && !right)
		moved |= moveTank(t1, t2, TANK_MOVE_RATE, 0);
	else if(right && !left)
		moved |= moveTank(t1, t2, -TANK_MOVE_RATE, 0);
	if(lr && !rr)
		moved |= rotateTankLeft(t1, t2);
	else if(rr && !lr)
		moved |= rotateTankRight(t1, t2);
	if(moved)
		printTank(t1);
}

int MIC_tick(int state){
	
	unsigned char us_pina = ~PINA;
	unsigned char up = GetBit(us_pina, MOVE_UP_BIT);
	unsigned char down = GetBit(us_pina, MOVE_DOWN_BIT);
	unsigned char left = GetBit(us_pina, MOVE_LEFT_BIT);
	unsigned char right = GetBit(us_pina, MOVE_RIGHT_BIT);
	
	switch(state){
		case MIC_Start:
			state = MIC_Process;
			break;
	}
	
	switch(state){
		case MIC_Process:
			controls[TANK_UP] = up;
			controls[TANK_DOWN] = down;
			controls[TANK_LEFT] = left;
			controls[TANK_RIGHT] = right;
			break;
	}

	return state;
}

int SIC_tick(int state){
	unsigned char us_pina = ~PINA;
	unsigned char shoot = GetBit(us_pina, SHOT_BIT);
	
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

int LRIC_tick(int state){
	
	unsigned char us_pina = ~PINA;
	unsigned char rotate_left = GetBit(us_pina, ROTATE_LEFT_BIT);
	
	switch(state){
		case LRIC_Start:
			state = LRIC_Wait;
			break;
		case LRIC_Wait:
			if(rotate_left){
				controls[TANK_LR] = 1;
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

int RRIC_tick(int state){
	
	unsigned char us_pina = ~PINA;
	unsigned char rotate_right = GetBit(us_pina, ROTATE_RIGHT_BIT);
	
	switch(state){	
		case RRIC_Start:
			state = RRIC_Wait;
			break;
		case RRIC_Wait:
			if(rotate_right){
				controls[TANK_RR] = 1;
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

int SMC_tick(int state){
	switch(state){
		case SMC_Start:
			state = SMC_Process;
			break;
	}
	
	switch(state){
		case SMC_Process:
			moveAllShots(shots_arr);
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
			moveTankFromInput(&t1, &t2, controls[TANK_UP], controls[TANK_DOWN], controls[TANK_LEFT], controls[TANK_RIGHT], controls[TANK_LR], controls[TANK_RR] );
			controls[TANK_LR] = 0;
			controls[TANK_RR] = 0;
	}
	
	return state;
}