#include "bar_series.h"

BarSeries::BarSeries(int size) :
    m_size(size)
{
    times   = new int[size];
    opens   = new double[size];
    highs   = new double[size];
    lows    = new double[size];
    closes  = new double[size];
    volumes = new double[size];
}

BarSeries::~BarSeries()
{
    delete times;
    delete opens;
    delete highs;
    delete lows;
    delete closes;
    delete volumes;
}

int BarSeries::size() const
{
    return m_size;
}
