#ifndef MQL5_COMPATIBLE_H
#define MQL5_COMPATIBLE_H

#include <algorithm>
#include <cmath>

#include <QString>
#include <QVariant>

#define EMPTY_VALUE     DBL_MAX
#define INVALID_HANDLE  -1

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

    bool getAsSeries() const {
        return is_time_series;
    }

    virtual const T& operator[](int i) const = 0;
};

#include <QVector>

template<typename T>
class _ListProxy : public _TimeSeries<T> {
protected:
    QList<T> * const data;
    T * lastT;
public:
    _ListProxy(QList<T> *list, T *last) :
        _TimeSeries<T>(true),   // time series as default
        data(list),
        lastT(last) {
    }
    const T& operator[](int i) const {
        const int size = data->size();

        if (this->is_time_series) {
            if (i == 0) {
                return *lastT;
            } else {
                return data->at(size - i);
            }
        } else {
            if (i == size) {
                return *lastT;
            } else {
                return data->at(i);
            }
        }
    }
};

template<typename T>
class _VectorProxy : public _TimeSeries<T> {
protected:
    QVector<T> * const data;
    const bool is_data_owner;

public:
    _VectorProxy() :
        _TimeSeries<T>(false),
        data(new QVector<T>()),
        is_data_owner(true) {
    }

    _VectorProxy(const _VectorProxy<T>& other) :
        _TimeSeries<T>(other.is_time_series),
        data(other.data),
        is_data_owner(false) {
    }

    ~_VectorProxy() {
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
};

template<typename T>
class Mql5DynamicArray : public _VectorProxy<T> {
public:
    Mql5DynamicArray() :
        _VectorProxy<T>() {
    }

    Mql5DynamicArray(const _VectorProxy<T>& other) :
        _VectorProxy<T>(other) {
    }

    ~Mql5DynamicArray() {
    }

    int resize(int new_size, int reserve_size) {
        if (new_size > this->data->capacity() && reserve_size > 0) {
            this->data->reserve(new_size + reserve_size);
        }
        this->data->resize(new_size);
        return new_size;
    }
};

template<typename T>
inline bool ArrayGetAsSeries(
   _TimeSeries<T>&         array            // array for checking
)
{
    return array.getAsSeries();
}

template<typename T>
inline bool ArraySetAsSeries(
   _TimeSeries<T>&         array,           // array by reference
   bool                    flag             // true denotes reverse order of indexing
)
{
    array.setAsSeries(flag);
    return true;
}

template<typename T, typename V>
inline void ArrayInitialize(
   _VectorProxy<T>&        array,           // initialized array
   const V&                value            // value that will be set
)
{
    array.initialize(static_cast<T>(value));
}

// TODO need UT, should compare result with MT5
template<typename T>
inline int ArrayResize(
   Mql5DynamicArray<T>&    array,           // array passed by reference
   int                     new_size,        // new array size
   int                     reserve_size=0   // reserve size value (excess)
)
{
    return array.resize(new_size, reserve_size);
}

#endif // MQL5_COMPATIBLE_H

