#include <QMetaEnum>
#include <QSettings>
#include <QDateTime>
#include <QDebug>

#include "abstract_strategy.h"
#include "../bar.h"
#include "../indicator/abstract_indicator.h"

AbstractStrategy::AbstractStrategy(const QString &id, const QString& instrumentID, const QString& time_frame, QObject *parent) :
    QObject(parent),
    stratety_id(id),
    instrument(instrumentID),
    time_frame_str(time_frame)
{
    qDebug() << "id = " << id << ", instrumentID = " << instrumentID << ", time_frame = " << time_frame;

    result = new QSettings(QSettings::IniFormat, QSettings::UserScope, "ctp", "strategy_result");
    result->beginGroup(stratety_id);
    position = result->value("position", 0).toInt();
    tp_price = result->value("tp_price", -1.0).toDouble();
    sl_price = result->value("sl_price", -1.0).toDouble();

    lastCalcualtedBarTime = -1;
}

AbstractStrategy::~AbstractStrategy()
{
    result->endGroup();
    delete result;
}

inline bool AbstractStrategy::isNewBar() const
{
    return (lastBar->time != lastCalcualtedBarTime);
}

inline void AbstractStrategy::resetPosition()
{
    position = 0;
    tp_price = -1.0;
    sl_price = -1.0;
    saveResult();
}

inline void AbstractStrategy::saveResult()
{
    result->setValue("lastSave", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
    result->setValue("position", position);
    result->setValue("tp_price", tp_price);
    result->setValue("sl_price", sl_price);
}

void AbstractStrategy::checkTPSL(double price)
{
    if (tp_price > 0.0) {
        if (position > 0 && price > tp_price) {
            resetPosition();
        }
        if (position < 0 && price < tp_price) {
            resetPosition();
        }
    }

    if (sl_price > 0.0) {
        if (position > 0 && price < sl_price) {
            resetPosition();
        }
        if (position < 0 && price > sl_price) {
            resetPosition();
        }
    }
}

void AbstractStrategy::checkIfNewBar()
{
    if (isNewBar()) {
        foreach (auto* indicator, indicators) {
            indicator->update();
        }
        onNewBar();
        saveResult();
        lastCalcualtedBarTime = lastBar->time;
    }
}

void AbstractStrategy::onNewTick(uint time, double lastPrice)
{
    checkTPSL(lastPrice);
}
