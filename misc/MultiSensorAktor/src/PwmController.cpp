/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <atomic>

#include <sblib/digital_pin.h>
#include <sblib/interrupt.h>
#include <sblib/ioports.h>
#include <sblib/timer.h>

#include <PwmController.h>

PwmController pwmController;

extern "C" void TIMER32_1_IRQHandler()
{
    pwmController.softwareTimerInterrupt();
}

namespace
{

constexpr uint32_t noEdge = 0xFFFFFFFF; //!< match value CT32B1 never reaches

Timer& timerOf(const PwmController::TimerId timer)
{
    switch (timer)
    {
    case PwmController::TimerId::CT16B0:
        return timer16_0;
    case PwmController::TimerId::CT32B0:
        return timer32_0;
    default:
        return timer32_1;
    }
}

} // namespace

PwmController::HardwareChannel PwmController::hardwareChannel(const uint32_t pin)
{
    switch (pin)
    {
    case PIO0_8:
    case PIO3_0:
        return {TimerId::CT16B0, 0};
    case PIO0_9:
    case PIO3_1:
        return {TimerId::CT16B0, 1};
    case PIO3_2:
        return {TimerId::CT16B0, 2};
    case PIO1_6:
    case PIO2_5:
        return {TimerId::CT32B0, 0};
    case PIO0_1:
    case PIO2_7:
        return {TimerId::CT32B0, 2};
    case PIO0_11:
    case PIO2_8:
        return {TimerId::CT32B0, 3};
    case PIO1_4:
        return {TimerId::CT32B1, 3};
    default:
        return {TimerId::none, 0};
    }
}

uint32_t PwmController::activeTicks(const uint8_t value)
{
    return (value * periodTicks + maxValue / 2) / maxValue;
}

uint32_t PwmController::matchValue(const uint8_t value, const bool invert)
{
    const uint32_t high = invert ? periodTicks - activeTicks(value) : activeTicks(value);
    return periodTicks - high;
}

PwmController::~PwmController()
{
    delete[] channels;
    delete[] schedules[0].edges;
    delete[] schedules[1].edges;
}

int PwmController::attach(const uint32_t pin, const bool invert)
{
    if (count == capacity)
    {
        const int newCapacity = capacity + 8;
        auto* grown = new Channel[newCapacity];
        for (int i = 0; i < count; i++)
        {
            grown[i] = channels[i];
        }
        delete[] channels;
        channels = grown;
        capacity = newCapacity;
    }

    Channel channel {static_cast<uint8_t>(digitalPinToPort(pin)), static_cast<uint16_t>(digitalPinToBitMask(pin)),
                     invert, hardwareChannel(pin), 0};
    if (channel.hardware.timer != TimerId::none)
    {
        uint8_t& used = usedMatches[static_cast<int>(channel.hardware.timer)];
        if (used & (1 << channel.hardware.match))
        {
            channel.hardware.timer = TimerId::none; // the match register drives another pin already
        }
        else
        {
            used |= 1 << channel.hardware.match;
        }
    }

    if (channel.hardware.timer != TimerId::none)
    {
        startTimer(channel.hardware.timer, false);
        const ::Timer& timer = timerOf(channel.hardware.timer);
        timer.match(channel.hardware.match, matchValue(0, invert));
        timer.pwmEnable(channel.hardware.match);
        pinMode(pin, OUTPUT_MATCH);
    }
    else
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, invert);
        startTimer(TimerId::CT32B1, true);
        softwareCount++;
        dirty = true;
    }

    channels[count] = channel;
    return count++;
}

void PwmController::startTimer(const TimerId timer, const bool softwareScheduler)
{
    const int index = static_cast<int>(timer);
    const ::Timer& t = timerOf(timer);
    if (!timerStarted[index])
    {
        t.begin();
        t.prescaler(SystemCoreClock / tickHz - 1);
        t.match(periodMatch[index], periodTicks - 1);
        t.matchMode(periodMatch[index], RESET);
        t.restart();
        timerStarted[index] = true;
    }
    if (softwareScheduler && !softwareStarted)
    {
        t.match(edgeMatch, noEdge);
        t.matchMode(edgeMatch, INTERRUPT);
        t.matchMode(periodMatch[index], RESET | INTERRUPT);
        t.setIRQPriority(InterruptPriority::low); // below the bus
        t.interrupts();
        softwareStarted = true;
    }
}

PwmController::HardwareChannel PwmController::channelHardware(const int channel) const
{
    return channels[channel].hardware;
}

