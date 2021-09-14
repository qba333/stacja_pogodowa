# Stacja pogodowa

## Opis
  
  Projekt jest tworzony z myślą o nauce programowania. Aktualnie układ zbudowany jest na płytce stykowej, program wyświetla na ekranie LCD pomiar jednego z dwóch czujników temperatury, przyciskiem zmienia czujnik z którego ma być wyświetlony pomiar.


## Wykorzystane elementy

- Arduino Uno - zawiera mikrokontroler ATmega328P
- dioda LED
- LCD HD44780 - typowy wyświetlacz 2x16
- PCF8574T - ekspander w module jako konwerter do podłączenia LCD za pomocą magistrali i2c
- 2x DS18B20 - czujniki temperatury korzystające z iterfejsu 1-Wire
- przycisk micro switch


## Planowane modyfikacje

- dodanie czujnika wilgoności
- dodanie komunikacji bluetooth -> smartfon
- dodanie układu RTC


## Aktualnne podłączenie - prototyp

![Aktualny układ](../../blob/main/pictures/stacja_prototyp.JPG)
