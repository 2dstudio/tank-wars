/*
 * ihund001_lgarc025_custom_project.c
 *
 * Created: 2/29/2016 10:49:57 PM
 *  Author: IEUser
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "lib\HX8357_1284.h"
#include "tank.h"
#include "timer.h"
#include "task.h"
#include "utilities.h"

#define NUM_TASKS 5

#define MOVE_UP_BIT 3
#define MOVE_DOWN_BIT 1
#define MOVE_LEFT_BIT 0
#define MOVE_RIGHT_BIT 2

#define ROTATE_LEFT_BIT 4
#define ROTATE_RIGHT_BIT 5

#define SHOT_BIT 6

#define TANK_MOVE_RATE 5

#define BULLET_QUEUE_SIZE 10

task tasks[NUM_TASKS];
const unsigned short numTasks = NUM_TASKS;

enum Display_Handler_States{DH_Start, DH_Process};
int DH_tick(int state);

enum Left_Rotation_Input_Controller_States{LRIC_Start, LRIC_Wait, LRIC_Hold};
int LRIC_tick(int state);

enum Right_Rotation_Input_Controller_States{RRIC_Start, RRIC_Wait, RRIC_Hold};
int RRIC_tick(int state);

enum Shot_Input_Controller_States{SIC_Start, SIC_Wait, SIC_Hold};
int SIC_tick(int state);

enum Movement_Input_Controller_States{MIC_Start, MIC_Process};
int MIC_tick(int state);

tank t1;

tank t1_old;

shot short_arr[4];

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	
	unsigned int display_refresh_rate = 20;
	unsigned int input_rate = 40;
	unsigned long TimePeriodGCD = 20;
	
	//bullet_queue = QueueInit(BULLET_QUEUE_SIZE);
	
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
	tasks[i].state = DH_Start;
	tasks[i].period = display_refresh_rate;
	tasks[i].elapsedTime = display_refresh_rate;
	tasks[i].TickFct = &DH_tick;
	
	
	TimerFlag = 0;
	TimerSet(TimePeriodGCD);
	TimerOn();
	
	SPI_MasterInit();
	displayInit();
	fillScreen(0xFFFF);
	
	initTank(&t1, 100, 100, 'N');
	t1_old = t1;
	
	printTank(t1);
	
	while(1)
	{	
		t1_old = t1;
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

int MIC_tick(int state){
	
	unsigned char us_pina = ~PINA;
	unsigned char up = GetBit(us_pina, MOVE_UP_BIT);
	unsigned char bottom = GetBit(us_pina, MOVE_DOWN_BIT);
	unsigned char left = GetBit(us_pina, MOVE_LEFT_BIT);
	unsigned char right = GetBit(us_pina, MOVE_RIGHT_BIT);
	
	unsigned char tank_move_rate = TANK_MOVE_RATE;
	
	int d_x = 0;
	int d_y = 0;
	
	switch(state){
		case MIC_Start:
			state = MIC_Process;
			break;
	}
	
	switch(state){
		case MIC_Process:
			if(up)
				d_y += tank_move_rate;
			if(bottom)
				d_y -= tank_move_rate;
			if(left)
				d_x += tank_move_rate;
			if(right)
				d_x -= tank_move_rate;
			moveTank(&t1, d_x, d_y);
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
				//makeShot(&t1);
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
				rotateTankLeft(&t1);
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
				rotateTankRight(&t1);
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

int DH_tick(int state){
	
	switch(state){ //Transitions
		case DH_Start:
			state = DH_Process;
			break;
	}
	
	switch(state){// Actions
		case DH_Process:
			if(tankMoved(& t1_old, & t1)){
				clearTank(t1_old);
				printTank(t1);
			}
			break;
	}
	
	return state;
}