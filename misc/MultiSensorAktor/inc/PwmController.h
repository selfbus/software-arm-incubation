/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef PWMCONTROLLER_H_
#define PWMCONTROLLER_H_

#include <cstdint>

/**
 * PWM outputs on the pins of the LPC1115, 100 Hz, value 0-255 (0 = off, 255 = on all the time).
 *
 * A pin gets a hardware channel if it has a match output of a free timer (CT16B0, CT32B0, CT32B1; never CT16B1,
 * which the bus uses): the timer drives the pin in its PWM mode without any interrupt. Every other pin is switched by
 * the software scheduler on CT32B1: an interrupt at the start of each period sets all its pins, one more at each
 * time a pin has to change, edges of the same time and port are written together. So every pin can be a PWM output.
 *
 * Fixed use of the match registers (period = reset of the timer, 1 MHz, 10000 ticks):
 * | timer  | period | PWM outputs                                       | interrupt                      |
 * | ------ | ------ | ------------------------------------------------- | ------------------------------ |
 * | CT16B0 | MR3    | MR0 PIO0_8/PIO3_0, MR1 PIO0_9/PIO3_1, MR2 PIO3_2   | none                           |
 * | CT32B0 | MR1    | MR0 PIO1_6/PIO2_5, MR2 PIO0_1/PIO2_7, MR3 PIO0_11/PIO2_8 | none                     |
 * | CT32B1 | MR0    | MR3 PIO1_4                                        | MR0, MR1 for the software pins |
 *
 * PIO0_10 (SWCLK) and PIO1_3 (SWDIO) are never switched to a match output. A match register drives all of its pins
 * alike, so only the first pin of a register gets it, the others are software pins.
 *
 * Software pins stand still while interrupts are disabled (writing a flash page, about 100 ms) and are delayed while
 * a higher interrupt (the bus) runs; their interrupt has the low priority. A new value applies from the next period
 * on, after update(). A hardware pin takes a new value at once; if it had not switched in the current period yet and
 * the new switching time is already past, it stays at the start level for the rest of this one period.
 */
class PwmController
{
public:
    static constexpr uint32_t tickHz = 1000000;                   //!< timer clock after the prescaler
    static constexpr uint32_t frequencyHz = 100;                  //!< PWM frequency
    static constexpr uint32_t periodTicks = tickHz / frequencyHz; //!< ticks per period
    static constexpr uint8_t maxValue = 255;                      //!< value for on all the time

    /** Timers usable for PWM. */
    enum class TimerId : uint8_t
    {
        CT16B0,
        CT32B0,
        CT32B1,
        none
    };

    /** A timer match output that drives a pin. */
    struct HardwareChannel
    {
        TimerId timer;
        uint8_t match;
    };

    /**
     * The match output that can drive @p pin as PWM, by the fixed use above.
     *
     * @param pin The pin: PIO0_8, ...
     * @return The timer and match register, timer TimerId::none if the pin can only be a software pin.
     */
    [[nodiscard]] static HardwareChannel hardwareChannel(uint32_t pin);

    /**
     * Match register value of a hardware channel: the pin is low from the start of the period until the match, high
     * from then on.
     *
     * @param value  0-255
     * @param invert true: high at 0, low at 255
     * @return 0 for high all the time, periodTicks (not reached) for low all the time.
     */
    [[nodiscard]] static uint32_t matchValue(uint8_t value, bool invert);

    /**
     * Ticks from the start of the period that a software pin is active (high, low if inverted).
     *
     * @param value 0-255
     * @return 0 for value 0, periodTicks for 255.
     */
    [[nodiscard]] static uint32_t activeTicks(uint8_t value);

    PwmController() = default;
    ~PwmController();

    /**
     * Make @p pin a PWM output with value 0: an output at the inactive level (low, high if inverted), driven by a
     * hardware channel if there is a free one, else by the software scheduler. Starts the timer it needs.
     * Call it during setup(), before the interrupts are enabled; a pin must not be attached twice.
     *
     * @param pin    The pin: PIO0_8, ...
     * @param invert true: the output is low while on
     * @return The number of the channel for setValue().
     */
    int attach(uint32_t pin, bool invert);

    /**
     * Set the value of a channel. Hardware channels change at once, software channels with the next update().
     *
     * @param channel Number from attach()
     * @param value   0-255
     */
    void setValue(int channel, uint8_t value);

    /**
     * Hand changed values of the software channels to the interrupt. It takes them at the start of the next period;
     * until then further changes wait for the next call. Call it from the application loop.
     */
    void update();

    /**
     * Interrupt of the software scheduler (CT32B1): MR0 starts a period, MR1 is the next edge.
     */
    void softwareTimerInterrupt();

    /** @return Number of attached channels. */
    [[nodiscard]] int channelCount() const { return count; }

    /** @return The hardware channel of @p channel, timer TimerId::none for a software channel. */
    [[nodiscard]] HardwareChannel channelHardware(int channel) const;

    /** One write of the software scheduler: the masked pins of one GPIO port. */
    struct PortWrite
    {
        uint16_t time;  //!< ticks from the start of the period
        uint8_t port;   //!< GPIO port 0-3
        uint16_t mask;  //!< pins written
        uint16_t value; //!< their new levels
    };

    /** What the interrupt writes in one period: the start levels of all software pins, then the edges in time order. */
    struct Schedule
    {
        uint16_t startMask[4];  //!< software pins of each port
        uint16_t startValue[4]; //!< their levels at the start of the period
        uint8_t edgeCount;
        uint8_t edgeCapacity;
        PortWrite* edges;       //!< room for one edge per software channel
    };

    /** @return The schedule the interrupt uses now (for the tests). */
    [[nodiscard]] const Schedule& activeSchedule() const { return schedules[active]; }

    /** @return true while a new schedule waits for the start of a period (for the tests). */
    [[nodiscard]] bool schedulePending() const { return pending; }

private:
    struct Channel
    {
        uint8_t port;
        uint16_t mask;
        bool invert;
        HardwareChannel hardware;
        uint8_t value;
    };

    static constexpr uint8_t periodMatch[3] = {3, 1, 0}; //!< period match register of CT16B0, CT32B0, CT32B1
    static constexpr uint8_t edgeMatch = 1;               //!< match register of CT32B1 for the software edges
    static constexpr uint32_t edgeGuardTicks = 2;         //!< edges this close to the current one are written too

    void startTimer(TimerId timer, bool softwareScheduler);
    void buildSchedule(Schedule& schedule) const;
    void writeEdges();

    Channel* channels = nullptr;
    int count = 0;
    int capacity = 0;
    uint8_t softwareCount = 0;
    bool timerStarted[3] = {false, false, false};
    bool softwareStarted = false;
    uint8_t usedMatches[3] = {0, 0, 0}; //!< bit n: match register n drives a pin

    Schedule schedules[2] = {};
    volatile uint8_t active = 0;   //!< schedule the interrupt uses; only the interrupt changes it
    volatile bool pending = false; //!< schedules[1 - active] is ready, the interrupt swaps at the next period
    bool dirty = false;            //!< a software value changed since the last schedule
    uint8_t nextEdge = 0;          //!< index of the next edge in the active schedule
};

/** The PWM outputs of the application; CT32B1's interrupt goes to it. */
extern PwmController pwmController;

#endif /* PWMCONTROLLER_H_ */
