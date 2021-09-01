# Stacja pogodowa

## Opis
  
  Projekt został stworzony z myślą o jego dalszym rozwijaniu i dodawaniu kolejnych możliwości. Aktualnie układ zbudowany jest na płytce stykowej.


## Wykorzystane elementy

- Arduino Uno - zawiera mikroprocesor ATmega328P
- dioda LED
- LCD HD44780 - typowy wyświetlacz 2x16
- PCF8574T - ekspander w module jako konwerter do podłączenia LCD za pomocą magistrali i2c
- 2x DS18B20 - czujniki temperatury korzystające z iterfejsu 1-Wire


## Planowane modyfikacje

- dodanie czujnika wilgoności
- dodanie komunikacji bluetooth -> smartfon
- dodanie układu RTC
