#ifndef QUANT_TRADER_H
#define QUANT_TRADER_H

#include <QMultiMap>

#include "market_watcher_interface.h"
#include "ctp_executer_interface.h"

class BarCollector;
class AbstractStrategy;
class BarSeries;

class QuantTrader : public QObject
{
    Q_OBJECT
protected:
    org::ctp::market_watcher *pWatcher;
    org::ctp::ctp_executer *pExecuter;

    // Following QString keys stands for instument names
    QMap<QString, BarCollector*> collector_map;
    QMap<QString, QMap<int, BarSeries*> > history_barseries;
    // QMultiMap<QString, AbstractIndicator*> indicator_map;  // managed by strategy
    QMultiMap<QString, AbstractStrategy*> strategy_map;

    void loadQuantTraderSettings();
    void loadTradeStrategySettings();

public:
    explicit QuantTrader(QObject *parent = 0);
    ~QuantTrader();

signals:

public slots:
    void onNewTick(int volume, double turnover, double openInterest, int time, double lastPrice, const QString &instrumentID);
};

#endif // QUANT_TRADER_H
