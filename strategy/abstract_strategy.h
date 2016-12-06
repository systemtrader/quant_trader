#ifndef ABSTRACT_STRATEGY_H
#define ABSTRACT_STRATEGY_H

#include <QObject>

class Bar;
class AbstractIndicator;
class QSettings;

class AbstractStrategy : public QObject
{
    Q_OBJECT
protected:
    const QString stratety_id;
    const QString instrument;
    const QString time_frame_str;

    QList<AbstractIndicator*> indicators;
    QList<Bar> *barList;
    Bar *lastBar;

    QSettings *result;
    int position;
    double tp_price;
    double sl_price;

    bool isNewBar();
    void resetPosition();
    void saveResult();

public:
    explicit AbstractStrategy(const QString& id, const QString& instrumentID, const QString& time_frame, QObject *parent = 0);
    ~AbstractStrategy();

    int getPosition() {
        return position;
    }

    // Should call setBarList after setParameter
    void setBarList(QList<Bar> *list, Bar &last) {
        barList = list;
        lastBar = &last;
    }

    // Inherit from AbstractStrategy and overwite following virtual functions
    virtual void setParameter(const QVariant& param1, const QVariant& param2, const QVariant& param3,
                              const QVariant& param4, const QVariant& param5, const QVariant& param6,
                              const QVariant& param7, const QVariant& param8, const QVariant& param9) = 0;
    virtual void onNewTick(int volume, double turnover, double openInterest, int time, double lastPrice);
    virtual void onNewBar() = 0;
    virtual void checkTPSL(double price);

signals:

public slots:
};

#endif // ABSTRACT_STRATEGY_H
