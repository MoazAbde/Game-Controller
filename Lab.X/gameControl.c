/* 
 * File:   gameControl.c
 * Author: Moaz Abdelmonem
 * ID: 1660622
 * University of Alberta
 * Created on November 20, 2022, 11:49 AM
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
#include "uart.h"


// initialization of LCD 
static void
ioinit(void)
{
 
  lcd_init();
}

FILE lcd_str = FDEV_SETUP_STREAM(lcd_putchar, NULL, _FDEV_SETUP_WRITE);   

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar,uart_getchar,_FDEV_SETUP_RW); //Print and Read stream for UART

// function Prototype 
int adc_conversion(unsigned char );

int main(void)
{
    ioinit(); // initialize LCD
    stderr = &lcd_str;
    
    //ADC Initialization
    DIDR0 = (1<<ADC1D) | (1 <<ADC0D); //disable digital I/p
    ADCSRA = ((1 << ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0) ); //Enable ADC and 128 prescaler clock
    ADMUX |= (1 << REFS0); //refrence voltage
    
    
    //Switch initialization
    DDRB &= ~(1 << PB0);
    PORTB |= (1 << PB0); // enable internal pull up
    
    int x_coordinate, y_coordinate, fire;
    char game_name[17], handle[10], x_sign = '+', y_sign = '+';
    char data[9], ammoScore[7];
    
    
    uart_init(); // initialize the UART
    fprintf(stderr, "\x1b\x0c"); // Clear Cursor
    
    fprintf(stderr, "  Press Button \x1b\xc0   to Start");
    
    while (PINB & (1 << PB0)); // spin wait for user to begin the game
    fprintf(stderr, "\x1b\x01");
    fprintf(stderr, "  Game Starting");
    
    
    fprintf(&uart_str, "Q\n"); //sending a query
     
    //waiting for all characters to be transmitted (character by character)
    for (int i = 0; i <= 16; i++)
    {
        while (!(UCSR0A & (1 << RXC0))); // wait for a char to be recived
        game_name[i] = UDR0;
    }
    
    fprintf(&uart_str, "HKFC\n"); // SEND THE HANDLE
    fscanf(&uart_str, "%s", handle); // Receive server handle
    
    fprintf(stderr, "\x1b\x01");
    fprintf(&lcd_str, "%s\x1b\xc0%s", game_name, handle); // Print out game name and handle
    
    
    
    while(1){
        _delay_ms(100); 
        //start converting
        x_coordinate = adc_conversion('x');
        y_coordinate = adc_conversion('y');
        
        if (PINB & (1 << PB0))
        { 
            fire = 0; // if user has not fired
        }
        else
        {
            fire = 1; // if user has fired
        }
        
        // checking for y positive and negative values
        if(x_coordinate>=0){
            x_sign = '+';
        }else if(x_coordinate<0){
            x_sign = '-';
            x_coordinate = abs(x_coordinate);
        }
        
        // checking for y positive and negative values
        if(y_coordinate>=0){
            y_sign = '+';
        }else if(y_coordinate<0){
            y_sign = '-';
            y_coordinate = abs(y_coordinate);
        }
        
        // Send out the cordinates and there signs to the game
        fprintf(&uart_str, "C%c%03d%c%03d%d\n", x_sign, x_coordinate, y_sign, y_coordinate, fire);
        
        // loop to get the data getting sent back 
        for (int i = 0; i <= 8; i++)
        {
            while (!(UCSR0A & (1 << RXC0))); //spin wait for data to be received
            data[i] = UDR0;
        }
        data[9] = '\0';
        // receive the ammo and score
        ammoScore[0] = data[8];
        ammoScore[1] = data[0];
        ammoScore[2] = data[1];
        
        ammoScore[3] = ' ';
        
        ammoScore[4] = data[4];
        ammoScore[5] = data[5];
        ammoScore[6] = data[6];
        ammoScore[7] = '\0';

        fprintf(stderr, "\x1b\xc0%s  %s", handle, ammoScore); // display the score and ammo  
    }

 return(0);
}
    

/**
 * @brief converts Analog voltages of the x and y position to digital voltages
 * the values are then adjusted to a perecentage in order to get sent to the UART
 *
 * @param coordinate needed to change the ADMUX 
 * @return int precent, percentage of the voltage 
 */
int adc_conversion(unsigned char coordinate)   
{
    int coordinateValue = 0;
    int percent = 0;
    ADCSRA |= (1<<ADEN); //ADC Enable
    
    if(coordinate == 'x'){
        //ADC0 SELECTION
        ADMUX &= ~(1<<MUX0);
        ADMUX &= ~(1<<MUX1);
        ADMUX &= ~(1<<MUX2);
        ADMUX &= ~(1<<MUX3);
    }else if(coordinate == 'y'){
        ADMUX |= (1<<MUX0);
        ADMUX &= ~(1<<MUX1);
        ADMUX &= ~(1<<MUX2);
        ADMUX &= ~(1<<MUX3);
    }
    
    ADCSRA |= (1 << ADSC); // Start CONVERSION
    
    while (ADCSRA & (1 << ADSC)); // SPIN Wait for conversion to complete
    coordinateValue = ADC; // reading the digital value from the ADC register
    
    //converting x and y voltages to precentages 
    if(coordinate == 'x'){
        
        if((coordinateValue>=514) && (coordinateValue<=1023)){
            coordinateValue-=514;
            percent = (float)coordinateValue/509 *100;   
        }else if((coordinateValue>=0) && (coordinateValue<514)){
            coordinateValue-=514;
            percent = (float)coordinateValue/514 *100;
        }
        
    }else if(coordinate == 'y'){
        
        if((coordinateValue>=517) && (coordinateValue<=1023)){
            coordinateValue-=517;
            percent = (float)coordinateValue/506 *100;   
        }else if((coordinateValue>=0) && (coordinateValue<517)){
            coordinateValue-=517;
            percent = (float)coordinateValue/517 *100;
        }
        
    }
    
    return percent;
    
}