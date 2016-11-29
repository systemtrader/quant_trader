#ifndef BAR_SERIES_H
#define BAR_SERIES_H


class BarSeries
{
protected:
    const int m_size;

public:
    int *times;
    double *opens;
    double *highs;
    double *lows;
    double *closes;
    double *volumes;

    BarSeries(int size);
    ~BarSeries();

    inline int size() const;
};

#endif // BAR_SERIES_H
