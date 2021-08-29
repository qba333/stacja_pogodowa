/*
 * main.c
 *
 *  Created on: 21 sie 2021
 *      Author: Kuba
 */
#define LED1_PIN 0
#define LED2_PIN 1
#define LED3_PIN 2


#include <avr/io.h>
#include <util/delay.h>

//#include "LCD/lcd44780.h"
//#include "lcdvtwi.h"
//#include "twi.h"

#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "lcdpcf8574/lcdpcf8574.h"
#include "1wire/ds18x20.h"

#include "DS1302/rtc.h"

void display_temp(uint8_t x);

uint8_t czujniki_cnt;		/* ilość czujników na magistrali */
volatile uint8_t s1_flag;	/* flaga tyknięcia timera co 1 sekundę */
volatile uint8_t sekundy;	/* licznik sekund 0-59 */

uint8_t subzero, cel, cel_fract_bits;

int main(void)
{
	DDRC |= (1<<LED1_PIN) | (1<<LED2_PIN) | (1<<LED3_PIN);
	PORTC |= (1<<LED1_PIN);


	/* ustawienie TIMER0 dla F_CPU=16MHz */
	TCCR0A |= (1<<WGM01);				/* tryb CTC */
	TCCR0B |= (1<<CS02)|(1<<CS00);		/* preskaler = 1024 */
	OCR0A = 155;							/* dodatkowy podział przez 78 (rej. przepełnienia) */
	TIMSK0 |= (1<<OCIE0A);				/* zezwolenie na przerwanie CompareMatch */
	/* przerwanie wykonywane z częstotliwością ok 10ms (100 razy na sekundę) */

    //init lcd
    lcd_init(LCD_DISP_ON);
	lcd_led(0); //lcd led ON


	/* sprawdzamy ile czujników DS18xxx widocznych jest na magistrali */
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
	if( DS18X20_OK == DS18X20_read_meas(gSensorIDs[0], &subzero, &cel, &cel_fract_bits) ) display_temp(0);
	else {
		lcd_gotoxy(1,0);
		lcd_puts(" error ");	/* wyświetlamy informację o błędzie jeśli np brak czujnika lub błąd odczytu */
	}

	/* dokonujemy odczytu temperatury z pierwszego czujnika o ile został wykryty */
	if( DS18X20_OK == DS18X20_read_meas(gSensorIDs[1], &subzero, &cel, &cel_fract_bits) ) display_temp(9);
	else {
		lcd_gotoxy(1,9);
		lcd_puts(" error ");
	}

	sei();	/* włączamy globalne przerwania */

//	unsigned char temp;
//	sec = 0;
	struct rtc_time ds1302;
	struct rtc_time *rtc;
	rtc = &ds1302;

	ds1302_init();
	ds1302_update(rtc);   // update all fields in the struct
	ds1302_set_time(rtc, SEC, 31);	//set the seconds to 31


	lcd_gotoxy(0,0);
	lcd_puts_p(PSTR("Tempertaura: ")); /* wyświetl napis na LCD */


    while(1)
        {
    		ds1302_update_time(rtc, SEC);	//read the seconds
    		lcd_gotoxy(0,0);
    		lcd_puti(rtc->second);

			if(s1_flag) {	/* sprawdzanie flagi tyknięć timera programowego co 1 sekundę */

				/* co trzy sekundy gdy reszta z dzielenia modulo 3 == 0 sprawdzaj ilość dostępnych czujników */
				if( 0 == (sekundy%3) ) {

					uint8_t *cl=(uint8_t*)gSensorIDs;	// pobieramy wskaźnik do tablicy adresów czujników
					for( uint8_t i=0; i<MAXSENSORS*OW_ROMCODE_SIZE; i++) *cl++ = 0; // kasujemy całą tablicę
					czujniki_cnt = search_sensors();	// ponownie wykrywamy ile jest czujników i zapełniamy tablicę
					lcd_gotoxy(14,0);
					lcd_puti( czujniki_cnt );	// wyświetlamy ilość czujników na magistrali
					PORTC ^= (1<<LED1_PIN);
				}

				/* co trzy sekundy gdy reszta z dzielenia modulo 3 == 1 wysyłaj rozkaz pomiaru do czujników */
				if( 1 == (sekundy%3) ) {
					DS18X20_start_meas( DS18X20_POWER_EXTERN, NULL );
					PORTC ^= (1<<LED2_PIN);
				}

				/* co trzy sekundy gdy reszta z dzielenia modulo 3 == 2 czyli jedn� sekund� po rozkazie konwersji
				 *  dokonuj odczytu i wyświetlania temperatur z 2 czujników jeśli są podłączone, jeśli nie
				 *  to pokaż komunikat o błędzie
				 */
				if( 2 == (sekundy%3) ) {
					if( DS18X20_OK == DS18X20_read_meas(gSensorIDs[0], &subzero, &cel, &cel_fract_bits) ) display_temp(0);
					else {
						lcd_gotoxy(0,1);
						lcd_puts(" error ");
					}

					if( DS18X20_OK == DS18X20_read_meas(gSensorIDs[1], &subzero, &cel, &cel_fract_bits) ) display_temp(9);
					else {
						lcd_gotoxy(9,1);
						lcd_puts(" error ");
					}
					PORTC ^= (1<<LED3_PIN);
				}

				/* zerujemy flag� aby tylko jeden raz w ci�gu sekundy wykona� operacje */
				s1_flag=0;
			} /* koniec sprawdzania flagi */



        }
    return 0;
}


void display_temp(uint8_t x)
{
	lcd_gotoxy(x,1);
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

	if(++cnt>99) {	/* gdy licznik ms > 99 (min�a 1 sekunda) */
		s1_flag=1;	/* ustaw flag� tykni�cia sekundy */
		sekundy++;	/* zwi�ksz licznik sekund */
		if(sekundy>59) sekundy=0; /* je�li ilo�� sekund > 59 - wyzeruj */
		cnt=0;	/* wyzeru licznik setnych ms */
	}
}

