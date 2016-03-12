/*
 * ihund001_lgarc025_custom_project.c
 *
 * Created: 2/29/2016 10:49:57 PM
 *  Author: IEUser
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "lib\HX8357_1284.h"
#include "tank.h"
#include "timer.h"
#include "task.h"
#include "utilities.h"


#define T1_MOVE_UP_BIT 3
#define T1_MOVE_DOWN_BIT 1
#define T1_MOVE_LEFT_BIT 2
#define T1_MOVE_RIGHT_BIT 0
#define T1_ROTATE_LEFT_BIT 5
#define T1_ROTATE_RIGHT_BIT 4
#define T1_SHOT_BIT 6


#define T2_MOVE_UP_BIT 2
#define T2_MOVE_DOWN_BIT 5
#define T2_MOVE_LEFT_BIT 3
#define T2_MOVE_RIGHT_BIT 1
#define T2_ROTATE_LEFT_BIT 4
#define T2_ROTATE_RIGHT_BIT 6
#define T2_SHOT_BIT 0

#define GAME_RESET_BIT 7

#define DEAD_STATE 99

#define NUM_TASKS 13
task tasks[NUM_TASKS];
const unsigned short numTasks = NUM_TASKS;
#define GAME_TASKS 12

// Tank Input Handler Tasks States
enum Left_Rotation_Input_Controller_States{LRIC_Start, LRIC_Wait, LRIC_Hold};
enum Right_Rotation_Input_Controller_States{RRIC_Start, RRIC_Wait, RRIC_Hold};
enum Shot_Input_Controller_States{SIC_Start, SIC_Wait, SIC_Sound_Off, SIC_Load, SIC_Reload, SIC_Hold};
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


// PowerUp Generator
enum Powerup_Gen_States{PG_Start, PG_Process};
int PG_tick(int state);


// Game Engine
enum Game_Engine{GE_Start, GE_Process};
int GE_tick(int state);


// Game Decider
enum GD_States{GD_Start, GD_Menu, GD_Speculate, GD_Decided};
int GD_tick(int state);


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
int output_pc;

bullet* shots_arr[MAX_CONCURRENT_SHOTS];
powerup* powerup_arr[MAX_CONCURRENT_POWERUPS];

#define CANNON_BIT 7
#define SNIPER_BIT 6

// Helper functions
void game_engine_move_tanks_helper();
void refresh_tanks();
void powerup_generator();
void powerup_cleaner();
void Initialise_Game(char, char);
void game_over(char *);
void kill_all_tasks();
void detect_power_up_gain();

int main(void)
{
	
	DDRA = 0x00; PORTA = 0xFF;
	DDRD = 0x00; PORTD = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	
	unsigned int display_refresh_rate = 20;
	unsigned int input_rate = 40;
	unsigned int powerup_rate = 6000;
	unsigned int TimePeriodGCD = 20;
	
	unsigned char i = 0;
	tasks[i].startState = MIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T1_MIC_tick;
	++i;
	tasks[i].startState = LRIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T1_LRIC_tick;
	++i;
	tasks[i].startState = RRIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T1_RRIC_tick;
	++i;
	tasks[i].startState = SIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T1_SIC_tick;
	++i;
	tasks[i].startState = MIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T2_MIC_tick;
	++i;
	tasks[i].startState = LRIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T2_LRIC_tick;
	++i;
	tasks[i].startState = RRIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T2_RRIC_tick;
	++i;
	tasks[i].startState = SIC_Start;
	tasks[i].period = input_rate;
	tasks[i].elapsedTime = input_rate;
	tasks[i].TickFct = &T2_SIC_tick;
	++i;
	tasks[i].startState = Flasher_Start;
	tasks[i].period = display_refresh_rate;
	tasks[i].elapsedTime = display_refresh_rate;
	tasks[i].TickFct = &T1_Flasher_tick;
	++i;
	tasks[i].startState = Flasher_Start;
	tasks[i].period = display_refresh_rate;
	tasks[i].elapsedTime = display_refresh_rate;
	tasks[i].TickFct = &T2_Flasher_tick;
	++i;
	tasks[i].startState = PG_Start;
	tasks[i].period = powerup_rate;
	tasks[i].elapsedTime = powerup_rate;
	tasks[i].TickFct = &PG_tick;
	++i;
	tasks[i].startState = GE_Start;
	tasks[i].period = display_refresh_rate;
	tasks[i].elapsedTime = display_refresh_rate;
	tasks[i].TickFct = &GE_tick;
	++i;
	tasks[i].startState = GD_Start;
	tasks[i].period = display_refresh_rate;
	tasks[i].elapsedTime = display_refresh_rate;
	tasks[i].TickFct = &GD_tick;
	
	
	TimerFlag = 0;
	TimerSet(TimePeriodGCD);
	TimerOn();
	
	SPI_MasterInit();
	displayInit();
	
	kill_all_tasks();
	tasks[12].state = tasks[12].startState;
	
	//Initialise_Game();
	
	while(1)
	{	
		for ( unsigned char i = 0; i < numTasks; i++ ) {
			if ( tasks[i].elapsedTime >= tasks[i].period ) {
				tasks[i].state = tasks[i].TickFct(tasks[i].state);
				tasks[i].elapsedTime = 0;
			}
			tasks[i].elapsedTime += TimePeriodGCD;
		}
		PORTC = output_pc;
		while(!TimerFlag);
		TimerFlag = 0;	
	}
}

int getTankSpeed(const tank * t){
	if(t->super_speed > 0){
		return t->tank_speed * 3;
	}	
	else
		return t->tank_speed;
}

void moveTankFromInput(tank * t1, tank* t2, int up, int down, int left, int right, int lr, int rr){
	int moved = t1->refresh;
	if(up && !down)
		moved |= moveTank(t1, t2, 0, getTankSpeed(t1));
	else if(down && !up)
		moved |= moveTank(t1, t2, 0, -getTankSpeed(t1));
	if(left && !right)
		moved |= moveTank(t1, t2, getTankSpeed(t1), 0);
	else if(right && !left)
		moved |= moveTank(t1, t2, -getTankSpeed(t1), 0);
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

int getReloadTime(const tank * t){
	if(t->fast_reload > 0){
		return 0;
	}
	else
		return t->reload_time;
}

int getLoadTime(const tank * t){
	if(t->fast_reload > 0){
		return 20;
	}
	else
		return t->load_time;
}

int T1_SIC_tick(int state){
	static int loads = 0;
	static int count = 0;
	
	unsigned char us_pina = ~PINA;
	unsigned char shoot = GetBit(us_pina, T1_SHOT_BIT);
	
	switch(state){
		case SIC_Start:
			state = SIC_Wait;
			break;
		case SIC_Wait:
			if(shoot){
				makeShot(&t1, shots_arr);
				output_pc = SetBit(output_pc, CANNON_BIT, 1);
				state = SIC_Sound_Off;
			}
			break;
		case SIC_Sound_Off:
			output_pc = SetBit(output_pc, CANNON_BIT, 0);
			++loads;
			if(loads == 3){
				loads = 0;
				state = SIC_Reload;
			}
			else
				state = SIC_Load;
			break;
		case SIC_Reload:
			++count;
			if(count >= 1 + getReloadTime(&t1)){
				count = 0;
				state = SIC_Load;
			}
			break;
		case SIC_Load:
			++count;
			if(count >= 1 + getLoadTime(&t1)){
				count = 0;
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
	static int loads = 0;
	static int count = 0;
	
	unsigned char us_pind = ~PIND;
	unsigned char shoot = GetBit(us_pind, T2_SHOT_BIT);
	
	switch(state){
		case SIC_Start:
			state = SIC_Wait;
			break;
		case SIC_Wait:
			if(shoot){
				makeShot(&t2, shots_arr);
				output_pc = SetBit(output_pc, CANNON_BIT, 1);
				state = SIC_Sound_Off;
			}
			break;
		case SIC_Sound_Off:
			output_pc = SetBit(output_pc, CANNON_BIT, 0);
			++loads;
			if(loads == 3){
				loads = 0;
				state = SIC_Reload;
			}
			else
				state = SIC_Load;
			break;		
		case SIC_Reload:
			++count;
			if(count >= 1 + getReloadTime(&t2)){
				count = 0;
				state = SIC_Load;
			}
			break;
		case SIC_Load:
			++count;
			if(count >= 1 + getLoadTime(&t2)){
				count = 0;
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
				t1.color = t1.flash_color;
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
			t2.color = t2.flash_color;
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

int PG_tick(int state){
	switch(state){
		case PG_Start:
			state = PG_Process;
			break;
	}
	
	switch(state){
		case PG_Process:
			if(rand()%2 == 0)
				powerup_generator();
			powerup_cleaner();
			break;
	}
	
	return state;
}

void decay_power_ups(){
	if(t1.super_speed > 0)
		--t1.super_speed;
	if(t1.fast_reload > 0)
		--t1.fast_reload;
	if(t2.super_speed > 0)
		--t2.super_speed;
	if(t2.fast_reload > 0)
		--t2.fast_reload;
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
			moveAllShots(shots_arr, &t1, &t2, powerup_arr);
			
			// Move tanks
			game_engine_move_tanks_helper();
			
			// Apply power up
			detect_power_up_gain();
			
			// Decay Power ups
			decay_power_ups();
	
			// Redisplay Tanks if needed
			refresh_tanks();		
			
			break;
	}
	
	return state;
}

void applyPowerUp(powerup * p, tank * t){
	if(p->type == 'H'){
		t->health += 100;
	}
	else if(p->type == 'B'){
		t->fast_reload = 1000;
	}
	else {
		t->super_speed = 1000;
	}
}


void detect_power_up_gain(){
	window powerup_window, t1_body_window, t1_cannon_window, t2_body_window, t2_cannon_window;
	
	// Check if powerUp collides with a tank;
	getTankBodyWindow(&t1, &t1_body_window);
	getTankCannonWindow(&t1, &t1_cannon_window);
	getTankBodyWindow(&t2, &t2_body_window);
	getTankCannonWindow(&t2, &t2_cannon_window);
	
	for(int i=0; i<MAX_CONCURRENT_POWERUPS; ++i){
		if(powerup_arr[i]!=NULL){
			getPowerUpWindow(powerup_arr[i], &powerup_window);
			if(windowsCollide(&powerup_window,&t1_body_window) || windowsCollide(&powerup_window,&t1_cannon_window)){
				applyPowerUp(powerup_arr[i], &t1);
				t1.flash = 1;
				t1.flash_color = powerup_arr[i]->color;
				clearPowerUp(powerup_arr[i]);
				free(powerup_arr[i]);
				powerup_arr[i] = NULL;	
			}
			if(windowsCollide(&powerup_window,&t2_body_window) || windowsCollide(&powerup_window,&t2_cannon_window)){
				applyPowerUp(powerup_arr[i], &t2);
				t2.flash = 1;
				t2.flash_color = powerup_arr[i]->color;
				clearPowerUp(powerup_arr[i]);
				free(powerup_arr[i]);
				powerup_arr[i] = NULL;
			}
		}
	}
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

int powerUpCollides(const powerup * p){
	window powerup_window, bullet_window, t1_body_window, t1_cannon_window, t2_body_window, t2_cannon_window;
	
	getPowerUpWindow(p, &powerup_window);
	
	// Check if powerUp collides with a tank;
	getTankBodyWindow(&t1, &t1_body_window);
	getTankCannonWindow(&t1, &t1_cannon_window);
	getTankBodyWindow(&t2, &t2_body_window);
	getTankCannonWindow(&t2, &t2_cannon_window);
	
	if(windowsCollide(&powerup_window,&t1_body_window) || windowsCollide(&powerup_window,&t1_cannon_window)){
		return 1;
	}
	if(windowsCollide(&powerup_window,&t2_body_window) || windowsCollide(&powerup_window,&t2_cannon_window)){
		return 2;
	}
	
	//Check if powerUp collides with a bullet
	for(int i=0; i<MAX_CONCURRENT_SHOTS; ++i){
		if(shots_arr[i]!=NULL){
			getBulletWindow(shots_arr[i], &bullet_window);
			if(windowsCollide(&powerup_window, &bullet_window)){
				return 3;	
			}
		}
	}
	
	//Check if powerup collides with another powerup
	for(int i=0; i<MAX_CONCURRENT_POWERUPS; ++i){
		if(powerup_arr[i]!=NULL){
			window powerup_win2;
			getPowerUpWindow(powerup_arr[i], &powerup_win2);
			if(windowsCollide(&powerup_window, &powerup_win2)){
				return 4;
			}
		}
	}
	
	return 0;
}

powerup* generateRandomPowerUp(){
	int x = rand()%(SCREEN_X_MAX - POWERUP_WIDTH);
	int y = rand()%(SCREEN_Y_MAX - POWERUP_HEIGHT);
	//int x = 100;
	//int y = 200;
	
	int tr = rand()%3;
	//int tr = 0;
	char type;
	switch(tr){
		case 0:
			type='H';
			break;
		case 1:
			type='B';
			break;
		case 2:
			type='S';
			break;
		default:
			type='H';
	}
	powerup * p = malloc(sizeof(powerup));
	initPowerUp(p, x, y, type);
	return p;
}

powerup * generateValidPowerUp(){
	for(int i=0; i<VALID_POWERUP_TRIES; ++i){
		powerup * p = generateRandomPowerUp();
		if(!powerUpCollides(p))
			return p;
		free(p);
	}
	return NULL;
}

void powerup_cleaner(){
	//Loop to delete Power ups
	for (int i=0; i<MAX_CONCURRENT_POWERUPS; ++i){
		if(powerup_arr[i]!=NULL){
			++powerup_arr[i]->age;
			if(powerup_arr[i]->age == 3){
				clearPowerUp(powerup_arr[i]);
				free(powerup_arr[i]);
				powerup_arr[i] = NULL;
			}
		}
	}
}

void powerup_generator(){
	//Loop to generate power ups
	for (int i=0; i<MAX_CONCURRENT_POWERUPS; ++i){
		if(powerup_arr[i]==NULL){
			//generate_powerup_here
			powerup_arr[i] = generateValidPowerUp();
			if(powerup_arr[i]!=NULL){
				printPowerUp(powerup_arr[i]);
				return;
			}
		}
	}
}

void kill_all_tasks(){
	for(int i=0; i<GAME_TASKS; ++i){
		tasks[i].state = DEAD_STATE;
	}
}

void game_over(char * output){
	char t1_score[20];
	char t2_score[20];
	sprintf(t1_score, "T1: %d", t1.health);
	sprintf(t2_score, "T2: %d", t2.health);
	kill_all_tasks();
	fillScreen(0xFFFF);
	drawString(80, 80, output, 0X0000, 4);
	drawString(80, 160, "Press 2", 0X0000, 4);
	drawString(110, 200, "To", 0X0000, 4);
	drawString(80, 240, "Restart", 0X0000, 4);
	drawString(80, 280, t1_score , 0X0000, 4);
	drawString(80, 320, t2_score , 0X0000, 4);
}

void main_Menu(){
	fillScreen(0xFFFF);
	drawString(40, 80, "Welcome to", 0X0000, 4);
	drawString(40, 120, "TankWars", 0X0000, 4);
	drawString(40, 280, "Select Tank", 0X0000, 4);
}

void tank1Selected(){
	drawString(80, 320, "T1 Ready", 0X0000, 4);
}

void tank2Selected(){
	drawString(80, 360, "T2 Ready", 0X0000, 4);
}

int GD_tick(int state){
	static int t1_selected = 0;
	static int t2_selected = 0;
	
	unsigned char us_pina = ~PINA;
	unsigned char us_pind = ~PIND;
	unsigned char reset_t1 = GetBit(us_pina, GAME_RESET_BIT);
	unsigned char reset_t2 = GetBit(us_pind, GAME_RESET_BIT);
	
	unsigned char t1_type1 = GetBit(us_pina, T1_SHOT_BIT);
	unsigned char t2_type1 = GetBit(us_pind, T2_SHOT_BIT);
	
	switch(state){
		case GD_Start:
			main_Menu();
			state = GD_Menu;
			break;
		case GD_Menu:
			if(t1_selected == 0){
				if(t1_type1){
					t1_selected = 1;
					tank1Selected();
				}
			}
			if(t2_selected == 0){
				if(t2_type1){
					t2_selected = 1;
					tank2Selected();				
				}
			}
			if(t1_selected && t2_selected){
				Initialise_Game(t1_selected, t2_selected);
				t1_selected = 0;
				t2_selected = 0;
				state = GD_Speculate;
			}
			break;
		case GD_Speculate:
			if(t1.health <= 0 && t2.health > 0){
				game_over("T2 won");	
				state = GD_Decided;	
			}
			else if(t2.health <= 0 && t1.health > 0){
				game_over("T1 won");
				state = GD_Decided;
			}
			else if(t1.health <= 0 && t2.health <=0){
				game_over("Draw");
				state = GD_Decided;
			}
			break;
		case GD_Decided:
			if(reset_t1 || reset_t2){
				state = GD_Start;
			}
	}
	
	return state;
}

void freeArrays(){
	
	//Free power ups array
	for (int i=0; i<MAX_CONCURRENT_POWERUPS; ++i){
		if(powerup_arr[i]!=NULL){
			free(powerup_arr[i]);
			powerup_arr[i] = NULL;
		}
	}
	
	//Free Shots Array
	for (int i=0; i<MAX_CONCURRENT_SHOTS; ++i){
		if(shots_arr[i] != NULL){
			free(shots_arr[i]);
			shots_arr[i] = NULL;
		}
	}
}

void Initialise_Game(char t1_type, char t2_type){
	output_pc = 0;
	fillScreen(0xFFFF);
	
	freeArrays();
	
	for(int i=0; i<NUM_TASKS; ++i){
		tasks[i].state = tasks[i].startState;
	}
	
	
	memset(shots_arr, 0, MAX_CONCURRENT_SHOTS* sizeof(bullet *));
	memset(powerup_arr, 0, MAX_CONCURRENT_POWERUPS* sizeof(powerup *));
	memset(t1_controls, 0, 6* sizeof(int));
	memset(t2_controls, 0, 6* sizeof(int));
		
	
	initTank(&t1, 100, 50, 'N', t1_type);
	initTank(&t2, 100, 400, 'S', t2_type);
	
	printTank(&t1);
	printTank(&t2);
}