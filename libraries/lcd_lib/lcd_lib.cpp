#include "lcd_lib.h"
#include <string.h>

#if LCD_MODE == 'ARDUINO'
  #include <SoftwareSerial.h>
#endif
#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>


#if LCD_MODE == 'ARDUINO'
  SoftwareSerial mySerial(LCD_PIN,LCD_PIN); // pin 2 = TX, pin 3 = RX (unused)
#endif

void debug_init()
{
  #if LCD_MODE == 'ARDUINO'
    Serial.begin(BAUD_RATE);
  #elif LCD_MODE == 'ORIGINAL' || LCD_MODE == 'DUMMY'
    //Configure 9600 baud, 8-bit no parity, 1 stop bit
    
    UBRR0H = (F_CPU/(BAUD_RATE*16L)-1) >> 8;
    UBRR0L = (F_CPU/(BAUD_RATE*16L)-1);
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
  #endif
}

void debug_out(char* text)
{
  #if LCD_MODE == 'ARDUINO'
    Serial.println(text);
  #elif LCD_MODE == 'ORIGINAL' || LCD_MODE == 'DUMMY'
    put_string(text);
  #endif
}

void put_string(char* string)
{
  for(int i = 0; i < strlen(string);i++)
  {
    while(!(UCSR0A & _BV(UDRE0)));
      UDR0 = string[i]; 
  }
}

void lcd_init()
{
#if LCD_MODE == 'ARDUINO'
     mySerial.begin(LCD_BAUD_RATE);
  #elif LCD_MODE == 'DUMMY'
	 debug_init();
  #elif LCD_MODE == 'ORIGINAL'
    LCD_DDR |= 1 << LCD_PIN;
  #endif
}
void lcd_purge()
{
  #if LCD_MODE == 'ARDUINO'
    mySerial.write(254); // cursor to beginning of first line
    mySerial.write(128);
    mySerial.write("                ");
    mySerial.write("                ");
  #elif LCD_MODE == 'DUMMY'
	debug_out("LCD PURGED\n");
  #elif LCD_MODE == 'ORIGINAL'
    uart_send_char(254); // cursor to beginning of first line
    uart_send_char(128);
    lcd_write("                                ");
  #endif
}
void lcd_write(char* text)
{
  #if LCD_MODE == 'ARDUINO'
    mySerial.write(254); // cursor to beginning of first line
    mySerial.write(128);
    mySerial.write(text);
  #elif  LCD_MODE == 'DUMMY'
    debug_out("LCD_SCREEN: ");
    debug_out(text);
    debug_out("\n");
  #elif LCD_MODE == 'ORIGINAL'
      uart_send_char(254); // cursor to beginning of first line
      uart_send_char(128);
	  pad_text(text);
      for(int i = 0; i < 32; i++)
          uart_send_char(text[i]);
  #endif
}


#if LCD_MODE == 'ORIGINAL'

void pad_text(char* text)
{
	int check = 0;
    for(int i = 0; i < 32; i++)
	{
		if(text[i] == '\0')
			check = 1;
		if(check)		
			text[i] = ' ';
	}
}

void uart_send_char(char c)
{
      LCD_PORT &= ~(1 << LCD_PIN);   
      _delay_us(UART_DELAY);
      for(int i = 0; i < 8; i++)
      {
        if((c >> i) & 00000001)
          LCD_PORT |= 1 << LCD_PIN;   
        else
          LCD_PORT &= ~(1 << LCD_PIN);   
        
        _delay_us(UART_DELAY);
      }
      LCD_PORT &= ~(1 << LCD_PIN);   
      _delay_us(UART_DELAY);
      LCD_PORT |= 1 << LCD_PIN;
      _delay_us(UART_DELAY);
}

#endif

