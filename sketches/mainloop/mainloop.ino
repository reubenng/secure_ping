#include "ENC28J60.h"
#include "lcd_lib.h"

/*
 * Main function for Secure Ping project
 * Written by Charlie Halliwell - crh2g14@soton.ac.uk
 */

uint8_t *buf;
uint8_t *localMAC;
unsigned long timer;
boolean timedOut = false;
boolean packet_sent = false;
boolean packet_rx = false;
unsigned long limit = 5000; //Timeout for send packet

const byte interruptPin = 2;

void receive_packet(){};
void purge(){};

void setup() {
  // Set initial value for buffer. Format: Length | Source MAC (6 bytes) | Target MAC (6 bytes) | Payload
  buf = (uint8_t *)malloc(60);
  strcpy(buf, "\x0A\x73\x6f\x75\x72\x63\x65\xFF\xFF\xFF\xFF\xFF\xFF\x73\x65\x6E\x64\x20\x6E\x75\x64\x65\x73"); //Broadcast frame written to buffer

  localMAC = (uint8_t *)malloc(6);
  strcpy(localMAC, "\x73\x6f\x75\x72\x63\x65");

  ENC_init(localMAC);

  //Timeout setup
  timedOut = false;
  timer = millis(); // Set initial timer value

  //Interrupt setup
  attachInterrupt(digitalPinToInterrupt(interruptPin),receive_packet, CHANGE);

  packet_sent = false;
  
}

void loop() {
  if(!packet_sent){ //If nothing has been sent, send broadcast packet
    send_packet(buf);
    packet_sent = true;
  }
 else if(millis()-timer >= limit){ //If nothing received within time limit, display no connection and purge
    lcd_write("Not connected");
    purge();
 }
  

}
