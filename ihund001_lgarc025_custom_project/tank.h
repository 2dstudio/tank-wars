#ifndef TANK_H
#define TANK_H

#define TANK_LENGTH 35
#define TANK_WIDTH 35
#define CANNON_WIDTH 7
#define CANNON_LENGTH 15

typedef struct _tank {
	unsigned long x;
	unsigned long y;
	unsigned int tank_length;
	unsigned int tank_width;
	unsigned int cannon_length;
	unsigned int cannon_width;
	char direction;
} tank;

void colorTank(tank t, unsigned int color);
void moveTankAndDisplay(tank * t, int d_x, int d_y);

int tankMoved(tank * t_old, tank * t){
	if((t_old->x != t->x) || (t_old->y != t->y)){
		return 1;
	}
	
	if(t_old->direction != t->direction){
		return 1;
	}
	
	return 0;
}

void moveTankInternal(tank * t, int d_x, int d_y){
	t->x += d_x;
	t->y += d_y;
}

void rotateTankRight(tank * t){
	if(t->direction == 'N')
		t->direction = 'E';
	else if (t->direction == 'E')
		t->direction = 'S';
	else if(t->direction == 'S')
		t->direction = 'W';
	else if(t->direction == 'W')
		t->direction = 'N';
}

void rotateTankLeft(tank * t){
	if(t->direction == 'N')
		t->direction = 'W';
	else if (t->direction == 'W')
		t->direction = 'S';
	else if(t->direction == 'S')
		t->direction = 'E';
	else if(t->direction == 'E')
		t->direction = 'N';
}

void moveTank(tank * t, int d_x, int d_y){
	//FIXME - Here check first if movement is valid.
	moveTankInternal(t, d_x, d_y);
}

void moveTankAndDisplay(tank * t, int d_x, int d_y){
	//FIXME - Here check first if movement is valid.
	moveTankInternal(t, d_x, d_y);	
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