#pragma once
#ifdef ARDUINO
#include <Arduino.h>
#else
#define ICACHE_RAM_ATTR
#endif // ARDUINO
#include <stdint.h>

// Display output pin assignments
#define IT (1 << 1)
#define IS (1 << 2)

#define MTEN (1 << 3)
#define HALF 1
#define QUARTER (1 << 10)
#define TWENTY (1 << 4)
#define MFIVE (1 << 11)
#define MINUTES (1 << 5)
#define PAST (1 << 12)

#define TO (1 << 17)
#define ONE (1 << 15)
#define TWO (1 << 14)
#define THREE (1 << 13)
#define FOUR (1 << 16)
#define HFIVE (1 << 6)
#define SIX (1 << 20)

#define SEVEN (1 << 19)
#define EIGHT (1 << 7)
#define NINE (1 << 21)
#define HTEN (1 << 18)
#define ELEVEN (1 << 9)
#define TWELVE (1 << 22)
#define OCLOCK (1 << 8)

const int SHDN_PIN = 4;

class WordKlokDisplay {
  public:
    WordKlokDisplay();

    void init();
    void stop();
    void showTime(int hours, int minutes);
    void setLightLevel(int level);
    void setScrollBar(int level);

    static void startAnimation();
    static void endAnimation();
    static ICACHE_RAM_ATTR void animationStep();

    typedef uint32_t payload_t;

    payload_t encodeTime(int hours, int minutes);

    private:
      static int _display_timer_divider;

      static int _animationStep;
      static int _animationCounter;
      static int _animationTreshold;
      static bool _animationStarted;
};