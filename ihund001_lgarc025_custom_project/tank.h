#ifndef TANK_H
#define TANK_H

#define MACRO_TANK_LENGTH 40
#define MACRO_TANK_WIDTH 40
#define MACRO_CANNON_WIDTH 10
#define MACRO_CANNON_LENGTH 25
#define MACRO_TANK_MOVE_RATE 5

#define MACRO_BULLET_SPEED 9
#define MACRO_BULLET_HEIGHT 10
#define MACRO_BULLET_WIDTH 10

#define SCREEN_X_MAX 320
#define SCREEN_X_MIN 0
#define SCREEN_Y_MAX 480
#define SCREEN_Y_MIN 0

#define POWERUP_HEIGHT 20
#define POWERUP_WIDTH 20

#define MAX_CONCURRENT_SHOTS 4
#define MAX_CONCURRENT_POWERUPS 1

#define INITIAL_HEALTH 5
#define INITIAL_BULLET_COUNT 5 

#define NORMAL_COLOR 0x0000
#define HIT_COLOR 0xF800

typedef struct _tank {
	int x;
	int y;
	unsigned int tank_length;
	unsigned int tank_width;
	unsigned int cannon_length;
	unsigned int cannon_width;
	char tank_direction;
	int tank_speed;
	char health;
	int bullet_speed;
	int bullet_height;
	int bullet_width;
	int bullet_count;
	int hit;
	int flash;
	int refresh;
	int color;
} tank;

typedef struct _bullet {
	int x;
	int y;
	int bullet_height;
	int bullet_width;
	unsigned int bullet_speed;
	char bullet_direction;
} bullet;

typedef struct _window {
	int l_x;
	int l_y;
	int u_x;
	int u_y;
} window;

typedef struct _powerup {
	int x;
	int y;
	char type;
	int age;
} powerup;

void colorTank(const tank * t, unsigned int color);
int tankInBounds(const tank *t);
void getCannonHead(const tank * t, int * cannon_x, int * cannon_y);
void clearTank(const tank * t);
void printTank(const tank * t);

void initPowerUp(powerup * power, int x, int y, char type){
	power->x = x;
	power->y = y;
	power->type = type;
	power->age = 0;
}

void initWindow(window * w, int l_x, int l_y, int u_x, int u_y){
	w->l_x = l_x;
	w->l_y = l_y;
	w->u_x = u_x;
	w->u_y = u_y;
}

void initTank(tank * t, int x, int y, char direction){
	t->x = x;
	t->y = y;
	t->tank_length = MACRO_TANK_LENGTH;
	t->tank_width = MACRO_TANK_WIDTH;
	t->cannon_length = MACRO_CANNON_LENGTH;
	t->cannon_width = MACRO_CANNON_WIDTH;
	t->bullet_speed = MACRO_BULLET_SPEED;
	t->bullet_height = MACRO_BULLET_HEIGHT;
	t->bullet_width = MACRO_BULLET_WIDTH;
	t->health = INITIAL_HEALTH;
	t->tank_speed = MACRO_TANK_MOVE_RATE;
	t->bullet_count = INITIAL_BULLET_COUNT;
	t->tank_direction = direction;
	
	t->flash = 0;
	t->hit = 0;
	t->refresh = 0;
	t->color = NORMAL_COLOR;
}

void initShot(const tank * t,bullet * s){
	getCannonHead(t, &s->x, &s->y);
	s->bullet_height = t->bullet_height;
	s->bullet_width = t->bullet_width;
	s->bullet_speed = t->bullet_speed;
	s->bullet_direction = t->tank_direction;
}

bullet * createShot(const tank * t){
	bullet * s = malloc(sizeof(bullet));
	initShot(t, s);
	return s;
}

int tankMoved(tank * t_old, tank * t){
	if((t_old->x != t->x) || (t_old->y != t->y)){
		return 1;
	}
	
	if(t_old->tank_direction != t->tank_direction){
		return 1;
	}
	
	return 0;
}

