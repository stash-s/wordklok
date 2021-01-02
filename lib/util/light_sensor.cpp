#ifdef ARDUINO
#include <Arduino.h>
#endif // ARDUINO

#include "light_sensor.h"

#define LIGHT_LEVEL_MAX 11
static const unsigned int light_level_bounds[] = {200, 300, 400, 500, 600, 700,
                                                  785, 830, 900, 990, 1023};
static const size_t pwm_level[] = {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 16};

LightSensor::LightSensor()
    : _interval(100), _readingHandler([](int i) {}),
      _levelHandler([](int i) {}) {}

void LightSensor::onReading(HandlerFunction handler) {
    _readingHandler = handler;
}

void LightSensor::onLevelSet(HandlerFunction handler) {
    _levelHandler = handler;
}

inline size_t map_adc_to_level(size_t adc) {
    for (int i = 0; i < LIGHT_LEVEL_MAX; ++i) {
        if (light_level_bounds[i] >= adc) {
            return pwm_level[i];
        }
    }
    return pwm_level[LIGHT_LEVEL_MAX];
}

void LightSensor::handle() {
#ifdef ARDUINO
    static size_t lastLevel = 2;
    static auto last_millis = millis();
    auto current_millis = millis();

    if (current_millis - last_millis >= _interval) {
        last_millis = current_millis;

        auto value = analogRead(A0);
        auto lightLevel = map_adc_to_level(value);


        _readingHandler(value);
        if (lightLevel != lastLevel) {
            lastLevel = lightLevel;
            _levelHandler(lightLevel);
        }
    }
#endif
}

void LightSensor::setFrequency(int frequency) {
    _interval = round((1000.0 / ((double)frequency)));
}
