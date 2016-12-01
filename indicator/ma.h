#ifndef MA_H
#define MA_H

#include <QObject>

#include "mql5_indicator.h"

class MA : public QObject, public MQL5IndicatorOnSingleDataBuffer
{
    Q_OBJECT
    Q_CLASSINFO("indicator_buffers", "1")
    Q_ENUMS(ENUM_MA_METHOD)
public:
    enum ENUM_MA_METHOD {
        MODE_SMA,   // Simple averaging
        MODE_EMA,   // Exponential averaging
        MODE_SMMA,  // Smoothed averaging
        MODE_LWMA,  // Linear-weighted averaging
    };

    explicit MA(int period, int shift, ENUM_MA_METHOD ma_method, MQL5IndicatorOnSingleDataBuffer::ENUM_APPLIED_PRICE applied_price = PRICE_CLOSE, QObject *parent = 0);
    ~MA() {}

protected:
    int            InpMAPeriod;     // Period
    int            InpMAShift;      // Shift
    ENUM_MA_METHOD InpMAMethod;     // Method

    IndicatorBuffer<double> ExtLineBuffer;

    void CalculateSimpleMA(int rates_total,int prev_calculated,int begin,const _TimeSeries<double> &price);
    void CalculateEMA(int rates_total,int prev_calculated,int begin,const _TimeSeries<double> &price);
    void CalculateLWMA(int rates_total,int prev_calculated,int begin,const _TimeSeries<double> &price);
    void CalculateSmoothedMA(int rates_total,int prev_calculated,int begin,const _TimeSeries<double> &price);

    void OnInit();
    int OnCalculate (const int rates_total,                     // size of the price[] array
                     const int prev_calculated,                 // bars handled on a previous call
                     const int begin,                           // where the significant data start from
                     const _TimeSeries<double>& price           // array to calculate
                     );

signals:

public slots:
};

#endif // MA_H
