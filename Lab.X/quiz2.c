/* 
 * File:   quiz2.c
 * Author: Moaz
 *
 * Created on November 28, 2022, 10:00 PM
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
#include <stdint.h>


// NOTE I AM USING AN EXTERNAL OSCILATOR AS A CLOCK SOURCE(14745600 HZ, THE ONE IN THE LAB KIT)
// initialization of LCD 
static void
ioinit(void)
{
 
  lcd_init();
}

FILE lcd_str = FDEV_SETUP_STREAM(lcd_putchar, NULL, _FDEV_SETUP_WRITE);  

// function Prototype 
int adc_conversion(unsigned char );

int main(int argc, char** argv) {
    
    ioinit(); // initialize LCD
    stderr = &lcd_str;
    //ADC Initialization
    DIDR0 = (1<<ADC1D) | (1 <<ADC0D); //disable digital I/p
    ADCSRA = ((1 << ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0) ); //Enable ADC and 128 prescaler clock
    ADMUX |= (1 << REFS0); //refrence voltage
    
    DDRB |= (1<<PB2);
    PORTB &= ~(1<<PB2);
    float decimalDuty;
    int dutyCycle = 0;
    int ocr1bValue;
    
    
    ICR1 = 9216; //TOP value 
    OCR1B = 4608;
    // phase correct PWM
    TCCR1A |= (1<<COM1B1)|(1<<WGM11);
    TCCR1B |= (1<<WGM13)|(1<<CS11);

    while(1){
        
        decimalDuty= ((float)adc_conversion('x')/100);
        dutyCycle = adc_conversion('x');
        ocr1bValue = decimalDuty * 9216;
        
        
        OCR1B = ocr1bValue;
        
        
        fprintf(stderr, " Moaz Abdelmonem\x1b\xc0             %d%%", dutyCycle);
        _delay_ms(200);
        fprintf(stderr, "\x1b\x01"); //Clear LCD
    }
    
    

    return (EXIT_SUCCESS);
}


/**
 * @brief converts Analog voltages of the x  position to digital voltages
 * the values are then adjusted to a perecentage in order to get sent to the UART
 *
 * @param coordinate needed to change the ADMUX 
 * @return int newprecent, percentage of the voltage 
 */
int adc_conversion(unsigned char coordinate)   
{
    int coordinateValue = 0;
    int percent = 0;
    int newPercent = 0;
    ADCSRA |= (1<<ADEN); //ADC Enable
    
    if(coordinate == 'x'){
        //ADC0 SELECTION
        ADMUX &= ~(1<<MUX0);
        ADMUX &= ~(1<<MUX1);
        ADMUX &= ~(1<<MUX2);
        ADMUX &= ~(1<<MUX3);
    }

    
    ADCSRA |= (1 << ADSC); // Start CONVERSION
    
    while (ADCSRA & (1 << ADSC)); // SPIN Wait for conversion to complete
    coordinateValue = ADC; // reading the digital value from the ADC register
    
    //converting x voltage precentage 
    if(coordinate == 'x'){
        
        if((coordinateValue>=514) && (coordinateValue<=1023)){
            coordinateValue-=514;
            percent = (float)coordinateValue/509 *100;   
            newPercent = ((float)percent/4)+25;
        }else if((coordinateValue>=0) && (coordinateValue<514)){
            coordinateValue-=514;
            percent = (float)coordinateValue/514 *100;
            newPercent = (((float)percent+100)/4);
        }
        
    }

    
    return newPercent;
    
}