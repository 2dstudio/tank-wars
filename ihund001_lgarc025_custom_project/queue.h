#ifndef QUEUE_H 
#define QUEUE_H

typedef struct _Q4uc {
	unsigned char buf[4];
	unsigned char cnt;
} Q4uc;

void Q4ucInit(Q4uc *Q) {
	(*Q).cnt=0;
}

unsigned char Q4ucFull(Q4uc Q) {
	return (Q.cnt == 4);
}

unsigned char Q4ucEmpty(Q4uc Q) {
	return (Q.cnt == 0);
}

void Q4ucPush(Q4uc *Q,
unsigned char item) {
	if (!Q4ucFull(*Q)) {
		//DisableInterrupts();
		(*Q).buf[(*Q).cnt] = item;
		(*Q).cnt++;
		//EnableInterrupts();
	}
}

unsigned char Q4ucPop(Q4uc *Q)
{
	int i;
	unsigned char item=0;
	if (!Q4ucEmpty(*Q)) {
		//DisableInterrupts();
		item = (*Q).buf[0];
		(*Q).cnt--;
		for (i=0; i<(*Q).cnt; i++) {
			// shift fwd
			(*Q).buf[i]=
			(*Q).buf[i+1];
		}
		//EnableInterrupts();
	}
	return(item);
}

#endif