#include "timer.h"
#include "io.h"
#include "print.h"

#define IRQ0_FREQUENCY             100
#define INPUT_FREQUENCY         1193180
#define COUNTER0_VALUE        INPUT_FREQUENCY / IRQ0_FREQUENCY
#define COUNTER0_PORT        0X40
#define COUNTER0_NO         0
#define COUNTER_MODE        2
#define READ_WRITE_LATCH    3
#define PIT_COUNTROL_PORT    0x43

static void frequency_set(uint8_t counter_port,\
              uint8_t counter_no,\
              uint8_t rwl,\
              uint8_t counter_mode,\
              uint16_t counter_value)
{
    outb(PIT_COUNTROL_PORT,(uint8_t) (counter_no<<6|rwl<<4|counter_mode<<1));
    outb(counter_port,(uint8_t)counter_value);
    outb(counter_port,(uint8_t)counter_value>>8);
}

void timer_init(void)
{
    put_str("timer_init start!\n");
    frequency_set(COUNTER0_PORT,\
          COUNTER0_NO,\
          READ_WRITE_LATCH,\
          COUNTER_MODE,\
          COUNTER0_VALUE);
    put_str("timer_init done!\n");
}