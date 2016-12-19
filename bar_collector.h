#ifndef BAR_COLLECTOR_H
#define BAR_COLLECTOR_H

#include <QObject>
#include <QMap>

class Bar;

class BarCollector : public QObject
{
    Q_OBJECT
    Q_ENUMS(TimeFrame)
public:
    enum TimeFrame {
        MIN1  = 0x01,
        MIN3  = 0x02,	// SinYee only
        MIN5  = 0x04,
        MIN10 = 0x08,	// SinYee only
        MIN15 = 0x10,
        MIN30 = 0x20,
        MIN60 = 0x40,
        DAY   = 0x80,
    };
    Q_DECLARE_FLAGS(TimeFrames, TimeFrame)

    explicit BarCollector(const QString& instrumentID, const TimeFrames &time_frame_flags, QObject *parent = 0);
    ~BarCollector();

    static QString collector_dir;
    Bar *getCurrentBar(const QString &time_frame_str);
    bool onMarketData(uint time, double lastPrice, int volume);

protected:
    const QString instrument;
    QList<TimeFrame> keys;
    QMap<TimeFrame, QList<Bar>> bar_list_map;
    QMap<TimeFrame, Bar> current_bar_map;
    int lastVolume;

signals:
    void collectedBar(const QString& instrumentID, int time_frame, const Bar& bar);
public slots:
    void saveBars();
};

#endif // BAR_COLLECTOR_H
