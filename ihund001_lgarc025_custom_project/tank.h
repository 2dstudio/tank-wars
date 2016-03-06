#ifndef TANK_H
#define TANK_H

#define MACRO_TANK_LENGTH 35
#define MACRO_TANK_WIDTH 35
#define MACRO_CANNON_WIDTH 7
#define MACRO_CANNON_LENGTH 15

#define MACRO_SHOT_SPEED 10
#define MACRO_SHOT_HEIGHT 7
#define MACRO_SHOT_WIDTH 7

#define SCREEN_X_MAX 320
#define SCREEN_X_MIN 0
#define SCREEN_Y_MAX 480
#define SCREEN_Y_MIN 0

#define MAX_CONCURRENT_SHOTS 4

typedef struct _tank {
	int x;
	int y;
	unsigned int tank_length;
	unsigned int tank_width;
	unsigned int cannon_length;
	unsigned int cannon_width;
	char direction;
} tank;

typedef struct shot {
	int x;
	int y;
	int shot_height;
	int shot_width;
	unsigned int speed;
	char direction;
} shot;

typedef struct window {
	int l_x;
	int l_y;
	int u_x;
	int u_y;
} window;

void initWindow(window * w, int l_x, int l_y, int u_x, int u_y){
	w->l_x = l_x;
	w->l_y = l_y;
	w->u_x = u_x;
	w->u_y = u_y;
}

void colorTank(tank * t, unsigned int color);
int tankInBounds(tank *t);
void getCannonHead(const tank * t, int * cannon_x, int * cannon_y);
void clearTank(tank * t);
void printTank(tank * t);

void initShot(shot * s, int x, int y, char direction, int shot_height, int shot_width, unsigned int speed){
	s->x = x;
	s->y = y;
	s->shot_height = shot_height;
	s->shot_width = shot_width;
	s->speed = speed;
	s->direction = direction;
}

shot * createShot(tank * t){
	int cannon_x, cannon_y;
	getCannonHead(t, &cannon_x, &cannon_y);
	shot * s = malloc(sizeof(struct shot));
	initShot(s, cannon_x, cannon_y, t->direction, MACRO_SHOT_HEIGHT, MACRO_SHOT_WIDTH, MACRO_SHOT_SPEED);
	return s;
}

int tankMoved(tank * t_old, tank * t){
	if((t_old->x != t->x) || (t_old->y != t->y)){
		return 1;
	}
	
	if(t_old->direction != t->direction){
		return 1;
	}
	
	return 0;
}

void moveTankInternal(tank * t, int d_x, int d_y, char left, char right){
	t->x += d_x;
	t->y += d_y;
	
	if(left){
		if(t->direction == 'N')
		t->direction = 'E';
		else if (t->direction == 'E')
		t->direction = 'S';
		else if(t->direction == 'S')
		t->direction = 'W';
		else if(t->direction == 'W')
		t->direction = 'N';
	}
	
	if(right){
		if(t->direction == 'N')
		t->direction = 'W';
		else if (t->direction == 'W')
		t->direction = 'S';
		else if(t->direction == 'S')
		t->direction = 'E';
		else if(t->direction == 'E')
		t->direction = 'N';
	}	
}

void getNewTankPosition(tank * t_new, tank * t, int d_x, int d_y, char left, char right){
	*t_new = *t;
	moveTankInternal(t_new, d_x, d_y, left, right);
}

void getEffectiveTankWindow(tank *t, window * w){
	int lower_x, lower_y, upper_x, upper_y;
	if(t->direction == 'N'){
		lower_x = t->x;
		lower_y = t->y;
		
		upper_x = t->x + t->tank_width;
		upper_y = t->y + t->tank_length + t->cannon_length;
	}
	else if(t->direction == 'E'){
		lower_x = t->x - t->cannon_length;
		lower_y = t->y;
		
		upper_x = t->x + t->tank_length;
		upper_y = t->y + t->tank_width;
	}
	else if(t->direction == 'S'){
		lower_x = t->x;
		lower_y = t->y - t->cannon_length;
		
		upper_x = t->x + t->tank_width;
		upper_y = t->y + t->tank_length;
	}
	else{
		lower_x = t->x;
		lower_y = t->y;
		
		upper_x = t->x + t->tank_length + t->cannon_length;
		upper_y = t->y + t->tank_width;
	}
	initWindow(w, lower_x, lower_y, upper_x, upper_y);
}

int windowInBounds(const window * w){
	if(w->l_x <SCREEN_X_MIN || w->u_x > SCREEN_X_MAX|| w->l_y < SCREEN_Y_MIN || w->u_y > SCREEN_Y_MAX){
		return 0;
	}
	else{
		return 1;
	}
}

int tankInBounds(tank * t){
	window w;
	getEffectiveTankWindow(t, &w);
	
	return windowInBounds(&w);
}

int moveTankIfValid(tank * t, int d_x, int d_y, char left, char right){
	tank t_new;
	getNewTankPosition(&t_new, t, d_x, d_y, left, right);
	if(!tankInBounds(&t_new)){
		return 0;
	}
	clearTank(t);
	*t = t_new;
	return 1;
}

