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

bool Bar::isNewBar() const
{
    return tick_volume == 0;
}

QDataStream& operator>>(QDataStream& s, KTExportBar& bar)
{
    s >> bar.m_time;
    s >> bar.m_fOpen;
    s >> bar.m_fHigh;
    s >> bar.m_fLow;
    s >> bar.m_fClose;
    s >> bar.m_fVolume;
    s >> bar.m_fAmount;
    s >> bar.m_wAdvance;
    s >> bar.m_wDecline;
    s >> bar.amount;
    s >> bar.settle;
    return s;
}

QDataStream& operator>>(QDataStream& s, Bar& bar)
{
    s >> bar.time;
    s >> bar.open;
    s >> bar.high;
    s >> bar.low;
    s >> bar.close;
    s >> bar.tick_volume;
    s >> bar.volume;
    return s;
}

QDataStream& operator<<(QDataStream& s, const Bar& bar)
{
    s << bar.time;
    s << bar.open;
    s << bar.high;
    s << bar.low;
    s << bar.close;
    s << bar.tick_volume;
    s << bar.volume;
    return s;
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
