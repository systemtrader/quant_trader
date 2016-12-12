#ifndef MQL5_INDICATOR_H
#define MQL5_INDICATOR_H

#include "abstract_indicator.h"
#include "../mql5_compatible.h"

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
        this->data->reserve(8192);  // TODO need 16K or more?
    }

    IndicatorBuffer(const _VectorProxy<T>& other) :
        _VectorProxy<T>(other) {
    }

    ~IndicatorBuffer() {
    }

    void resize(int new_size) {
        this->data->resize(new_size);
    }
};

template<typename M, typename T>
class RemapListMember : public _TimeSeries<T> {
protected:
    QList<M>* mlist;
    T M::* pm;
    M* lastM;

public:
    RemapListMember(QList<M> *list, T M::* pmt, M *m) :
        _TimeSeries<T>(false),
        mlist(list),
        pm(pmt),
        lastM(m) {
    }

    ~RemapListMember() {}

    const T& operator[](int i) const {
        const int size = mlist->size();

        if (this->is_time_series) {
            if (i == 0) {
                return lastM->*pm;
            } else {
                return mlist->at(size - i).*pm;
            }
        } else {
            if (i == size) {
                return lastM->*pm;
            } else {
                return mlist->at(i).*pm;
            }
        }
    }
};

class Bar;

class MQL5Indicator : public AbstractIndicator
{
public:
    explicit MQL5Indicator(int indicator_buffers);
    ~MQL5Indicator();

    virtual void OnInit() = 0;

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
    bool remaped;   // if true, delete following pointers in deconstructor
    _TimeSeries<uint> *time;
    _TimeSeries<double> *open;
    _TimeSeries<double> *high;
    _TimeSeries<double> *low;
    _TimeSeries<double> *close;
    _TimeSeries<qint64> *tick_volume;
    _TimeSeries<qint64> *volume;
    _TimeSeries<int> *spread;

    void setBarList(QList<Bar> *list, Bar *last) override;
    void update() override;

    virtual void preCalculate();

    void SetIndexBuffer(
        int                       index,                        // buffer index
        IndicatorBuffer<double> & buffer,                       // array
        ENUM_INDEXBUFFER_TYPE     data_type = INDICATOR_DATA    // what will be stored
    );

    virtual
    int OnCalculate (const int rates_total,                     // size of input time series
                     const int prev_calculated,                 // bars handled in previous call
                     const _TimeSeries<uint>& time,             // Time
                     const _TimeSeries<double>& open,           // Open
                     const _TimeSeries<double>& high,           // High
                     const _TimeSeries<double>& low,            // Low
                     const _TimeSeries<double>& close,          // Close
                     const _TimeSeries<qint64>& tick_volume,    // Tick Volume
                     const _TimeSeries<qint64>& volume,         // Real Volume
                     const _TimeSeries<int>& spread             // Spread
                     ) = 0;
};

typedef double (* SIMPLIFY_PRICE)(double, double, double, double);

class MQL5IndicatorOnSingleDataBuffer : public QObject, public MQL5Indicator {
    Q_OBJECT
    Q_PROPERTY(ENUM_APPLIED_PRICE applyTo READ getAppliedTo CONSTANT)
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

    explicit MQL5IndicatorOnSingleDataBuffer(int indicator_buffers, ENUM_APPLIED_PRICE applyTo, QObject *parent = 0);
    ~MQL5IndicatorOnSingleDataBuffer() {}

    ENUM_APPLIED_PRICE getAppliedTo() const { return applied; }

protected:
    const ENUM_APPLIED_PRICE applied;
    SIMPLIFY_PRICE simplify_func;
    IndicatorBuffer<double> applied_price_buffer;

    void preCalculate() override;
    int OnCalculate (const int rates_total,                     // size of input time series
                     const int prev_calculated,                 // bars handled in previous call
                     const _TimeSeries<uint>& time,             // Time
                     const _TimeSeries<double>& open,           // Open
                     const _TimeSeries<double>& high,           // High
                     const _TimeSeries<double>& low,            // Low
                     const _TimeSeries<double>& close,          // Close
                     const _TimeSeries<qint64>& tick_volume,    // Tick Volume
                     const _TimeSeries<qint64>& volume,         // Real Volume
                     const _TimeSeries<int>& spread             // Spread
                     ) override;

    virtual
    int OnCalculate (const int rates_total,                     // size of the price[] array
                     const int prev_calculated,                 // bars handled on a previous call
                     const int begin,                           // where the significant data start from
                     const _TimeSeries<double>& price           // array to calculate
                     ) = 0;
};

#endif // MQL5_INDICATOR_H
