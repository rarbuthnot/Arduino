/*******************************************************************
    Copyright (C) 2009 FreakLabs
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.

    Originally written by Christopher Wang aka Akiba.
    Please post support questions to the FreakLabs forum.

*******************************************************************/
/*!
    \file 
    \ingroup


*/
/**************************************************************************/
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "chb.h"
#include "chb_demo.h"
#include "chb_buf.h"
#include "chb_drvr.h"
#include "freakusb.h"

// 65536 - 1953 = 63583, these are the counts needed for 250 msec interrupt
// using the timer prescaler settings
#define TICK_CNT 63583  
static U16 freq = 0;
static bool update = false;

/**************************************************************************/
/*!
    This is the main command table.
*/
/**************************************************************************/
cmd_t cmd_tbl[] = 
{
    {"hello",   cmd_hello},
    {"radar", cmd_radar_cnt},
    {NULL,      NULL}
};

/**************************************************************************/
/*!
    Retrieves a pointer to the command table. This is used by the command
    line to parse the table.
*/
/**************************************************************************/
cmd_t *cmd_get()
{
    return cmd_tbl;
}

/**************************************************************************/
/*!
    This is the main function.
*/
/**************************************************************************/
int main()
{ 
    double speed;

    // init the command line
    cmd_init();

    // turn on 3.3V LED
    DDRC |= 1<<PORTC7;
    PORTC |= 1<<PORTC7;

    // and off we go...
    while (1)
    {
        cmd_poll();

        // check if we have a new frequency reading. 
        if (update == true)
        {
            if (freq != 0)
            {
                // formula used is: speed = freq * c/((24*10^9) * 2)
                // c is in km/hr & if you calc all constants, final formula is 
                // speed = freq * 0.0225 where speed is in km/hr 
                freq = freq << 2;       // multiple the frequency * 4 (using leftshift 2 places)
                speed = freq * 0.0225;  // multiplying freq * 0.0225 will give speed in km/hr
                printf("Freq: %02d Hz Speed: %2.3f km/hr.\n", freq, speed); 
                update = false;         
            }
        }
    }
    return 0;
}

/**************************************************************************/
/*!
    This is just to initialize the radar sensor. We're going to use Timer/Counter
    1 to capture the number of edges it sees on the T1 pin. We're also setting
    Timer 3 to interrupt every 250 msec. On each interrupt, we'll multiply the
    number of edges we count * 4 which will give us an estimated frequency. Hence
    if we see 15 edges in 250 msec, then that's approximately equivalent to 60
    edges/second or 60 Hz.
 
    Usage:
    radar <arg>
 
    arg = 1 turns initializes and enables the radar sensor
    arg = 0 disables it.
*/
/**************************************************************************/
void cmd_radar_cnt(U8 argc, char **argv)
{
    U8 on = strtol(argv[1], NULL, 10);

    if (on)
    {
        // init timer1 to increment on each positive edge on the T1 input
        TCNT1   = 0;          
        TCCR1B  |= _BV(CS12) | _BV(CS11) | _BV(CS10);   

        // init timer0 to interrupt every 250 msec
        TCNT3   = TICK_CNT;                 // we want one tick to be 250 msec
        TIMSK3  = _BV(TOIE3);               // enable the timer 0 overflow interrupt 
        TCCR3B  |= _BV(CS32) | _BV(CS30);   // init clock prescaler so timer increments every 250 usec
    }
    else
    {
        // disable all timers
        TCCR1B = 0;
        TCCR3B = 0;
        TIMSK3 = 0;
    }
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void cmd_hello(U8 argc, char **argv)
{
    printf_P(PSTR("Hello World!\n"));
}

/**************************************************************************/
/*!
    Dummy ISR. In case we overflow, it won't cause a mysterious hang. Timer/Counter
    1 is just used to count number of edges we see on Timer/Capture 1 input.
*/
/**************************************************************************/
ISR(TIMER1_OVF_vect)
{
    // do nothing. this is just a dummy ISR in case it actually overflows.
}

/**************************************************************************/
/*!
    Timer 3 is set to interrupt every 250 msec. On each interrupt, we just
    copy the number of edges we saw, reset the edge counter, reset the
    timer 3 counter so that it will interrupt again after 250 msec, and then
    let the main function know we updated the value. 
*/
/**************************************************************************/
ISR(TIMER3_OVF_vect)
{
    freq = TCNT1;
    TCNT1 = 0;
    TCNT3 = TICK_CNT;
    update = true;
}

