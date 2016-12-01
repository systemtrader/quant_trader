#include <QMetaEnum>

#include "abstract_strategy.h"
#include "indicator/abstract_indicator.h"

static const int barCollector_enumIdx = BarCollector::staticMetaObject.indexOfEnumerator("TimeFrame");

AbstractStrategy::AbstractStrategy(const QString& instrument, const QString& time_frame_str, QObject *parent) :
    QObject(parent),
    position(0),
    tp_price(-1.0),
    sl_price(-1.0)
{
    int time_frame_value = BarCollector::staticMetaObject.enumerator(barCollector_enumIdx).keyToValue(time_frame_str.trimmed().toLatin1().data());
    this->time_frame = static_cast<BarCollector::TimeFrame>(time_frame_value);
}

AbstractStrategy::~AbstractStrategy()
{
    //
}

bool AbstractStrategy::isNewBar()
{
    bool is_new_bar = false;
    // TODO
    return is_new_bar;
}

void AbstractStrategy::onNewTick(int volume, double turnover, double openInterest, int time, double lastPrice)
{
    foreach (AbstractIndicator* indicator, indicators) {
        indicator->update();
    }

    if (isNewBar()) {
        onNewBar();
    }
    checkTPSL(lastPrice);
}

void AbstractStrategy::checkTPSL(double price)
{
    if (tp_price > 0.0) {
        if (position > 0 && price > tp_price) {
            position = 0;
        }
        if (position < 0 && price < tp_price) {
            position = 0;
        }
    }

    if (sl_price > 0.0) {
        if (position > 0 && price < sl_price) {
            position = 0;
        }
        if (position < 0 && price > sl_price) {
            position = 0;
        }
    }
}
