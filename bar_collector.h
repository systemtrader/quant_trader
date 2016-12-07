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

    explicit BarCollector(const QString& instrumentID, TimeFrames time_frame_flags, QObject *parent = 0);
    ~BarCollector();

    Bar* getCurrentBar(const QString &time_frame_str);

protected:
    const QString instrument;
    QList<TimeFrame> keys;
    QMap<TimeFrame, QList<Bar>> bar_list_map;
    QMap<TimeFrame, Bar> current_bar_map;

    void saveBars();

signals:
    //void newBar(const Bar& bar);
public slots:
    void onNewTick(int volume, double turnover, double openInterest, uint time, double lastPrice);
};

#endif // BAR_COLLECTOR_H