void PwmController::setValue(const int channel, const uint8_t value)
{
    if (channel < 0 || channel >= count)
    {
        return;
    }
    Channel& c = channels[channel];
    if (c.hardware.timer != TimerId::none)
    {
        timerOf(c.hardware.timer).match(c.hardware.match, matchValue(value, c.invert));
    }
    else if (c.value != value)
    {
        dirty = true;
    }
    c.value = value;
}

void PwmController::update()
{
    if (!dirty || pending)
    {
        return;
    }
    // The interrupt doesn't touch this schedule until pending is set
    Schedule& next = schedules[1 - active];
    if (next.edgeCapacity < softwareCount)
    {
        delete[] next.edges;
        next.edges = new PortWrite[softwareCount];
        next.edgeCapacity = softwareCount;
    }
    buildSchedule(next);
    dirty = false;
    std::atomic_signal_fence(std::memory_order_seq_cst); // the schedule is complete before the interrupt sees pending
    pending = true;
}

void PwmController::buildSchedule(Schedule& schedule) const
{
    for (int port = 0; port < 4; port++)
    {
        schedule.startMask[port] = 0;
        schedule.startValue[port] = 0;
    }
    schedule.edgeCount = 0;

    for (int i = 0; i < count; i++)
    {
        const Channel& c = channels[i];
        if (c.hardware.timer != TimerId::none)
        {
            continue;
        }
        const uint16_t activeLevel = c.invert ? 0 : c.mask;
        const uint16_t inactiveLevel = c.invert ? c.mask : 0;
        const uint32_t ticks = activeTicks(c.value);
        schedule.startMask[c.port] |= c.mask;
        if (ticks == 0)
        {
            schedule.startValue[c.port] |= inactiveLevel;
            continue;
        }
        schedule.startValue[c.port] |= activeLevel;
        if (ticks >= periodTicks)
        {
            continue;
        }

        // Insert in time order; an edge of the same time and port takes this pin too
        int at = 0;
        while (at < schedule.edgeCount && schedule.edges[at].time < ticks)
        {
            at++;
        }
        int same = at;
        while (same < schedule.edgeCount && schedule.edges[same].time == ticks && schedule.edges[same].port != c.port)
        {
            same++;
        }
        if (same < schedule.edgeCount && schedule.edges[same].time == ticks)
        {
            schedule.edges[same].mask |= c.mask;
            schedule.edges[same].value |= inactiveLevel;
            continue;
        }
        for (int k = schedule.edgeCount; k > at; k--)
        {
            schedule.edges[k] = schedule.edges[k - 1];
        }
        schedule.edges[at] = PortWrite {static_cast<uint16_t>(ticks), c.port, c.mask, inactiveLevel};
        schedule.edgeCount++;
    }
}

void PwmController::softwareTimerInterrupt()
{
    const ::Timer& t = timer32_1;
    const int flags = t.flags();
    if (flags & ::Timer::flagMask(TIMER_MATCH_MAT1))
    {
        t.resetFlag(TIMER_MATCH_MAT1);
        writeEdges();
    }
    if (flags & ::Timer::flagMask(TIMER_MATCH_MAT0))
    {
        t.resetFlag(TIMER_MATCH_MAT0);
        if (pending)
        {
            active = 1 - active;
            pending = false;
        }
        const Schedule& schedule = schedules[active];
        for (int port = 0; port < 4; port++)
        {
            if (schedule.startMask[port])
            {
                gpioPorts[port]->MASKED_ACCESS[schedule.startMask[port]] = schedule.startValue[port];
            }
        }
        nextEdge = 0;
        // The first edge is at least one step (39 ticks) after the start, its interrupt comes
        t.match(edgeMatch, schedule.edgeCount ? schedule.edges[0].time : noEdge);
    }
}

void PwmController::writeEdges()
{
    const ::Timer& t = timer32_1;
    const Schedule& schedule = schedules[active];
    while (nextEdge < schedule.edgeCount)
    {
        const PortWrite& edge = schedule.edges[nextEdge];
        if (edge.time > t.value() + edgeGuardTicks)
        {
            t.match(edgeMatch, edge.time);
            if (t.value() < edge.time)
            {
                return; // its interrupt comes
            }
            continue; // passed meanwhile, the match won't come
        }
        gpioPorts[edge.port]->MASKED_ACCESS[edge.mask] = edge.value;
        nextEdge++;
    }
    t.match(edgeMatch, noEdge);
}
