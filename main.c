/*
 * main.c
 *
 *  Created on: 21 sie 2021
 *
 *      Author: Jakub Wróblewski
 *
 *      MCU: ATmega328P
 *
 */
#define LED1_PIN 0


#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "lcdpcf8574/lcdpcf8574.h"
#include "1wire/ds18x20.h"


void display_temp(uint8_t x, uint8_t y);

uint8_t czujniki_cnt;		/* ilość czujników temperatury na magistrali 1-Wire */
volatile uint8_t s1_flag;	/* flaga tyknięcia timera co 1 sekundę */
volatile uint8_t ms100_flag;	/* flaga tyknięcia timera co 100 ms */
volatile uint8_t k1_flag;	/* flaga naciśnięcia przycisku */
volatile uint8_t sekundy;	/* licznik sekund 0-59 */
volatile uint8_t sto_ms;	/* licznik 100ms 0-9 */
volatile uint8_t screen;	/* numer wyświetlanego ekranu zmienianego przyciiskiem */

uint8_t subzero, cel, cel_fract_bits;	/* zmienne czujników temperatury */


int main(void)
{
	DDRC |= (1<<LED1_PIN);	// ustawienie pinu diody led jako wyjście
	PORTD |= (1<<PD7);		// podciągnięcie wejścia do Vcc

	/* ustawienie TIMER0 dla F_CPU=16MHz */
	TCCR0A |= (1<<WGM01);				/* tryb CTC */
	TCCR0B |= (1<<CS02)|(1<<CS00);		/* preskaler = 1024 */
	OCR0A = 155;						/* dodatkowy podział przez 156 (rej. przepełnienia) */
	TIMSK0 |= (1<<OCIE0A);				/* zezwolenie na przerwanie CompareMatch */
	/* przerwanie wykonywane z częstotliwością ok 10ms (100 razy na sekundę) */

	/* ustawienie przerwania PCINT na pinie PD7 */
	PCICR |= (1<<PCINT2);	// zezwolenie na przewrania PCINT2 (na porcie D)
	PCMSK2 |= (1<<PCINT23);	// PCINT23 = PD7

	uint8_t i = 0;		/* licznik */

    lcd_init(LCD_DISP_ON);	// inicjalizacja LCD
	lcd_led(0);				// włączenie podświetlenia LCD


	/* sprawdzamy ile czujników DS18B20 widocznych jest na magistrali */
	czujniki_cnt = search_sensors();

	/* wysyłamy rozkaz wykonania pomiaru temperatury
	 * do wszystkich czujników na magistrali 1Wire
	 * zakładając, że zasilane są w trybie NORMAL,
	 * gdyby był to tryb Parasite, należałoby użyć
	 * jako pierwszego prarametru DS18X20_POWER_PARASITE */
	DS18X20_start_meas( DS18X20_POWER_EXTERN, NULL );

	/* czekamy 750ms na dokonanie konwersji przez podłączone czujniki */
	_delay_ms(750);



	sei();	/* włączamy globalne przerwania */

    while(1)
        {

    		if(k1_flag) { 	/* sprawdzenie flagi naciśnięcia przycisku - eliminacja drgań styków */

    			if( ms100_flag ) i++;
    			if( i>=2 ) {
    				i = 0;
    				k1_flag = 0;
    			}
    		}


    		if(ms100_flag) { 	/* sprawdzanie flagi tyknięć timera programowego co 100 milisekund */

    			/* wyświetlanie temperatury wewnątrz - ekran 1 */
				if( ( 0 == (sto_ms%2) ) && ( screen == 0 ) ) {
					if( DS18X20_OK == DS18X20_read_meas(gSensorIDs[0], &subzero, &cel, &cel_fract_bits) ) {
						lcd_gotoxy(1,0);
						lcd_puts_p(PSTR("  Temperature   ")); /* wyświetl napis na LCD */
						lcd_gotoxy(4,1);
						lcd_puts_p(PSTR(" in:"));
						display_temp(8,1);
					}
					else {
						lcd_gotoxy(8,1);
						lcd_puts(" error ");
					}
				}

    			/* wyświetlanie temperatury na zewnątrz - ekran 2 */
				if( ( 0 == (sto_ms%2) ) && ( screen == 1 ) ) {
					if( DS18X20_OK == DS18X20_read_meas(gSensorIDs[1], &subzero, &cel, &cel_fract_bits) ) {
						lcd_gotoxy(1,0);
						lcd_puts_p(PSTR("  Temperature   ")); /* wyświetl napis na LCD */
						lcd_gotoxy(4,1);
						lcd_puts_p(PSTR("out:"));
						display_temp(8,1);
					}
					else {
						lcd_gotoxy(8,1);
						lcd_puts(" error ");
					}
				}

    			/* wyświetlanie wilgotności powietrza - ekran 3 */
				if( ( 0 == (sto_ms%2) ) && ( screen == 2 ) ) {
					lcd_gotoxy(1,0);
					lcd_puts_p(PSTR("  Humidity    ")); /* wyświetl napis na LCD */
					lcd_gotoxy(0,1);
					lcd_puts_p(PSTR("                "));

				}


				/* co sekundę mrygnięcie diodą */
				if( 0 == (sto_ms%10) ) {
					PORTC |= (1<<LED1_PIN);
				}
				if( 1 == (sto_ms%10) ) {
					PORTC &= !(1<<LED1_PIN);
				}

				lcd_gotoxy(0,0);
				lcd_puti(screen+1);		// wyświetlanie numeru ekranu

				/* zerowanie flagi aby tylko jeden raz w ciągu 100ms wykonać operacje */
				ms100_flag=0;

    		}

			if(s1_flag) {	/* sprawdzanie flagi tyknięć timera programowego co 1 sekundę */


				/* co dwie sekundy gdy reszta z dzielenia modulo 2 == 0 wysyłaj rozkaz pomiaru do czujników */
				if( 0 == (sekundy%2) ) {
					DS18X20_start_meas( DS18X20_POWER_EXTERN, NULL );
				}

				/* zerujemy flagę aby tylko jeden raz w ciągu sekundy wykonać operacje */
				s1_flag=0;
			} /* koniec sprawdzania flagi */

        }
    return 0;
}