void moveTankInternal(tank * t, int d_x, int d_y, char left, char right){
	t->x += d_x;
	t->y += d_y;
	
	if(right){
		if(t->tank_direction == 'N')
		t->tank_direction = 'E';
		else if (t->tank_direction == 'E')
		t->tank_direction = 'S';
		else if(t->tank_direction == 'S')
		t->tank_direction = 'W';
		else if(t->tank_direction == 'W')
		t->tank_direction = 'N';
	}
	
	if(left){
		if(t->tank_direction == 'N')
		t->tank_direction = 'W';
		else if (t->tank_direction == 'W')
		t->tank_direction = 'S';
		else if(t->tank_direction == 'S')
		t->tank_direction = 'E';
		else if(t->tank_direction == 'E')
		t->tank_direction = 'N';
	}	
}

void getNewTankPosition(tank * t_new, const tank * t, int d_x, int d_y, char left, char right){
	*t_new = *t;
	moveTankInternal(t_new, d_x, d_y, left, right);
}

void getEffectiveTankWindow(const tank *t, window * w){
	int lower_x, lower_y, upper_x, upper_y;
	if(t->tank_direction == 'N'){
		lower_x = t->x;
		lower_y = t->y;
		
		upper_x = t->x + t->tank_width;
		upper_y = t->y + t->tank_length + t->cannon_length;
	}
	else if(t->tank_direction == 'E'){
		lower_x = t->x - t->cannon_length;
		lower_y = t->y;
		
		upper_x = t->x + t->tank_length;
		upper_y = t->y + t->tank_width;
	}
	else if(t->tank_direction == 'S'){
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

void getTankBodyWindow(const tank *t, window * body_window){
	int lower_x, lower_y, upper_x, upper_y;
	
	lower_x = t->x;
	lower_y = t->y;
	if(t->tank_direction == 'N' || t->tank_direction == 'S'){
		upper_x = t->x + t->tank_width;
		upper_y = t->y + t->tank_length;
	}
	else{
		upper_x = t->x + t->tank_length;
		upper_y = t->y + t->tank_width;
	}
	initWindow(body_window, lower_x, lower_y, upper_x, upper_y);
}

void getTankCannonWindow(const tank *t, window * cannon_window){
	int lower_x, lower_y, upper_x, upper_y;
	
	unsigned int cannon_offset =  ( t->tank_width - t->cannon_width )/2;
	
	if(t->tank_direction == 'N'){
		lower_x = t->x + cannon_offset;
		lower_y = t->y +t->tank_length;
		
		upper_x = t->x + cannon_offset + t->cannon_width;
		upper_y = t->y + t->tank_length + t->cannon_length;
	}
	else if(t->tank_direction == 'E'){
		lower_x = t->x - t->cannon_length;
		lower_y = t->y + cannon_offset;
		
		upper_x = t->x;
		upper_y = t->y + cannon_offset + t->cannon_width;
	}
	else if(t->tank_direction == 'S'){
		lower_x = t->x + cannon_offset;
		lower_y = t->y - t->cannon_length;
		
		upper_x = t->x + cannon_offset + t->cannon_width;
		upper_y = t->y;
	}
	else{
		lower_x = t->x + t->tank_length;
		lower_y = t->y + cannon_offset;
		
		upper_x = t->x + t->tank_length + t->cannon_length;
		upper_y = t->y + cannon_offset + t->cannon_width;
	}
	initWindow(cannon_window, lower_x, lower_y, upper_x, upper_y);
}

void getBulletWindow(const bullet *s, window * bullet_window){
	int lower_x, lower_y, upper_x, upper_y;
	
	lower_x = s->x;
	lower_y = s->y;
	upper_x = s->x + s->bullet_width;
	upper_y = s->y + s->bullet_height;
	initWindow(bullet_window, lower_x, lower_y, upper_x, upper_y);
}

int windowInBounds(const window * w){
	if(w->l_x <SCREEN_X_MIN || w->u_x > SCREEN_X_MAX|| w->l_y < SCREEN_Y_MIN || w->u_y > SCREEN_Y_MAX){
		return 0;
	}
	else{
		return 1;
	}
}

int tankInBounds(const tank * t){
	window w;
	getEffectiveTankWindow(t, &w);
	return windowInBounds(&w);
}

int windowsCollide(const window * w1, const window * w2){
	if (w2->u_x <= w1->l_x)
		return 0;
	else if (w2->l_x >= w1->u_x)
		return 0;
	else{
		if(w2->u_y <= w1->l_y)
			return 0;
		else if(w2->l_y >= w1->u_y)
			return 0;
		else
			return 1;
	}
}

int tanksCollide(const tank *t1,const tank *t2){
	window t1_body_window, t1_cannon_window, t2_body_window, t2_cannon_window;
	getTankBodyWindow(t1, &t1_body_window);
	getTankCannonWindow(t1, &t1_cannon_window);
	getTankBodyWindow(t2, &t2_body_window);
	getTankCannonWindow(t2, &t2_cannon_window);
	
	//Check if any combinations of windows collide
	if(windowsCollide(&t1_body_window,&t2_body_window))
		return 1;
	if(windowsCollide(&t1_body_window,&t2_cannon_window))
		return 1;
	if(windowsCollide(&t1_cannon_window,&t2_body_window))
		return 1;
	if(windowsCollide(&t1_cannon_window,&t2_cannon_window))
		return 1;
	return 0;
}

int moveTankIfValid(tank * t1, const tank *t2, int d_x, int d_y, char left, char right){
	tank t_new;
	getNewTankPosition(&t_new, t1, d_x, d_y, left, right);
	if(!tankInBounds(&t_new)){
		return 0;
	}
	if(tanksCollide(&t_new, t2)){
		return 0;
	}
	clearTank(t1);
	*t1 = t_new;
	return 1;
}

int moveTank(tank * t1, const tank * t2, int d_x, int d_y){
	return moveTankIfValid(t1, t2, d_x, d_y, 0, 0);
}

int rotateTankRight(tank * t1, const tank * t2){
	return moveTankIfValid(t1, t2, 0, 0, 0, 1);
}

int rotateTankLeft(tank * t1, const tank *t2){
	return moveTankIfValid(t1, t2, 0, 0, 1, 0);
}

void printTank(const tank * t){
	colorTank(t, t->color);
}

void clearTank(const tank * t){
	colorTank(t, 0xffff);
}

void colorTank(const tank * tp, unsigned int color){
	tank t = *tp;
	unsigned int cannon_offset =  ( t.tank_width - t.cannon_width )/2;
	
	if(t.tank_direction == 'N'){
		//fillRect(t.x, t.y, t.tank_width, t.tank_length, color);
		//fillRect(t.x + cannon_offset , t.y + t.tank_length, t.cannon_width, t.cannon_length, color);
		drawRect(t.x, t.y, t.tank_width, t.tank_length, color);
		drawRect(t.x + cannon_offset , t.y + t.tank_length, t.cannon_width, t.cannon_length, color);
	} else if(t.tank_direction == 'E') {
		//fillRect(t.x, t.y, t.tank_length, t.tank_width, color);
		//fillRect(t.x - t.cannon_length, t.y + cannon_offset, t.cannon_length, t.cannon_width, color);
		drawRect(t.x, t.y, t.tank_length, t.tank_width, color);
		drawRect(t.x - t.cannon_length, t.y + cannon_offset, t.cannon_length, t.cannon_width, color);
	} else if(t.tank_direction == 'S') {
		//fillRect(t.x, t.y, t.tank_width, t.tank_length, color);
		//fillRect(t.x + cannon_offset , t.y - t.cannon_length, t.cannon_width, t.cannon_length, color);
		drawRect(t.x, t.y, t.tank_width, t.tank_length, color);
		drawRect(t.x + cannon_offset , t.y - t.cannon_length, t.cannon_width, t.cannon_length, color);
	}else if(t.tank_direction == 'W') {
		//fillRect(t.x, t.y, t.tank_length, t.tank_width, color);
		//fillRect(t.x + t.tank_length, t.y + cannon_offset, t.cannon_length, t.cannon_width, color);
		drawRect(t.x, t.y, t.tank_length, t.tank_width, color);
		drawRect(t.x + t.tank_length, t.y + cannon_offset, t.cannon_length, t.cannon_width, color);
	}
}

int shotInBounds(bullet *s){
	int lower_x = s->x, lower_y = s->y, upper_x = s->x + s->bullet_width, upper_y = s->y + s->bullet_height;
	window w;
	initWindow(&w, lower_x, lower_y, upper_x, upper_y);
	
	return windowInBounds(&w);
}

int shotHitTank(const bullet * s, tank * t1, tank * t2){
	window bullet_window, t1_body_window, t1_cannon_window, t2_body_window, t2_cannon_window;
	
	getBulletWindow(s, &bullet_window);
	
	getTankBodyWindow(t1, &t1_body_window);
	getTankCannonWindow(t1, &t1_cannon_window);
	getTankBodyWindow(t2, &t2_body_window);
	getTankCannonWindow(t2, &t2_cannon_window);
	
	if(windowsCollide(&bullet_window,&t1_body_window) || windowsCollide(&bullet_window,&t1_cannon_window)){
		t1->hit = 1;
		t1->flash = 1;
		return 1;
	}
	if(windowsCollide(&bullet_window,&t2_body_window) || windowsCollide(&bullet_window,&t2_cannon_window)){
		t2->hit = 1;
		t2->flash = 1;
		return 1;
	}
	return 0;
}

int moveSingleShot(bullet * s, tank * t1, tank *t2){
	if(s->bullet_direction == 'N')
		s->y += s->bullet_speed;
	else if(s->bullet_direction == 'E')
		s->x -= s->bullet_speed;
	else if(s->bullet_direction == 'S')
		s->y -= s->bullet_speed;
	else
		s->x += s->bullet_speed;
	
	if(!shotInBounds(s))
		return 0;
	
	if(shotHitTank(s, t1, t2))
		return 0;
		
	return 1;
}

void colorShot(const bullet * s, unsigned int color){
	drawRect(s->x, s->y, s->bullet_width, s->bullet_height, color);
}

void clearShot(bullet * s){
	colorShot(s, 0xffff);
}

void printShot(bullet * s){
	colorShot(s, 0);
}

void deleteShot(bullet * s){
	free(s);
}

void moveAllShots(bullet * shots_arr[], tank * t1, tank *t2){
	for (int i=0; i<MAX_CONCURRENT_SHOTS; ++i){
		if(shots_arr[i] != NULL){
			clearShot(shots_arr[i]);
			if(moveSingleShot(shots_arr[i], t1, t2))
				printShot(shots_arr[i]);
			else{
				deleteShot(shots_arr[i]);
				shots_arr[i] = NULL;
			}
		}
	}
}

void makeShot(tank * t, bullet * shots_arr[]){
	bullet * s = createShot(t);
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
	
	if(t->tank_direction == 'N'){
		*cannon_x = t->x + cannon_offset;
		*cannon_y = t->y + t->tank_length + t->cannon_length;
	} else if (t->tank_direction == 'E'){
		*cannon_x = t->x - t->cannon_length - MACRO_BULLET_HEIGHT;
		*cannon_y = t->y + cannon_offset;
	} else if (t->tank_direction == 'S'){
		*cannon_x = t->x + cannon_offset;
		*cannon_y = t->y - t->cannon_length - MACRO_BULLET_HEIGHT;
	} else {
		*cannon_x = t->x + t->tank_length + t->cannon_length;
		*cannon_y = t->y + cannon_offset;
	}
}

void colorPowerUp(const powerup *p, unsigned int color){
	fillRect(p->x, p->y, POWERUP_WIDTH, POWERUP_HEIGHT, color);
}

void clearPowerUp(const powerup* p){
	colorPowerUp(p, 0xffff);
}

void printPowerUp(const powerup* p){
	if(p->type == 'H')
		colorPowerUp(p, 0x07e0);
	else if(p->type == 'B')
		colorPowerUp(p, 0xF800);
	else if(p->type == 'I')
		colorPowerUp(p, 0xFFE0);
}

#endif