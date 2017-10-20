#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#define LED_CMD 0xED

/**
 * @brief Subscribes and enables keyboard exclusive interrupts
 *
 * @return Returns bit order in interrupt mask; negative value on failure
 */
int keyboard_subscribe_int(void);

/**
 * @brief Unsubscribes keyboard interrupts
 *
 * @return Return 0 upon success and non-zero otherwise
 */
int keyboard_unsubscribe_int();

/**
 * @brief Keyboard interrupt handler
 *
 * return break/make code we want to print
 */
unsigned long keyboard_c_handler();


/**
 * @brief Writes a command to kbd
 *
 * @param command: command we want to send
 *
 * @return Return 0 upon success and non-zero otherwise
 */
int kbd_write(unsigned long command);


/**
 * @brief Reads from outbuff
 *
 * @param *data: data to receive placed here
 *
 * @return Return 0 upon success and non-zero otherwise
 */
int kbd_read(unsigned long *data);



/**
 * @brief Toggle on or off the led given in the argument
 *
 */

void kb_set_led(unsigned short led);




#endif /* __KEYBOARD_H */
