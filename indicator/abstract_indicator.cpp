#include "abstract_indicator.h"

AbstractIndicator::AbstractIndicator()
{
    //
}

AbstractIndicator::~AbstractIndicator()
{
    //
}

void AbstractIndicator::setBarList(QList<Bar> *list)
{
    barlist = list;
}
