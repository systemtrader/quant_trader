#ifndef MQL5_INDICATOR_H
#define MQL5_INDICATOR_H

#include "abstract_indicator.h"
#include "mql5_compatible.h"

enum ENUM_INDEXBUFFER_TYPE {
    INDICATOR_DATA,
    INDICATOR_COLOR_INDEX,
    INDICATOR_CALCULATIONS,
};

enum ENUM_PLOT_PROPERTY_INTEGER {
    PLOT_ARROW,
    PLOT_ARROW_SHIFT,
    PLOT_DRAW_BEGIN,
    PLOT_DRAW_TYPE,
    PLOT_SHOW_DATA,
    PLOT_SHIFT,
    PLOT_LINE_STYLE,
    PLOT_LINE_WIDTH,
    PLOT_COLOR_INDEXES,
    PLOT_LINE_COLOR,
};

enum ENUM_CUSTOMIND_PROPERTY_INTEGER {
    INDICATOR_DIGITS,
    INDICATOR_HEIGHT,
    INDICATOR_LEVELS,
    INDICATOR_LEVELCOLOR,
    INDICATOR_LEVELSTYLE,
    INDICATOR_LEVELWIDTH,
};

enum ENUM_PLOT_PROPERTY_STRING {
    PLOT_LABEL,
};

enum ENUM_CUSTOMIND_PROPERTY_STRING {
    INDICATOR_SHORTNAME,
    INDICATOR_LEVELTEXT,
};

enum ENUM_PLOT_PROPERTY_DOUBLE {
    PLOT_EMPTY_VALUE,
};

inline bool IndicatorSetDouble (...) { return true; }
inline bool IndicatorSetInteger(...) { return true; }
inline bool IndicatorSetString (...) { return true; }
inline bool PlotIndexSetDouble (...) { return true; }
inline bool PlotIndexSetInteger(...) { return true; }
inline bool PlotIndexSetString (...) { return true; }
inline int  PlotIndexGetInteger(...) { return 0; }

template<typename T>
class IndicatorBuffer : public _VectorProxy<T> {
public:
    IndicatorBuffer() :
        _VectorProxy<T>() {
        data->reserve(8192);  // TODO need 16K or more?
    }

    IndicatorBuffer(const _VectorProxy<T>& other) :
        _VectorProxy<T>(other) {
    }

    ~IndicatorBuffer() {
    }

    void resize(int new_size) {
        data->resize(new_size);
    }
};

class MQL5Indicator : public AbstractIndicator
{
    Q_GADGET
public:
    explicit MQL5Indicator(int indicator_buffers);
    ~MQL5Indicator() {}

    const IndicatorBuffer<double>& getBufferByIndex(const int index = 0) const {
        return *(indicator_buffers[index]);
    }

    const IndicatorBuffer<double>& operator[](const int index) const {
        return *(indicator_buffers[index]);
    }

protected:
    int rates_total;
    int prev_calculated;
    QVector<IndicatorBuffer<double>*> indicator_buffers;

    void update();

    virtual void OnInit() = 0;
    virtual void preCalculate();

    void SetIndexBuffer(
        int                       index,                        // buffer index
        IndicatorBuffer<double> & buffer,                       // array
        ENUM_INDEXBUFFER_TYPE     data_type = INDICATOR_DATA    // what will be stored
    );

    virtual int OnCalculate (const int rates_total,                     // size of input time series
                             const int prev_calculated,                 // bars handled in previous call
                             const Mql5DynamicArray<int>& time,         // Time
                             const Mql5DynamicArray<double>& open,      // Open
                             const Mql5DynamicArray<double>& high,      // High
                             const Mql5DynamicArray<double>& low,       // Low
                             const Mql5DynamicArray<double>& close,     // Close
                             const Mql5DynamicArray<long>& tick_volume, // Tick Volume
                             const Mql5DynamicArray<long>& volume,      // Real Volume
                             const Mql5DynamicArray<int>& spread        // Spread
                             ) = 0;
};

typedef double (* SIMPLIFY_PRICE)(double, double, double, double);

class MQL5IndicatorOnSingleDataBuffer : public MQL5Indicator {
    Q_GADGET
    Q_ENUMS(ENUM_APPLIED_PRICE)
public:
    enum ENUM_APPLIED_PRICE {
        PRICE_CLOSE,    // Close price
        PRICE_OPEN,     // Open price
        PRICE_HIGH,     // The maximum price for the period
        PRICE_LOW,      // The minimum price for the period
        PRICE_MEDIAN,   // Median price, (high + low)/2
        PRICE_TYPICAL,  // Typical price, (high + low + close)/3
        PRICE_WEIGHTED, // Average price, (high + low + close + close)/4
    };

    explicit MQL5IndicatorOnSingleDataBuffer(int indicator_buffers, ENUM_APPLIED_PRICE applied_price);
    ~MQL5IndicatorOnSingleDataBuffer() {}

protected:
    const ENUM_APPLIED_PRICE applied_price;
    SIMPLIFY_PRICE simplify_func;
    IndicatorBuffer<double> applied_price_buffer;

    void preCalculate();
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

    virtual int OnCalculate (const int rates_total,                     // size of the price[] array
                             const int prev_calculated,                 // bars handled on a previous call
                             const int begin,                           // where the significant data start from
                             const _TimeSeries<double>& price           // array to calculate
                             ) = 0;
};

#endif // MQL5_INDICATOR_H
