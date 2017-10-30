#include <lcd_lib.h>

const char line[32] = "Here are some words";

void setup()
{ 
  debug_init();
  lcd_init();
  _delay_ms(500); // wait for display to boot up
  lcd_purge();
}

void loop() 
{ 
  lcd_purge();
  lcd_write(line);
  debug_out("example\n");
  _delay_ms(2000);
}
