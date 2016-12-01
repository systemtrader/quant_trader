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
                     const Mql5DynamicArray<int>& time,         // Time
                     const Mql5DynamicArray<double>& open,      // Open
                     const Mql5DynamicArray<double>& high,      // High
                     const Mql5DynamicArray<double>& low,       // Low
                     const Mql5DynamicArray<double>& close,     // Close
                     const Mql5DynamicArray<long>& tick_volume, // Tick Volume
                     const Mql5DynamicArray<long>& volume,      // Real Volume
                     const Mql5DynamicArray<int>& spread        // Spread
                     );
    double GetHigh(int nPosition,int nStartPeriod,const Mql5DynamicArray<double> &HiData);
    double GetLow(int nPosition,int nStartPeriod,const Mql5DynamicArray<double> &LoData);

signals:

public slots:
};

#endif // PARABOLICSAR_H
