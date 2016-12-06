#include <QDataStream>
#include <QDebug>
#include <QMetaEnum>

#include "bar_collector.h"

const int barCollector_enumIdx = BarCollector::staticMetaObject.indexOfEnumerator("TimeFrame");

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
    instrument(instrumentID)
{
    uint mask = 0x80;
    while (mask != 0x00) {
        int result = time_frame_flags & mask;
        if (result != 0) {
            bar_list_map.insert(static_cast<TimeFrame>(result), QList<Bar>());
            current_bar_map.insert(static_cast<TimeFrame>(result), Bar());
        }
        mask >>= 1;
    }
    if (!bar_list_map.contains(TimeFrame::MIN1)) {
        bar_list_map.insert(TimeFrame::MIN1, QList<Bar>());
        current_bar_map.insert(TimeFrame::MIN1, Bar());
    }
}

BarCollector::~BarCollector()
{
    saveBars();
}

Bar* BarCollector::getCurrentBar(QString time_frame_str)
{
    int time_frame_value = BarCollector::staticMetaObject.enumerator(barCollector_enumIdx).keyToValue(time_frame_str.trimmed().toLatin1().data());
    TimeFrame time_frame = static_cast<BarCollector::TimeFrame>(time_frame_value);
    return &current_bar_map[time_frame];
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

    foreach (Bar & bar, current_bar_map) {
        if (bar.isNewBar()) {
            bar.open = lastPrice;
            // TODO convert time value to time_t format
            bar.time = time & 0xffff00;
        }

        if (lastPrice > bar.high) {
            bar.high = lastPrice;
        }

        if (lastPrice < bar.low) {
            bar.low = lastPrice;
        }

        bar.close = lastPrice;
    }
}
