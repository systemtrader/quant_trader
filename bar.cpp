#include "bar.h"

Bar::Bar()
{
    init();
}

Bar::Bar(const Bar &other)
{
    time = other.time;
    open = other.open;
    high = other.high;
    low = other.low;
    close = other.close;
    tick_volume = other.tick_volume;
    volume = other.volume;
}

Bar::~Bar()
{
}

Bar::Bar(const KTExportBar &ktbar)
{
    time = ktbar.m_time;
    open = ktbar.m_fOpen;
    high = ktbar.m_fHigh;
    low = ktbar.m_fLow;
    close = ktbar.m_fClose;
    tick_volume = 1;
    volume = ktbar.m_fVolume;
}

void Bar::init()
{
    time = 0;
    open = -1.5f;
    high = -1.0f;
    low = 100000.0f;
    close = -1.0f;
    tick_volume = 0;
    volume = 0.0f;
}

bool Bar::isNewBar()
{
    return tick_volume == 0;
}

QDebug operator<<(QDebug dbg, const Bar &bar)
{
    dbg.nospace() <<   "time = " << bar.time
                  << ", open = " << bar.open
                  << ", high = " << bar.high
                  << ", low = " << bar.low
                  << ", close = " << bar.close
                  << ", tick_volume = " << bar.tick_volume
                  << ", volume = " << bar.volume;
    return dbg.space();
}
