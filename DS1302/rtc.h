// rtc.c
// author COSAR DINDAR
// Date: November,23th 2012


#ifndef _RTC_H_ 
#define _RTC_H_ 

// ds1302 connection 
#define ds1302_PORT PORTD
#define ds1302_DDR DDRD
#define ds1302_PIN PIND
#define rst 4
#define clk 6
#define io 7

// Ustawianie stanów wyjśc
#define rst_1() ds1302_PORT |= (1<<rst); 
#define rst_0() ds1302_PORT &= ~ (1<<rst); 
#define clk_1() ds1302_PORT |= (1<<clk); 
#define clk_0() ds1302_PORT &= ~ (1<<clk); 
#define io_1() 	ds1302_PORT |= (1<<io); 
#define io_0() 	ds1302_PORT &= ~ (1<<io); 

#define AM		10
#define PM		11
#define H24		12

#define READ	15
#define WRITE	16

#define mon 	1
#define tue 	2
#define wed 	3
#define thu 	4
#define fri 	5
#define sat 	6
#define sun 	7

// Adresy rejestrów do odczytu/zapisu
#define sec_w 	0x80
#define sec_r 	0x81
#define min_w 	0x82
#define min_r 	0x83
#define hour_w 	0x84
#define hour_r 	0x85
#define date_w 	0x86
#define date_r 	0x87
#define month_w 0x88
#define month_r 0x89
#define day_w 	0x8a
#define day_r 	0x8b
#define year_w 	0x8c
#define year_r 	0x8d

#define SEC 20
#define MIN 21
#define HOUR 22
#define DAY 22
#define DATE 23
#define MONTH 24
#define YEAR 25

#define w_protect 0x8e

struct rtc_time{
   char second; 
   char minute; 
   char hour; 
   char day;
   char date;
   char month; 
   char year;
   char hour_format;
   };

void ds1302_update_time(struct rtc_time *, unsigned char);
void ds1302_set_time(struct rtc_time * time, unsigned char field, unsigned char w_byte);
void ds1302_comms(struct rtc_time *, unsigned char, unsigned char, unsigned char);
void ds1302_update(struct rtc_time *);
void ds1302_init (void); //ds1302 init 
void ds1302_reset(void);	 //ds1302_reset 
unsigned char ds1302_read_byte(unsigned char);
void ds1302_write_byte(unsigned char, unsigned char);
void write(unsigned char);
unsigned char read(void);
void ds1302_write_byte(unsigned char,unsigned char);

#endif 
