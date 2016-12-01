#ifndef PARABOLICSAR_H
#define PARABOLICSAR_H

#include <QObject>

#include "mql5_indicator.h"

class ParabolicSAR : public QObject, public MQL5Indicator
{
    Q_OBJECT
    Q_CLASSINFO("indicator_buffers", "3")

public:
    explicit ParabolicSAR(double step = 0.02, double maximum = 0.2, QObject *parent = 0);
    ~ParabolicSAR() {}

protected:
    double InpSARStep;
    double InpSARMaximum;

    IndicatorBuffer<double> ExtSARBuffer;
    IndicatorBuffer<double> ExtEPBuffer;
    IndicatorBuffer<double> ExtAFBuffer;

    int    ExtLastRevPos;
    bool   ExtDirectionLong;
    double ExtSarStep;
    double ExtSarMaximum;

    void OnInit();
    int OnCalculate (const int rates_total,                     // size of input time series
                     const int prev_calculated,                 // bars handled in previous call
                     const _TimeSeries<uint>& time,             // Time
                     const _TimeSeries<double>& open,           // Open
                     const _TimeSeries<double>& high,           // High
                     const _TimeSeries<double>& low,            // Low
                     const _TimeSeries<double>& close,          // Close
                     const _TimeSeries<long>& tick_volume,      // Tick Volume
                     const _TimeSeries<long>& volume,           // Real Volume
                     const _TimeSeries<int>& spread             // Spread
                     );
    double GetHigh(int nPosition,int nStartPeriod,const _TimeSeries<double> &HiData);
    double GetLow(int nPosition,int nStartPeriod,const _TimeSeries<double> &LoData);

signals:

public slots:
};

#endif // PARABOLICSAR_H
