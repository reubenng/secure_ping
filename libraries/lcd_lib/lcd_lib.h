#include <config.h>


void debug_init();
void debug_out(char* text);
void lcd_init();
void lcd_purge();
void lcd_write(char* text);
void put_string(char* string);
#if LCD_MODE == 'ORIGINAL'
void pad_text(char* text);
    void uart_send_char(char c);
#endif
