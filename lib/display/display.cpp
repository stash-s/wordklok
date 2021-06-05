
#include "display.h"
#ifdef ARDUINO
#include <Arduino.h>
#include <SPI.h>
#endif // ARDUINO

#define PWM_MAX 33

WordKlokDisplay::WordKlokDisplay() {}

static volatile WordKlokDisplay::payload_t _payload = 0;

int WordKlokDisplay::_display_timer_divider;
int WordKlokDisplay::_animationStep = 0;
int WordKlokDisplay::_animationCounter = 0;
int WordKlokDisplay::_animationTreshold = 0;
bool WordKlokDisplay::_animationStarted = false;
static volatile int _pwm_level = PWM_MAX - 1;

#ifdef ARDUINO
IRAM_ATTR
inline void setDataBits(uint16_t bits) {
    const uint32_t mask = ~((SPIMMOSI << SPILMOSI) | (SPIMMISO << SPILMISO));
    --bits;
    SPI1U1 = ((SPI1U1 & mask) | ((bits << SPILMOSI) | (bits << SPILMISO)));
}

IRAM_ATTR
static void shift_out(WordKlokDisplay::payload_t payload) {

    while (SPI1CMD & SPIBUSY) { // wait until HSPI is ready.
    }

    setDataBits(24);  // set up
    SPI1W0 = payload; //

    SPI1CMD |= SPIBUSY; // initiate transfer
}

IRAM_ATTR
static void isr_call() {

    static int pwm_counter = 0;

    WordKlokDisplay::animationStep();

    if (0 == pwm_counter) {
        shift_out(_payload);
    } else {
        if (pwm_counter == _pwm_level) {
            shift_out(0);
        }
    }

    ++pwm_counter;
    if (pwm_counter >= PWM_MAX) {
        pwm_counter = 0;
    }
}

#endif // ARDUINO

void WordKlokDisplay::init() {
#ifdef ARDUINO

    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setHwCs(true);
    SPI.setBitOrder(MSBFIRST);
    SPI.setFrequency(1000000);

    _display_timer_divider = (clockCyclesPerMicrosecond() / 16) *
                             300; // 300us = 3.3kHz sampling freq

    timer1_isr_init();
    timer1_attachInterrupt(isr_call);
    timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
    timer1_write(_display_timer_divider);

    pinMode(SHDN_PIN, OUTPUT);
    digitalWrite(SHDN_PIN, LOW);

#endif // ARDUINO
}

void WordKlokDisplay::stop() {
#ifdef ARDUINO
    digitalWrite(SHDN_PIN, HIGH);
#endif // ARDUINO
}

inline void setPayload(WordKlokDisplay::payload_t payload) {
    union {
        uint32_t l;
        uint8_t b[4];
    } data_;
    data_.l = payload;
    // MSBFIRST Byte first
    _payload = (data_.b[2] | (data_.b[1] << 8) | (data_.b[0] << 16));
}

void WordKlokDisplay::showTime(int hours, int minutes) {

    payload_t payload = encodeTime(hours, minutes);
    setPayload(payload);
}

WordKlokDisplay::payload_t encodeMinutes(int minutes);

WordKlokDisplay::payload_t hour_to_digit[] = {
    TWELVE, ONE,   TWO,  THREE, FOUR,   HFIVE, SIX,
    SEVEN,  EIGHT, NINE, HTEN,  ELEVEN, TWELVE};

WordKlokDisplay::payload_t WordKlokDisplay::encodeTime(int hours, int minutes) {

    payload_t rv = IT | IS;

    rv |= encodeMinutes(minutes);

    hours = hours % 12;

    if (minutes < 35) {
        rv |= hour_to_digit[hours];
    } else {
        if (hours < 12) {
            rv |= hour_to_digit[hours + 1];
        } else {
            rv |= ONE;
        }
    }

    return rv;
}

WordKlokDisplay::payload_t encodeMinutes(int minutes) {

    if (minutes >= 55) {
        return MFIVE | MINUTES | TO;
    } else if (minutes >= 50) {
        return MTEN | MINUTES | TO;
    } else if (minutes >= 45) {
        return QUARTER | TO;
    } else if (minutes >= 40) {
        return TWENTY | MINUTES | TO;
    } else if (minutes >= 35) {
        return TWENTY | MFIVE | MINUTES | TO;
    } else if (minutes >= 30) {
        return HALF | PAST;
    } else if (minutes > 24) {
        return TWENTY | MFIVE | MINUTES | PAST;
    } else if (minutes > 19) {
        return TWENTY | MINUTES | PAST;
    } else if (minutes > 14) {
        return QUARTER | PAST;
    } else if (minutes > 9) {
        return MTEN | MINUTES | PAST;
    } else if (minutes >= 5) {
        return MFIVE | MINUTES | PAST;
    } else {
        return OCLOCK;
    }
}

void WordKlokDisplay::setLightLevel(int level) { _pwm_level = level; }

void WordKlokDisplay::setScrollBar(int level) {

    payload_t payload = 0;

    switch (level) {
    case 7:
        payload |= (IT | IS | MTEN | HALF);

    case 6:
        payload |= (QUARTER | TWENTY);

    case 5:
        payload |= (MFIVE | MINUTES);

    case 4:
        payload |= (PAST | TO | ONE | TWO);

    case 3:
        payload |= (THREE | FOUR | HFIVE);

    case 2:
        payload |= (SIX | SEVEN | EIGHT);

    case 1:
        payload |= (NINE | HTEN | ELEVEN);

    case 0:
        payload |= (TWELVE | OCLOCK);

    default:
        break;
    }

    setPayload(payload);
}

void WordKlokDisplay::startAnimation() {
    _animationStarted = true;
    _animationTreshold = _display_timer_divider / 4;
    _animationStep = 0;
}
void WordKlokDisplay::endAnimation() { _animationStarted = false; }

IRAM_ATTR void WordKlokDisplay::animationStep() {
    if (_animationStarted) {

        if (_animationCounter >= _animationTreshold) {
            _animationCounter = 0;

            if (_animationStep > 7) {
                _animationStep = 0;
            }

            payload_t payload = 0;

            switch (_animationStep) {
            case 7:
                payload |= (IT | IS | MTEN | HALF);
                break;

            case 6:
                payload |= (QUARTER | TWENTY);
                break;

            case 5:
                payload |= (MFIVE | MINUTES);
                break;

            case 4:
                payload |= (PAST | TO | ONE | TWO);
                break;

            case 3:
                payload |= (THREE | FOUR | HFIVE);
                break;

            case 2:
                payload |= (SIX | SEVEN | EIGHT);
                break;

            case 1:
                payload |= (NINE | HTEN | ELEVEN);
                break;

            case 0:
                payload |= (TWELVE | OCLOCK);
                break;

            default:
                break;
            }

            setPayload(payload);

            ++_animationStep;
        }

        ++_animationCounter;
    }
}