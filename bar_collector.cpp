#include <QDataStream>
#include <QDebug>
#include <QMetaEnum>
#include <QDateTime>
#include <QFile>

#include "bar.h"
#include "bar_collector.h"

extern int barCollector_enumIdx;
QString BarCollector::collector_dir;

static QDataStream& operator<<(QDataStream& s, const Bar& bar)
{
    s << bar.time;
    s << bar.tick_volume;
    s << bar.open;
    s << bar.high;
    s << bar.low;
    s << bar.close;
    s << bar.volume;
    return s;
}

BarCollector::BarCollector(const QString& instrumentID, const TimeFrames &time_frame_flags, QObject *parent) :
    QObject(parent),
    instrument(instrumentID)
{
    QSet<TimeFrame> key_set;
    uint mask = 0x80;
    while (mask != 0x00) {
        int result = time_frame_flags & mask;
        if (result != 0) {
            key_set.insert(static_cast<TimeFrame>(result));
        }
        mask >>= 1;
    }
    key_set.insert(TimeFrame::MIN1);
    keys = key_set.toList();
    foreach (const auto key, keys) {
        bar_list_map.insert(key, QList<Bar>());
        current_bar_map.insert(key, Bar());
    }
}

BarCollector::~BarCollector()
{
    saveBars();
}

Bar* BarCollector::getCurrentBar(const QString &time_frame_str)
{
    int time_frame_value = BarCollector::staticMetaObject.enumerator(barCollector_enumIdx).keyToValue(time_frame_str.trimmed().toLatin1().data());
    TimeFrame time_frame = static_cast<BarCollector::TimeFrame>(time_frame_value);
    return &current_bar_map[time_frame];
}

void BarCollector::saveBars()
{
    foreach (const auto key, keys) {
        auto & barList = bar_list_map[key];
        QString time_frame_str = BarCollector::staticMetaObject.enumerator(barCollector_enumIdx).valueToKey(key);
        QString file_name = collector_dir + "/" + time_frame_str + "/" + instrument + "/" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz") + ".bar";
        QFile barFile(file_name);
        barFile.open(QFile::WriteOnly);
        QDataStream wstream(&barFile);
        wstream.setFloatingPointPrecision(QDataStream::DoublePrecision);
        wstream << barList;
        barList.clear();
    }
}

#define MIN_UNIT    60
#define HOUR_UNIT   3600

QMap<BarCollector::TimeFrame, int> createTimeTable() {
    QMap<BarCollector::TimeFrame, int> timeTable;
    timeTable.insert(BarCollector::MIN1, 1 * MIN_UNIT);
    timeTable.insert(BarCollector::MIN5, 5 * MIN_UNIT);
    timeTable.insert(BarCollector::MIN15, 15 * MIN_UNIT);
    timeTable.insert(BarCollector::MIN60, 1 * HOUR_UNIT);
    return timeTable;
}

const QMap<BarCollector::TimeFrame, int> g_time_table = createTimeTable();

void BarCollector::onNewTick(int volume, double turnover, double openInterest, uint time, double lastPrice)
{
    foreach (const auto key, keys) {
        Bar & bar = current_bar_map[key];
        const int time_unit = g_time_table[key];  // TODO optimize, use time_unit as key

        if (bar.tick_volume > 0) {
            if ((time / time_unit) > (bar.time / time_unit)) {
                bar_list_map[key].append(bar);
                emit collectedBar(instrument, key, bar);
                bar.init();
            }
        }

        if (bar.isNewBar()) {
            bar.open = lastPrice;
            // TODO convert time value to time_t format
            bar.time = time / time_unit * time_unit;
        }

        if (lastPrice > bar.high) {
            bar.high = lastPrice;
        }

        if (lastPrice < bar.low) {
            bar.low = lastPrice;
        }

        bar.close = lastPrice;
        bar.tick_volume ++;
    }
}
