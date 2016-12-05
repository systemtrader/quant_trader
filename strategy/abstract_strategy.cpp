#include <QMetaEnum>

#include "abstract_strategy.h"
#include "indicator/abstract_indicator.h"

AbstractStrategy::AbstractStrategy(const QString& instr, const QString& time_frame, QObject *parent) :
    QObject(parent),
    instrument(instr),
    time_frame_str(time_frame),
    position(0),
    tp_price(-1.0),
    sl_price(-1.0)
{
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

void AbstractStrategy::setBarList(QList<Bar> *list)
{
    barlist = list;
    foreach (AbstractIndicator* indicator, indicators) {
        indicator->setBarList(list);
    }
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
