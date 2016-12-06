#include "abstract_indicator.h"

AbstractIndicator::AbstractIndicator()
{
    //
}

AbstractIndicator::~AbstractIndicator()
{
    //
}

void AbstractIndicator::setBarList(QList<Bar> *list, Bar *last)
{
    barList = list;
    lastBar = last;
}
