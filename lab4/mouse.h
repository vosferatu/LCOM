#ifndef __MOUSE_H
#define __MOUSE_H

typedef enum {INIT, DRAW, COMP} state_t;
typedef enum {RDOWN, RUP, MOVE} event_t;

int mouse_subscribe_int();

int mouse_unsubscribe_int();

int mouse_read(unsigned long *data);

int mouse_write(char where_to, unsigned long command);

void mouse_int_handler(unsigned int count, char packets[]);

void mouse_interface(unsigned long command);

void gesture_machine(char packets[], int *distance);

int gesture_state_machine(char packets[], short *distance, event_t event);


#endif /* __MOUSE.H */
