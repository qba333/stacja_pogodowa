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
volatile uint8_t sekundy;	/* licznik sekund 0-59 */

uint8_t subzero, cel, cel_fract_bits;


int main(void)
{
	DDRC |= (1<<LED1_PIN);


	/* ustawienie TIMER0 dla F_CPU=16MHz */
	TCCR0A |= (1<<WGM01);				/* tryb CTC */
	TCCR0B |= (1<<CS02)|(1<<CS00);		/* preskaler = 1024 */
	OCR0A = 155;						/* dodatkowy podział przez 156 (rej. przepełnienia) */
	TIMSK0 |= (1<<OCIE0A);				/* zezwolenie na przerwanie CompareMatch */
	/* przerwanie wykonywane z częstotliwością ok 10ms (100 razy na sekundę) */


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


	/* dokonujemy odczytu temperatury z pierwszego czujnika o ile został wykryty */
	/* wyświetlamy temperaturę gdy czujnik wykryty */
	if( DS18X20_OK == DS18X20_read_meas(gSensorIDs[0], &subzero, &cel, &cel_fract_bits) ) display_temp(3,0);
	else {
		lcd_gotoxy(3,0);
		lcd_puts(" error ");	/* wyświetlamy informację o błędzie jeśli np brak czujnika lub błąd odczytu */
	}

	/* dokonujemy odczytu temperatury z pierwszego czujnika o ile został wykryty */
	if( DS18X20_OK == DS18X20_read_meas(gSensorIDs[1], &subzero, &cel, &cel_fract_bits) ) display_temp(3,1);
	else {
		lcd_gotoxy(3,1);
		lcd_puts(" error ");
	}

	sei();	/* włączamy globalne przerwania */



	lcd_gotoxy(0,0);
	lcd_puts_p(PSTR("T1:")); /* wyświetl napis na LCD */
	lcd_gotoxy(0,1);
	lcd_puts_p(PSTR("T2:"));


    while(1)
        {

			if(s1_flag) {	/* sprawdzanie flagi tyknięć timera programowego co 1 sekundę */

				/* co trzy sekundy gdy reszta z dzielenia modulo 3 == 0 sprawdzaj ilość dostępnych czujników */
				if( 0 == (sekundy%3) ) {

					uint8_t *cl=(uint8_t*)gSensorIDs;	// pobieramy wskaźnik do tablicy adresów czujników
					for( uint8_t i=0; i<MAXSENSORS*OW_ROMCODE_SIZE; i++) *cl++ = 0; // kasujemy całą tablicę
					czujniki_cnt = search_sensors();	// ponownie wykrywamy ile jest czujników i zapełniamy tablicę
					lcd_gotoxy(15,0);
					lcd_puti( czujniki_cnt );	// wyświetlamy ilość czujników na magistrali
				}

				/* co trzy sekundy gdy reszta z dzielenia modulo 3 == 1 wysyłaj rozkaz pomiaru do czujników */
				if( 1 == (sekundy%3) ) {
					DS18X20_start_meas( DS18X20_POWER_EXTERN, NULL );
				}

				/* co trzy sekundy gdy reszta z dzielenia modulo 3 == 2 czyli jedną sekundę po rozkazie konwersji
				 *  dokonuj odczytu i wyświetlania temperatur z 2 czujników jeśli są podłączone, jeśli nie
				 *  to pokaż komunikat o błędzie */
				if( 2 == (sekundy%3) ) {
					if( DS18X20_OK == DS18X20_read_meas(gSensorIDs[0], &subzero, &cel, &cel_fract_bits) ) display_temp(3,0);
					else {
						lcd_gotoxy(3,0);
						lcd_puts(" error ");
					}

					if( DS18X20_OK == DS18X20_read_meas(gSensorIDs[1], &subzero, &cel, &cel_fract_bits) ) display_temp(3,1);
					else {
						lcd_gotoxy(3,1);
						lcd_puts(" error ");
					}
				}

				/* co sekundę zmiana stanu diody */
				if( 0 == (sekundy%1) ) {
					PORTC ^= (1<<LED1_PIN);
				}

				/* zerujemy flagę aby tylko jeden raz w ciągu sekundy wykonać operacje */
				s1_flag=0;
			} /* koniec sprawdzania flagi */

        }
    return 0;
}


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

/* ================= PROCEDURA OBSŁUGI PRZERWANIA - COMPARE MATCH */
/* pełni funkcję timera programowego wyznaczającego podstawę czasu = 1s */
ISR(TIMER0_COMPA_vect)
{
	static uint8_t cnt=0;	/* statyczna zmienna cnt do odliczania setnych ms */

	if(++cnt>99) {	/* gdy licznik ms > 99 (minęła 1 sekunda) */
		s1_flag=1;	/* ustaw flagę tyknięcia sekundy */
		sekundy++;	/* zwiększ licznik sekund */
		if(sekundy>59) sekundy=0; /* jeśli iloś sekund > 59 - wyzeruj */
		cnt=0;	/* wyzeru licznik setnych ms */
	}
}
