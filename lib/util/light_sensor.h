#ifndef __LIGHT_SENSOR_H__
#define __LIGHT_SENSOR_H__

#include <functional>

class LightSensor {

  public:
    typedef std::function<void(int)> HandlerFunction;

    LightSensor();
    void onReading(HandlerFunction);
    void onLevelSet(HandlerFunction);
    void handle();

    void setFrequency(int);

  private:
    unsigned long _interval;
    HandlerFunction _readingHandler;
    std::function<void(int)> _levelHandler;
};

#endif // __LIGHT_SENSOR_H__
