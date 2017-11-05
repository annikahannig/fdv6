
/**
 * FDv6 Sound Over Uart
 * --------------------
 */

#include "config.h"

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "uart.h"

#define RX_CMD(buf, x) (strncmp(buf, x, sizeof(x)-1)==0)

#define STATUS_PORT PORTC
#define STATUS_DDR  DDRC
#define STATUS_LED  (1<<PC3)

#define FD_PORT PORTC
#define FD_DDR  DDRC

#define FD_DIR  (1<<PC4)
#define FD_STEP (1<<PC5)

#define FD_FORWARD ( FD_PORT &= ~FD_DIR )
#define FD_BACKWARD ( FD_PORT |= FD_DIR )

#define FD_IS_FORWARD (( FD_PORT & FD_DIR ) == 0)
#define FD_IS_BACKWARD (( FD_PORT & FD_DIR ) > 0)

#define FD_POS_MAX 78


/*
 * Print welcome and version
 */
void _cmd_helo()
{
  USART_writeln("999 FDv6 VERSION 1.0.0");
  USART_writeln("001 READY");
}


void fd_step()
{
  // pulse
  FD_PORT &= ~FD_STEP; // pull to ground
  _delay_us(10);
  FD_PORT |= FD_STEP;
  _delay_us(10);
}

void fd_reset()
{
  uint8_t i;

  FD_FORWARD;

  for( i = 0; i < FD_POS_MAX; i++ ) {
    fd_step();
    _delay_us(5000);
  }

  FD_BACKWARD;

  for( i = 0; i < FD_POS_MAX; i++ ) {
    fd_step();
    _delay_us(5000);
  }

 FD_FORWARD;
}

/*
 * Helper wait longer that 255 ms
 */
void wait_ms(uint16_t delay)
{
  while( delay-- ) {
    _delay_ms(1);
  }
}

/**
 * Step motor control timer interupt
 */
ISR( TIMER1_COMPA_vect ) {
  static uint8_t pos;

  fd_step();
  if( FD_IS_FORWARD ) {
    pos++;
  }
  else {
    pos--;
  }

  if( pos >= FD_POS_MAX ) {
    FD_BACKWARD;
  }
  else if( pos == 0 ) {
    FD_FORWARD;
  }
}

void fd_set_tone(uint16_t tone) {
  // recalculate compare
  TCNT1 = 0;
  OCR1A = (uint16_t) ((float)F_CPU / (1024.0f * (float)tone))-1;
}

void fd_play()
{
  TIMSK1 |= (1<<OCIE1A);
}

void fd_stop()
{
  TIMSK1 &= ~(1<<OCIE1A);
}

void fd_init()
{
  // Set outputs
  FD_DDR |= FD_DIR|FD_STEP;
  fd_reset();
  _delay_ms(100);

  // Setup tone generator timer
  TCCR1A = 0;
  TCCR1B = (1<<WGM12)|(1<<CS12)|(1<<CS10); // Clear on match, Prescale: 1024
}


int main(void)
{
  char *cmd;
  uint8_t tone = 0;

  // Setup UART
  USART_init();
  fd_init();

  _cmd_helo();
  // Enable interrupts
  sei();

  for(;;) {
    cmd = USART_read();
    if (cmd == NULL) {
      _delay_us(10);
      continue;
    }

    if RX_CMD(cmd, "HELO") {
      _cmd_helo();
    }
    else if RX_CMD(cmd, "023 ") { // Set row 0
      USART_writeln("200 ACK");
      tone = atoi(cmd + 4);
      fd_set_tone(tone);
      if (tone > 0) {
        fd_play();
      }
      else {
        fd_stop();
      }
    }
  }
}


