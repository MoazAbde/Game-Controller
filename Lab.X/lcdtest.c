/* 
 * File:   lcdtest.c
 * Author: Moaz
 *
 * Created on November 28, 2022, 2:59 PM
 */

#include <avr/io.h>
#include "defines.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include "hd44780.h"
#include "lcd.h"


/*
 * 
 */
// initialization of LCD 
//static void
//ioinit(void)
//{
// 
//  lcd_init();
//}

FILE lcd_str = FDEV_SETUP_STREAM(lcd_putchar, NULL, _FDEV_SETUP_WRITE); 

int main(int argc, char** argv) {

    lcd_init();// initialize LCD
    stderr = &lcd_str;
    fprintf(stderr, "  Moaz \x1b\xc0    Start");
    
    while(1){
        
        fprintf(stderr, "Moaz \x1b\xc0 %d", 420);
        
        _delay_ms(200);
        fprintf(stderr, "\x1b\x01");
    }
    return (EXIT_SUCCESS);
}

