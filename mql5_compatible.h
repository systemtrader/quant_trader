#ifndef MQL5_COMPATIBLE_H
#define MQL5_COMPATIBLE_H

#include <algorithm>
#include <cmath>

#include <QString>
#include <QVariant>

typedef QString string;
#define DoubleToString(x, y) QString::number(x, 'f', y)

#ifdef MQL5_PRINT_SUPPORT
#include <QDebug>
inline
void Print(QVariant arg1,      QVariant arg2 = "", QVariant arg3 = "",
           QVariant arg4 = "", QVariant arg5 = "", QVariant arg6 = "") {
    qDebug() << arg1 << arg2 << arg3 << arg4 << arg5 << arg6;
}
#else
inline
void Print(...) {}
#endif

// The predefined Variables
#define _Digits 1   // no use
#define _Point
#define _LastError
#define _Period
#define _RandomSeed
#define _StopFlag
#define _Symbol
#define _UninitReason

// State Checking
#define IsStopped() (false)

inline
double MathMin(double x, double y)
{
#ifndef _MSC_VER
    return fmin(x, y);
#else
    return std::min(x, y);
#endif
}

inline
double MathMax(double x, double y)
{
#ifndef _MSC_VER
    return fmax(x, y);
#else
    return std::max(x, y);
#endif
}

template<typename T>
class _TimeSeries {
protected:
    bool is_time_series;

public:
    _TimeSeries(bool is_time_series) :
        is_time_series(is_time_series) {
    }

    virtual ~_TimeSeries() {
        //
    }

    void setAsSeries(bool is_series) {
        is_time_series = is_series;
    }

    bool getAsSeries() {
        return is_time_series;
    }

    virtual const T& operator[](int i) const = 0;
    virtual T& operator[](int i) = 0;
    virtual void initialize(const T& value) = 0;
    virtual int resize(int new_size, int reserve_size = 0) = 0;
};

#include <QVector>

template<typename T>
class Mql5DynamicArray : public _TimeSeries<T> {
    QVector<T> * const data;
    const bool is_data_owner;

public:
    Mql5DynamicArray() :
        _TimeSeries<T>(false),
        data(new QVector<T>()),
        is_data_owner(true) {
        data->reserve(8192);  // TODO need adjust?
    }

    Mql5DynamicArray(int size) :
        _TimeSeries<T>(false),
        data(new QVector<T>(size)),   // unnecessary?
        is_data_owner(true) {
    }

    Mql5DynamicArray(const Mql5DynamicArray& other) :
        _TimeSeries<T>(other.is_time_series),
        data(other.data),
        is_data_owner(false) {
    }

    Mql5DynamicArray(QVector<T> *data) :
        _TimeSeries<T>(false),
        data(data),
        is_data_owner(false) {
    }

    ~Mql5DynamicArray() {
        if (is_data_owner) {
            delete data;
        }
    }

    operator QVector<T>&() const {
        return *data;
    }

    const T& operator[](int i) const {
        if (this->is_time_series) {
            return data->at(data->size() - 1 - i);
        } else {
            return data->at(i);
        }
    }

    T& operator[](int i) {
        if (this->is_time_series) {
            return data->operator[](data->size() - 1 - i);
        } else {
            return data->operator[](i);
        }
    }

    void initialize(const T& value) {
        data->fill(value);
    }

    // TODO need UT, compare result with MT5
    int resize(int new_size, int reserve_size = 0) {
        if (new_size > data->capacity() && reserve_size > 0) {
            data->reserve(new_size + reserve_size);
        }
        data->resize(new_size);
        return new_size;
    }
};

template<typename T>
inline bool ArrayGetAsSeries(
   const _TimeSeries<T>&  array    // array for checking
)
{
    return array.getAsSeries();
}

template<typename T>
inline bool  ArraySetAsSeries(
   const _TimeSeries<T>&  array,    // array by reference
   bool                        flag      // true denotes reverse order of indexing
)
{
    array.setAsSeries(flag);
    return true;
}

template<typename T, typename V>
inline void ArrayInitialize(
   _TimeSeries<T>&    array,   // initialized array
   const V&                value    // value that will be set
)
{
    array.initialize(static_cast<T>(value));
}

template<typename T>
inline int ArrayResize(
   _TimeSeries<T>&    array,              // array passed by reference
   int                     new_size,           // new array size
   int                     reserve_size=0      // reserve size value (excess)
)
{
    return array.resize(new_size, reserve_size);
}

#endif // MQL5_COMPATIBLE_H

