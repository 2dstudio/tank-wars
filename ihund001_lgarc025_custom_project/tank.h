#ifndef TANK_H
#define TANK_H

#define TANK_LENGTH 35
#define TANK_WIDTH 35
#define CANNON_WIDTH 7
#define CANNON_LENGTH 15

#define SCREEN_X_MAX 300
#define SCREEN_X_MIN 0
#define SCREEN_Y_MAX 300
#define SCREEN_Y_MIN 0

typedef struct _tank {
	int x;
	int y;
	unsigned int tank_length;
	unsigned int tank_width;
	unsigned int cannon_length;
	unsigned int cannon_width;
	char direction;
} tank;

void colorTank(tank t, unsigned int color);
int inBounds(tank *t);

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

int inBounds(tank * t){
	int lower_x = 0;
	int lower_y = 0; 
	int upper_x = 0;
	int upper_y = 0;
	
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
	else if(t->direction == 'W'){
		lower_x = t->x;
		lower_y = t->y;
		
		upper_x = t->x + t->tank_length + t->cannon_length;
		upper_y = t->y + t->tank_width;
	}
	
	if(lower_x <SCREEN_X_MIN || upper_x > SCREEN_X_MAX|| lower_y < SCREEN_Y_MIN || upper_y > SCREEN_Y_MAX){
		return 0;
	}
	else{
		return 1;
	}
}

void moveTankIfValid(tank * t, int d_x, int d_y, char left, char right){
	tank t_new;
	getNewTankPosition(&t_new, t, d_x, d_y, left, right);
	if(inBounds(&t_new)){
		*t = t_new;
	}
}

void moveTank(tank * t, int d_x, int d_y){
	moveTankIfValid(t, d_x, d_y, 0, 0);
}

void rotateTankRight(tank * t){
	moveTankIfValid(t, 0, 0, 0, 1);
}

void rotateTankLeft(tank * t){
	moveTankIfValid(t, 0, 0, 1, 0);
}

void initTank(tank * t, int x, int y, char direction){
	t->x = x;
	t->y = y;
	t->tank_length = TANK_LENGTH;
	t->tank_width = TANK_WIDTH;
	t->cannon_length = CANNON_LENGTH;
	t->cannon_width = CANNON_WIDTH;
	t->direction = direction;
}

void printTank(tank t){
	colorTank(t, 0);
}

void clearTank(tank t){
	colorTank(t, 0xffff);
}

void colorTank(tank t, unsigned int color){
	
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
#endif