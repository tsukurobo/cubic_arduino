#include <Arduino.h>

namespace nano33BLE_digitalWriteFast
{
    constexpr PinName DIGITAL_PIN_PIN_NAMES[] = {PinName::p35, PinName::p42, PinName::p43, PinName::p44, PinName::p47, PinName::p45, PinName::p46, PinName::p23, PinName::p21, PinName::p27, PinName::p34, PinName::p33, PinName::p40, PinName::p13, PinName::p4, PinName::p5, PinName::p30, PinName::p29, PinName::p31, PinName::p2, PinName::p28, PinName::p3, PinName::p24, PinName::p16, PinName::p6, PinName::p41, PinName::p19, PinName::p17, PinName::p26, PinName::p25, PinName::p14, PinName::p15, PinName::p32, PinName::p22};
    constexpr PinName Pin(uint8_t pin)
    {
        return DIGITAL_PIN_PIN_NAMES[pin];
    }
    inline void digitalWriteFast(PinName pinName, PinStatus val)
    {
        if (val)
        {
            nrf_gpio_pin_set(pinName);
        }
        else
        {
            nrf_gpio_pin_clear(pinName);
        }
    }
}