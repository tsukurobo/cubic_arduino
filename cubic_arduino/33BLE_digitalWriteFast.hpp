#include <Arduino.h>

namespace nano33BLE_digitalWriteFast
{
    constexpr PinName D(uint8_t pin)
    {
        PinName DigitalPinNames[] = {PinName::P1_11,
                                     PinName::P1_12,
                                     PinName::P1_15,
                                     PinName::P1_13,
                                     PinName::P1_14,
                                     PinName::P0_23,
                                     PinName::P0_21,
                                     PinName::P0_27,
                                     PinName::P1_2,
                                     PinName::P1_1,
                                     PinName::P1_8,
                                     PinName::P0_13};

        return DigitalPinNames[pin - 2];
    }
    constexpr PinName A(uint8_t pin)
    {
        PinName AnalogPinNames[] = {
            PinName::p4,
            PinName::p5,
            PinName::p30,
            PinName::p29,
            PinName::p31,
            PinName::p2,
            PinName::p28,
            PinName::p3,
        };
        return AnalogPinNames[pin];
    }
    constexpr PinName Pin(uint8_t pin)
    {
        return pin < 14 ? D(pin) : A(pin - 14);
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