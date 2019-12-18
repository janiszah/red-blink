
#include <avr/sleep.h>

typedef struct {
  uint8_t dir;
  uint8_t out;
} led_t;

#define L1  PIN1_bm
#define L2  PIN2_bm
#define L3  PIN3_bm

const led_t led_data[] = {
  { (L2 | L3), (L3) },
  { (L2 | L3), (L2) },
  { (L1 | L2), (L1) },
  { (L1 | L2), (L2) },
  { (L1 | L3), (L1) },
  { (L1 | L3), (L3) },
};


uint16_t y = 0;

void Button_callback(void)
{
  y ^= 1;
}

void setup() {
  PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
  attachInterrupt(0, Button_callback, FALLING);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
}


uint8_t x = 0;
uint32_t t = 0;
uint8_t state = 0;

void loop() {

  if((millis() - t) > (200)) {
    t = millis();
    state ^= 1;
  }

  if(state && y) {
    PORTA.DIR = led_data[x].dir;
    PORTA.OUT = led_data[x].out;
    delay(10);
    x++;
    if(x >= 6)
      x = 0;
  } else {
    PORTA.DIR = 0x8E;
    PORTA.OUT = 0;
  }

  if(y == 0)
    sleep_cpu();
}