int moveTank(tank * t, int d_x, int d_y){
	return moveTankIfValid(t, d_x, d_y, 0, 0);
}

int rotateTankRight(tank * t){
	return moveTankIfValid(t, 0, 0, 0, 1);
}

int rotateTankLeft(tank * t){
	return moveTankIfValid(t, 0, 0, 1, 0);
}

void initTank(tank * t, int x, int y, char direction){
	t->x = x;
	t->y = y;
	t->tank_length = MACRO_TANK_LENGTH;
	t->tank_width = MACRO_TANK_WIDTH;
	t->cannon_length = MACRO_CANNON_LENGTH;
	t->cannon_width = MACRO_CANNON_WIDTH;
	t->direction = direction;
}

void printTank(tank * t){
	colorTank(t, 0);
}

void clearTank(tank * t){
	colorTank(t, 0xffff);
}

void colorTank(tank * tp, unsigned int color){
	tank t = *tp;
	unsigned int cannon_offset =  ( t.tank_width - t.cannon_width )/2;
	
	if(t.direction == 'N'){
		//fillRect(t.x, t.y, t.tank_width, t.tank_length, color);
		//fillRect(t.x + cannon_offset , t.y + t.tank_length, t.cannon_width, t.cannon_length, color);
		drawRect(t.x, t.y, t.tank_width, t.tank_length, color);
		drawRect(t.x + cannon_offset , t.y + t.tank_length, t.cannon_width, t.cannon_length, color);
	} else if(t.direction == 'E') {
		//fillRect(t.x, t.y, t.tank_length, t.tank_width, color);
		//fillRect(t.x - t.cannon_length, t.y + cannon_offset, t.cannon_length, t.cannon_width, color);
		drawRect(t.x, t.y, t.tank_length, t.tank_width, color);
		drawRect(t.x - t.cannon_length, t.y + cannon_offset, t.cannon_length, t.cannon_width, color);
	} else if(t.direction == 'S') {
		//fillRect(t.x, t.y, t.tank_width, t.tank_length, color);
		//fillRect(t.x + cannon_offset , t.y - t.cannon_length, t.cannon_width, t.cannon_length, color);
		drawRect(t.x, t.y, t.tank_width, t.tank_length, color);
		drawRect(t.x + cannon_offset , t.y - t.cannon_length, t.cannon_width, t.cannon_length, color);
	}else if(t.direction == 'W') {
		//fillRect(t.x, t.y, t.tank_length, t.tank_width, color);
		//fillRect(t.x + t.tank_length, t.y + cannon_offset, t.cannon_length, t.cannon_width, color);
		drawRect(t.x, t.y, t.tank_length, t.tank_width, color);
		drawRect(t.x + t.tank_length, t.y + cannon_offset, t.cannon_length, t.cannon_width, color);
	}
}

int shotInBounds(shot *s){
	int lower_x = s->x, lower_y = s->y, upper_x = s->x + s->shot_width, upper_y = s->y + s->shot_height;
	window w;
	initWindow(&w, lower_x, lower_y, upper_x, upper_y);
	
	return windowInBounds(&w);
}

int moveSingleShot(shot * s){
	// Code to move shot here
	s->y += s->speed;
	
	// Bound Checking
	if(!shotInBounds(s))
		return 0;
		
	// Hit Checking
	return 1;
}

void colorShot(const shot * s, unsigned int color){
	drawRect(s->x, s->y, s->shot_width, s->shot_height, color);
}

void clearShot(shot * s){
	colorShot(s, 0xffff);
}

void printShot(shot * s){
	colorShot(s, 0);
}

void deleteShot(shot * s){
	free(s);
}

void moveAllShots(shot * shots_arr[]){
	for (int i=0; i<MAX_CONCURRENT_SHOTS; ++i){
		if(shots_arr[i]!=0){
			clearShot(shots_arr[i]);
			if(moveSingleShot(shots_arr[i]))
				printShot(shots_arr[i]);
			else{
				deleteShot(shots_arr[i]);
				shots_arr[i] = NULL;
			}
		}
	}
}

void makeShot(tank * t, shot * shots_arr[]){
	shot * s = createShot(t);
	for (int i=0; i<MAX_CONCURRENT_SHOTS; ++i){
		if(shots_arr[i] == NULL){
			shots_arr[i] = s;
			return;
		}
	}
	deleteShot(s);
}

void getCannonHead(const tank * t, int * cannon_x, int * cannon_y){
	
	unsigned int cannon_offset =  ( t->tank_width - t->cannon_width )/2;
	
	if(t->direction == 'N'){
		*cannon_x = t->x + cannon_offset;
		*cannon_y = t->y + t->tank_length + t->cannon_length;
	} else{
		*cannon_x=200;
		*cannon_y=100;
	}
	
	 /*else if (t->direction == 'E'){
		
	} else if (t->direction == 'S'){
		
	} else if (t->direction == 'W'){
		
	}*/
}
#endif