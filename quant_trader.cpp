#include <QSettings>

#include "quant_trader.h"
#include "bar_collector.h"
#include "strategy/DblMaPsar_strategy.h"

static const int barCollector_enumIdx = BarCollector::staticMetaObject.indexOfEnumerator("TimeFrame");

QuantTrader::QuantTrader(QObject *parent) :
    QObject(parent)
{
    loadQuantTraderSettings();
    loadTradeStrategySettings();

    pExecuter = new org::ctp::ctp_executer("org.ctp.ctp_executer", "/ctp_executer", QDBusConnection::sessionBus(), this);
    pWatcher = new org::ctp::market_watcher("org.ctp.market_watcher", "/market_watcher", QDBusConnection::sessionBus(), this);
    connect(pWatcher, SIGNAL(newTick(int,double,double,int,double,QString)), this, SLOT(onNewTick(int,double,double,int,double,QString)));
}

void QuantTrader::loadQuantTraderSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ctp", "quant_trader");
    settings.beginGroup("Collector");
    QStringList instruments = settings.childKeys();

    foreach (const QString &str, instruments) {
        QString time_frame_string_with_dots = settings.value(str).toString();
        QStringList time_frame_stringlist = time_frame_string_with_dots.split('|');
        BarCollector::TimeFrames time_frame_flags;
        foreach (const QString &tf, time_frame_stringlist) {
            int time_frame_value = BarCollector::staticMetaObject.enumerator(barCollector_enumIdx).keyToValue(tf.trimmed().toLatin1().data());
            BarCollector::TimeFrame time_frame = static_cast<BarCollector::TimeFrame>(time_frame_value);
            time_frame_flags |= time_frame;
        }

        BarCollector *collector = new BarCollector(time_frame_flags, this);
        collector_map[str] = collector;
        qDebug() << str << ":\t" << time_frame_flags << "\t" << time_frame_stringlist;
    }
    settings.endGroup();
}

void QuantTrader::loadTradeStrategySettings()
{
    static QMap<QString, const QMetaObject*> meta_object_map;
    meta_object_map.insert("DblMaPsarStrategy", &DblMaPsarStrategy::staticMetaObject);
    // Register more strategies here

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ctp", "trade_strategy");
    QStringList groups = settings.childGroups();
    qDebug() << groups.size() << " stragegs in all.";

    foreach (const QString& group, groups) {
        settings.beginGroup(group);
        QString strategy_name = settings.value("strategy").toString();
        QString instrument_name = settings.value("instrument").toString();
        QString time_frame = settings.value("timeframe").toString();

        const QMetaObject* strategy_meta_object = meta_object_map.value(strategy_name);
        QObject *object = strategy_meta_object->newInstance(Q_ARG(QString, instrument_name), Q_ARG(QString, time_frame), Q_ARG(QObject*, this));
        if (object == NULL) {
            qDebug() << "Instantiating strategy " << group << " failed!";
            settings.endGroup();
            continue;
        }

        QVariant param1 = settings.value("param1");
        QVariant param2 = settings.value("param2");
        QVariant param3 = settings.value("param3");
        QVariant param4 = settings.value("param4");
        QVariant param5 = settings.value("param5");
        QVariant param6 = settings.value("param6");
        QVariant param7 = settings.value("param7");
        QVariant param8 = settings.value("param8");
        QVariant param9 = settings.value("param9");

        settings.endGroup();

        AbstractStrategy *strategy = qobject_cast<AbstractStrategy*>(object);
        if (strategy == NULL) {
            qDebug() << "Cast strategy " << group << " failed!";
            delete object;
            continue;
        }

        strategy->setParameter(param1, param2, param3, param4, param5, param6, param7, param8, param9);
        strategy_map.insert(instrument_name, strategy);
    }
}

QuantTrader::~QuantTrader()
{
}

void QuantTrader::onNewTick(int volume, double turnover, double openInterest, int time, double lastPrice, const QString &instrumentID)
{
    BarCollector *collector = collector_map.value(instrumentID);
    if (collector != NULL) {
        collector->onNewTick(volume, turnover, openInterest, time, lastPrice);
    }

    QList<AbstractStrategy *> strategies = strategy_map.values(instrumentID);
    foreach (AbstractStrategy * strategy, strategies) {
        strategy->onNewTick(volume, turnover, openInterest, time, lastPrice);
    }
}