/* ================= PROCEDURA OBSŁUGI PRZERWANIA - COMPARE MATCH */
/* pełni funkcję timera programowego wyznaczającego podstawę czasu = 1s */
ISR( TIMER0_COMPA_vect )
{
	static uint8_t cnt=0;	/* statyczna zmienna cnt do odliczania setnych ms */

	if(++cnt>9) {	/* gdy licznik ms > 99 (minęła 1 sekunda) */
		ms100_flag=1;	/* ustaw flagę tyknięcia 100 ms */
		sto_ms++; /* zwiększ licznik 100ms */
		if(sto_ms>9) {
			sto_ms=0;
			s1_flag=1;	/* ustaw flagę tyknięcia sekundy */
			sekundy++;	/* zwiększ licznik sekund */
		}
		if(sekundy>59) sekundy=0; /* jeśli iloś sekund > 59 - wyzeruj */
		cnt=0;	/* wyzeru licznik setnych ms */
	}
}

/* przerwanie wywoływane za pomocą przycisku */
ISR( PCINT2_vect )
{
	if( !( PIND & (1<<PD7) ) ) {
		if (k1_flag == 0) {
			screen++;
			k1_flag = 1;
		}

		if( screen > 2 ) screen=0;
	}
}

/* ---------- definicje funkcji ---------- */
void display_temp(uint8_t x, uint8_t y)
{
	lcd_gotoxy(x,y);
	if(subzero) lcd_puts("-");	/* jeśli subzero==1 wyświetla znak minus (temp. ujemna) */
	else lcd_puts(" ");	/* jeśli subzero==0 wyświetl spację zamiast znaku minus (temp. dodatnia) */
	lcd_puti(cel);	/* wyświetl dziesiętne częci temperatury  */
	lcd_puts(".");	/* wyświetl kropkę */
	lcd_puti(cel_fract_bits); /* wyświetl dziesiętne częci stopnia */
	lcd_puts(" C "); /* wyświetl znak jednostek (C - stopnie Celsiusza) */
}
