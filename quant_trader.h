#ifndef QUANT_TRADER_H
#define QUANT_TRADER_H

#include <QMultiMap>

#include "market_watcher_interface.h"
#include "ctp_executer_interface.h"

class Bar;
class BarCollector;
class AbstractIndicator;
class AbstractStrategy;

class QuantTrader : public QObject
{
    Q_OBJECT
protected:
    org::ctp::market_watcher *pWatcher;
    org::ctp::ctp_executer *pExecuter;

    // Following QString keys stands for instument names
    QMap<QString, BarCollector*> collector_map;
    QMap<QString, QMap<int, QList<Bar>>> bars_map;
    QMultiMap<QString, AbstractIndicator*> indicator_map;
    QMultiMap<QString, AbstractStrategy*> strategy_map;
    QMap<QString, int> position_map;

    QString kt_export_dir;
    void loadQuantTraderSettings();
    void loadTradeStrategySettings();
    QList<Bar>* getBars(const QString &instrumentID, const QString &time_frame_str);

public:
    explicit QuantTrader(QObject *parent = 0);
    ~QuantTrader();

    static QuantTrader *instance;
    AbstractIndicator* registerIndicator(const QString &instrumentID, const QString &time_frame_str, QString indicator_name, ...);

signals:

private slots:
    void onNewTick(int volume, double turnover, double openInterest, uint time, double lastPrice, const QString &instrumentID);
    void onNewBar(const QString &instrumentID, int time_frame, const Bar& bar);
};

#endif // QUANT_TRADER_H
