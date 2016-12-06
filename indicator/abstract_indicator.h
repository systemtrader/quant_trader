#ifndef ABSTRACT_INDICATOR_H
#define ABSTRACT_INDICATOR_H

#include <QObject>

class Bar;

class AbstractIndicator
{
    Q_GADGET
protected:
    QList<Bar> *barList;
    Bar *lastBar;

public:
    explicit AbstractIndicator();
    virtual ~AbstractIndicator();

    virtual void setBarList(QList<Bar> *list, Bar &last);
    virtual void update() = 0;
};

#endif // ABSTRACT_INDICATOR_H
