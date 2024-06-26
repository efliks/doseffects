/*****************************************
  Mroczna Harmonia

  clock.c   Facility to handle custom timer

  (C) 2001, 2023  M. Feliks
*****************************************/

#include <dos.h>
#include <conio.h>

#define CLOCK_FREQUENCY 1193181
#define DOS_DIVISOR 65535
#define DOS_FREQUENCY CLOCK_FREQUENCY / DOS_DIVISOR   // 18.206775005722132 ticks/s

#define MY_DIVISOR 8192 
#define MY_FREQUENCY CLOCK_FREQUENCY / DOS_DIVISOR   // 145.6519775390625 ticks/s

#define TIMER_FACTOR MY_FREQUENCY / DOS_FREQUENCY  // call DOS interrupt every 8 ticks

void interrupt (*old_clock_interrupt)(void);

unsigned long timer = 0, timer_dos = 0, timer_stop;


void timer_start(unsigned long ticks)
{
    timer_stop = timer + ticks;
}

void timer_wait()
{
    while (timer < timer_stop) {
        // Reduce CPU load in VirtualBox from 100% to 65%
        _asm {
            hlt
        }
    }
}

void interrupt clock_interrupt(void)
{
    timer++;

    if ((++timer_dos) > TIMER_FACTOR) {
        timer_dos = 0;
        old_clock_interrupt();
    }
}

void clock_init(void)
{
    old_clock_interrupt = _dos_getvect(0x1c);
    _dos_setvect(0x1c, clock_interrupt);
	
    outp(0x43, 0x34);

    outp(0x40, (unsigned char)(MY_DIVISOR & 0xff));
    outp(0x40, (unsigned char)((MY_DIVISOR >> 8) & 0xff));
}

void clock_shutdown(void)
{
    _dos_setvect(0x1c, old_clock_interrupt);
    
    outp(0x43, 0x34);
    outp(0x40, -1);
    outp(0x40, -1);
}
