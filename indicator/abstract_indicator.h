#ifndef ABSTRACT_INDICATOR_H
#define ABSTRACT_INDICATOR_H

#include <QObject>

class AbstractIndicator
{
    Q_GADGET
public:
    explicit AbstractIndicator();
    virtual ~AbstractIndicator();

    virtual void update() = 0;
};

#endif // ABSTRACT_INDICATOR_H
