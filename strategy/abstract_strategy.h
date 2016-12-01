#ifndef ABSTRACT_STRATEGY_H
#define ABSTRACT_STRATEGY_H

#include "bar_collector.h"

class AbstractIndicator;

class AbstractStrategy : public QObject
{
    Q_OBJECT
    Q_ENUMS(BarCollector::TimeFrame)
protected:
    BarCollector::TimeFrame time_frame;
    QList<AbstractIndicator*> indicators;

    int position;
    double tp_price;
    double sl_price;

    bool isNewBar();

public:
    explicit AbstractStrategy(const QString& instrument, const QString& time_frame_str, QObject *parent = 0);
    ~AbstractStrategy();

    // Inherit from AbstractStrategy and overwite following functions
    virtual void setParameter(const QVariant& param1, const QVariant& param2, const QVariant& param3,
                              const QVariant& param4, const QVariant& param5, const QVariant& param6,
                              const QVariant& param7, const QVariant& param8, const QVariant& param9) = 0;
    virtual void onNewTick(int volume, double turnover, double openInterest, int time, double lastPrice);
    virtual void onNewBar() = 0;
    virtual void checkTPSL(double price);

signals:

public slots:
};

#endif // ABSTRACT_STRATEGY_H
