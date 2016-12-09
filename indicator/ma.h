#ifndef MA_H
#define MA_H

#include "mql5_indicator.h"

class MA : public MQL5IndicatorOnSingleDataBuffer
{
    Q_OBJECT
    Q_CLASSINFO("parameter_number", "4")
    Q_PROPERTY(int MAPeriod READ getMAPeriod CONSTANT)
    Q_PROPERTY(int MAShift READ getMAShift CONSTANT)
    Q_PROPERTY(ENUM_MA_METHOD MAMethod READ getMAMethod CONSTANT)
    Q_ENUMS(ENUM_MA_METHOD)
public:
    enum ENUM_MA_METHOD {
        MODE_SMA,   // Simple averaging
        MODE_EMA,   // Exponential averaging
        MODE_SMMA,  // Smoothed averaging
        MODE_LWMA,  // Linear-weighted averaging
    };

    Q_INVOKABLE explicit MA(int MAPeriod, int MAShift, ENUM_MA_METHOD MAMethod, ENUM_APPLIED_PRICE applyTo = PRICE_CLOSE, QObject *parent = 0);
    ~MA() {}

    void OnInit() override;

    int getMAPeriod() const { return InpMAPeriod; }
    int getMAShift() const { return InpMAShift; }
    ENUM_MA_METHOD getMAMethod() const { return InpMAMethod; }

protected:
    const int            InpMAPeriod;     // Period
    const int            InpMAShift;      // Shift
    const ENUM_MA_METHOD InpMAMethod;     // Method

    IndicatorBuffer<double> ExtLineBuffer;

    void CalculateSimpleMA(int rates_total,int prev_calculated,int begin,const _TimeSeries<double> &price);
    void CalculateEMA(int rates_total,int prev_calculated,int begin,const _TimeSeries<double> &price);
    void CalculateLWMA(int rates_total,int prev_calculated,int begin,const _TimeSeries<double> &price);
    void CalculateSmoothedMA(int rates_total,int prev_calculated,int begin,const _TimeSeries<double> &price);

    int OnCalculate (const int rates_total,                     // size of the price[] array
                     const int prev_calculated,                 // bars handled on a previous call
                     const int begin,                           // where the significant data start from
                     const _TimeSeries<double>& price           // array to calculate
                     ) override;

signals:

public slots:
};

#endif // MA_H
