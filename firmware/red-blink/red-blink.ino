#include <avr/sleep.h>
// #include <avr/iotn402.h>

typedef struct {
  uint8_t dir;
  uint8_t out;
} led_t;

#define L1  PIN1_bm
#define L2  PIN2_bm
#define L3  PIN3_bm
#define BTN PIN6_bm

const led_t led_data[] = {
  { (L2 | L3), (L3) },
  { (L2 | L3), (L2) },
  { (L1 | L2), (L1) },
  { (L1 | L2), (L2) },
  { (L1 | L3), (L1) },
  { (L1 | L3), (L3) },
};

#define BTN_IS_PRESSED()  ((PORTA.IN & BTN) == 0)

#define LED_ON(_x)                  \
  do {                              \
    PORTA.DIR = led_data[(_x)].dir; \
    PORTA.OUT = led_data[(_x)].out; \
  } while(0)

#define LEDS_OFF()    \
  do {                \
    PORTA.DIR = 0x8E; \
    PORTA.OUT = 0;    \
  } while(0)


uint16_t y = 0;
uint8_t state = 0;
uint8_t mode = 0;

void Button_callback(void)
{
}

void setup() {
  PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
  attachInterrupt(0, Button_callback, FALLING);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
}


uint8_t x = 0;
uint32_t t = 0;
uint8_t ledState = 0;


void loop() {
  _processPowerButton();

  if(state) {
    switch(mode) {
      case 0: _mode0(); break;
      case 1: _mode1(); break;
      case 2: _mode2(); break;
    }
  }
}


static void _mode0(void) {
  if((millis() - t) > (200)) {
    t = millis();
    ledState ^= 1;
  }

  if(ledState) {
    LED_ON(x);
    delay(20);
    x++;
    if(x >= 6)
      x = 0;
  } else {
    LEDS_OFF();
  }
}


static void _mode1(void) {
  static uint8_t brightness = 0;
  static uint8_t dir = 0;

  int32_t dt = (micros() - t);

  if(dt >= 260) {
    t = micros();
    
    x++;
    if(x >= 6) {
      x = 0;

      if(dir == 0) {
        brightness++;
        if(brightness == 0) {
          brightness = 255;
          dir = 1;
        }
      } else {
        brightness--;
        if(brightness == 255) {
          brightness = 0;
          dir = 0;
        }
      }
    }
    LED_ON(x);
  } 
  else if(dt > brightness) {
    LEDS_OFF();
  }
}


static void _mode2(void) {
  static uint8_t tim = 128;
  int32_t dt = (micros() - t);

  if(dt > 256) {
    t = micros();
    
    if(ledState)
      LED_ON(x);      

    x++;
    if(x >= 6) {
      x = 0;

      tim--;
      if(tim == 0) {
        ledState ^= 1;

        if(ledState == 0) {
          LEDS_OFF();
          tim = 128;
        } else {
          tim = 32;
        }
      }   
    }
  }

}




static void _processPowerButton(void) {
  static uint32_t btnPressTime = 0;
  static uint32_t btnDebounce = 0;
  static uint8_t btnState = 0;

  if(BTN_IS_PRESSED()) {
    if(btnState == 0) {
      btnState = 1;
      btnPressTime = millis();
    }

    if(btnState == 1 && (millis() - btnPressTime) > 500) {
      state ^= 1;

      if(state == 0) {
        btnState = 0;
        LEDS_OFF();
        sleep_cpu();
      } else {
        btnState = 2;
      }
    }
  } else {

    if(btnState == 1 && (millis() - btnDebounce) > 10) {
      btnDebounce = millis();

      mode++;
      if(mode > 2)
        mode = 0;
    }

    btnState = 0;

    if(state == 0) {
      LEDS_OFF();
      sleep_cpu();
    }
  }
}