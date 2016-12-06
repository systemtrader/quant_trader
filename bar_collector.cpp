#include <QDataStream>
#include <QDebug>

#include "bar_collector.h"

static QDataStream& operator<<(QDataStream& s, const Bar& bar)
{
    s << bar.time;
    s << bar.open;
    s << bar.high;
    s << bar.low;
    s << bar.close;
    s << bar.volume;
    return s;
}

BarCollector::BarCollector(const QString& instrumentID, TimeFrames time_frame_flags, QObject *parent) :
    QObject(parent),
    instrument(instrumentID),
    time_frame_flags(time_frame_flags | MIN1)
{
    new_bar_open = true;
    current_bar.init();
}

BarCollector::~BarCollector()
{
    saveBars();
}

void BarCollector::onNew1MinBar()
{
    qDebug() << "onNew1MinBar()" << "\t" << current_bar.time;
    one_min_bars.append(current_bar);
    new_bar_open = true;
    current_bar.init();
}

void BarCollector::saveBars()
{
    // TODO
}

void BarCollector::onNewTick(int volume, double turnover, double openInterest, int time, double lastPrice)
{
    if (current_bar.open > 0.0f) {
        if ((time >> 8) > (current_bar.time >> 8)) {
            onNew1MinBar();
        }
    }

    if (new_bar_open) {
        current_bar.open = lastPrice;
        // TODO convert time value to time_t format
        current_bar.time = time & 0xffff00;
        new_bar_open = false;
    }

    if (lastPrice > current_bar.high) {
        current_bar.high = lastPrice;
    }

    if (lastPrice < current_bar.low) {
        current_bar.low = lastPrice;
    }

    current_bar.close = lastPrice;
}
