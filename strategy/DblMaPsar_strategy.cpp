#include <QMetaEnum>
#include <QDebug>

#include "DblMaPsar_strategy.h"
#include "../bar.h"
#include "../quant_trader.h"

extern int MA_METHOD_enumIdx;
extern int APPLIED_PRICE_enumIdx;

DblMaPsarStrategy::DblMaPsarStrategy(const QString &id, const QString& instrumentID, const QString& time_frame, QObject *parent) :
    AbstractStrategy(id, instrumentID, time_frame, parent)
{
    //
}

void DblMaPsarStrategy::setParameter(const QVariant& param1, const QVariant& param2, const QVariant& param3,
                                     const QVariant& param4, const QVariant& param5, const QVariant& param6,
                                     const QVariant& /*7*/ , const QVariant& /*8*/ , const QVariant& /*9*/)
{
    int fastPeriod = param1.toInt();
    int slowPeriod = param2.toInt();

    int ma_method_value = MA::staticMetaObject.enumerator(MA_METHOD_enumIdx).keyToValue(param3.toString().trimmed().toLatin1().data());
    if (ma_method_value < 0) {
        qDebug() << "Parameter3 ma_method error!";
    }
    int applied_price_value = MQL5IndicatorOnSingleDataBuffer::staticMetaObject.enumerator(APPLIED_PRICE_enumIdx).keyToValue(param4.toString().trimmed().toLatin1().data());
    if (applied_price_value < 0) {
        qDebug() << "Parameter4 applied_price error!";
    }

    double SARStep = param5.toDouble();
    double SARMaximum = param6.toDouble();

    setParameter(fastPeriod, slowPeriod, static_cast<MA::ENUM_MA_METHOD>(ma_method_value), static_cast<MQL5IndicatorOnSingleDataBuffer::ENUM_APPLIED_PRICE>(applied_price_value), SARStep, SARMaximum);
}

void DblMaPsarStrategy::setParameter(int fastPeriod, int slowPeriod, MA::ENUM_MA_METHOD ma_method, MQL5IndicatorOnSingleDataBuffer::ENUM_APPLIED_PRICE applied_price, double SARStep, double SARMaximum)
{
    qDebug() << "fastPeriod = " << fastPeriod << ", slowPeriod = " << slowPeriod << ", ma_method = " << ma_method << ", applied_price = " << applied_price << ", SARStep = " << SARStep << ", SARMaximum = " << SARMaximum;

    fast_ma = (MQL5Indicator*)QuantTrader::instance->registerIndicator(instrument, time_frame_str, "MA", fastPeriod, 0, ma_method, applied_price);
    slow_ma = (MQL5Indicator*)QuantTrader::instance->registerIndicator(instrument, time_frame_str, "MA", slowPeriod, 0, ma_method, applied_price);
    psar = (MQL5Indicator*)QuantTrader::instance->registerIndicator(instrument, time_frame_str, "ParabolicSAR", SARStep, SARMaximum);

    indicators.append(fast_ma);
    indicators.append(slow_ma);
    indicators.append(psar);
}

void DblMaPsarStrategy::onNewBar()
{
    IndicatorBuffer<double> fast_ma_buf = fast_ma->getBufferByIndex(0);
    IndicatorBuffer<double> slow_ma_buf = slow_ma->getBufferByIndex(0);
    IndicatorBuffer<double> psar_buf    = psar->getBufferByIndex(0);

    ArraySetAsSeries(fast_ma_buf, true);
    ArraySetAsSeries(slow_ma_buf, true);
    ArraySetAsSeries(psar_buf, true);

    _ListProxy<Bar> bars(barList, lastBar);

    if (fast_ma_buf[1] > slow_ma_buf[1] && fast_ma_buf[2] <= slow_ma_buf[2]) {
        if (psar_buf[1] < bars[1].low) {
            position = 1;
        }
    }

    if (fast_ma_buf[1] < slow_ma_buf[1] && fast_ma_buf[2] >= slow_ma_buf[2]) {
        if (psar_buf[1] > bars[1].high) {
            position = -1;
        }
    }
}
