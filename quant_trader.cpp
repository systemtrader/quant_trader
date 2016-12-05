#include <QSettings>

#include "quant_trader.h"
#include "bar_collector.h"
#include "indicator/ma.h"
#include "indicator/parabolicsar.h"
#include "strategy/DblMaPsar_strategy.h"

static const int barCollector_enumIdx = BarCollector::staticMetaObject.indexOfEnumerator("TimeFrame");
QuantTrader* QuantTrader::instance;

QuantTrader::QuantTrader(QObject *parent) :
    QObject(parent)
{
    QuantTrader::instance = this;
    registerIndicator("cu1701", "MIN1", "MA");
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

static QDataStream& operator>>(QDataStream& s, KTExportBar& bar)
{
    s >> bar.m_time;
    s >> bar.m_fOpen;
    s >> bar.m_fHigh;
    s >> bar.m_fLow;
    s >> bar.m_fClose;
    s >> bar.m_fVolume;
    s >> bar.m_fAmount;
    s >> bar.m_wAdvance;
    s >> bar.m_wDecline;
    s >> bar.amount;
    s >> bar.settle;
    return s;
}

static QDataStream& operator>>(QDataStream& s, Bar& bar)
{
    s >> bar.time;
    s >> (quint32&)bar.tick_volume;
    s >> bar.open;
    s >> bar.high;
    s >> bar.low;
    s >> bar.close;
    s >> bar.volume;
    return s;
}

// 上海期货交易所                                   燃油, 线材
static const QStringList SQ = {"fu", "wr"};
// 上海期货交易所 (夜盘)              铜,   铝,   锌,   铅,   镍,   锡,   金,   银,螺纹钢,热轧卷板,沥青,天然橡胶
static const QStringList SY = {"cu", "al", "zn", "pb", "ni", "sn", "au", "ag", "rb", "hc", "bu", "ru"};
// 大连商品交易所                                  玉米, 玉米淀粉, 纤维板,  胶合板, 鸡蛋, 线型低密度聚乙烯, 聚氯乙烯, 聚丙烯
static const QStringList DL = {"c",  "cs", "fb", "bb", "jd", "l",  "v",  "pp"};
// 大连商品交易所  (夜盘)          黄大豆1号, 黄大豆2号, 豆粕, 大豆原油, 棕榈油, 冶金焦炭, 焦煤, 铁矿石
static const QStringList DY = {"a",  "b",  "m",  "y",  "p",  "j",  "jm", "i"};
// 郑州商品交易所
static const QStringList ZZ = {"jr", "lr", "pm", "ri", "rs", "sf", "sm", "wh"};
// 郑州商品交易所 (夜盘)
static const QStringList ZY = {"cf", "fg", "ma", "oi", "rm", "sr", "ta", "zc", "tc"};	// zc原来为tc
// 中金所
static const QStringList ZJ = {"ic", "if", "ih", "t",  "tf"};

#define String const QString&
static QString getSuffix(String instrument) {
    for (String instr :SQ) {
        if (instrument.toLower() == instr) {
            return ".SQ";
        }
    }
    for (String instr :SY) {
        if (instrument.toLower() == instr) {
            return ".SY";
        }
    }
    for (String instr :DL) {
        if (instrument.toLower() == instr) {
            return ".DL";
        }
    }
    for (String instr :DY) {
        if (instrument.toLower() == instr) {
            return ".DY";
        }
    }
    for (String instr :ZZ) {
        if (instrument.toLower() == instr) {
            return ".ZZ";
        }
    }
    for (String instr :ZY) {
        if (instrument.toLower() == instr) {
            return ".ZY";
        }
    }
    for (String instr :ZJ) {
        if (instrument.toLower() == instr) {
            return ".ZJ";
        }
    }
    return ".notfound";
}
#undef String

static QString getKTExportName(const QString &instrument) {
    QString month = instrument.right(2);
    QString name = instrument.left(instrument[1].isLetter() ? 2 : 1);
    return name + month;
}

QList<Bar>* QuantTrader::getBars(const QString &instrument, const QString &time_frame_str)
{
    int time_frame_value = BarCollector::staticMetaObject.enumerator(barCollector_enumIdx).keyToValue(time_frame_str.trimmed().toLatin1().data());
    if (bars_map.contains(instrument)) {
        if (bars_map[instrument].contains(time_frame_value)) {
            return bars_map[instrument][time_frame_value];
        }
    }

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ctp", "quant_trader");
    settings.beginGroup("HistoryPath");
    QString kt_export_path = settings.value("ktexport").toString();
    QString collector_path = settings.value("collector").toString();
    settings.endGroup();

    // Load KT Export Data
    const QString kt_export_file_name = kt_export_path + "/" + time_frame_str + "/" + getKTExportName(instrument) + getSuffix(instrument);
    QFile kt_export_file(kt_export_file_name);
    kt_export_file.open(QFile::ReadOnly);
    QDataStream stream(&kt_export_file);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    QList<KTExportBar> *ktbarlist = new QList<KTExportBar>();
    stream >> *ktbarlist;
    QList<Bar> *barlist = new QList<Bar>();
    foreach (const KTExportBar &ktbar, *ktbarlist) {
        barlist->append(ktbar);
    }

    // load Collector Bars
    const QString collector_bar_path = collector_path + "/" + time_frame_str + "/" + getKTExportName(instrument);
    QDir collector_bar_dir(collector_bar_path);
    QStringList entries = collector_bar_dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    foreach (const QString &barfilename, entries) {
        QFile barfile(barfilename);
        barfile.open(QFile::ReadOnly);
        QDataStream stream(&kt_export_file);
        stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
        stream >> *barlist;
    }

    bars_map[instrument][time_frame_value] = barlist;
    return barlist;
}

QuantTrader::~QuantTrader()
{
    qDebug() << "~QuantTrader";
}

AbstractIndicator* QuantTrader::registerIndicator(const QString &instrument, const QString &time_frame_str, const QString &indicator_name, ...)
{
    AbstractIndicator* ret = nullptr;
    va_list ap;
    va_start(ap, indicator_name);

    if (indicator_name == "MA") {
        int period = va_arg(ap, int);
        int shift = va_arg(ap, int);
        MA::ENUM_MA_METHOD ma_method = va_arg(ap, MA::ENUM_MA_METHOD);
        MQL5IndicatorOnSingleDataBuffer::ENUM_APPLIED_PRICE applied_price = va_arg(ap, MQL5IndicatorOnSingleDataBuffer::ENUM_APPLIED_PRICE);

        foreach (AbstractIndicator *indicator, indicator_map.values("instrument")) {
            QObject *obj = (QObject*) indicator;
            if (QString("MA") == obj->metaObject()->className()) {
                MA *ma = (MA*) indicator;
                if (ma->getMAPeroid() == period && ma->getMAShift() == shift && ma->getMAMethod() == ma_method) {
                    ret = indicator;
                    break;
                }
            }
        }
        if (ret == nullptr) {
            ret = new MA(period, shift, ma_method, applied_price, QuantTrader::instance);
            indicator_map.insert(instrument, ret);
            ret->setBarList(getBars(instrument, time_frame_str));
        }
    } else if (indicator_name == "ParabolicSAR") {
        double SARStep = va_arg(ap, double);
        double SARMaximum = va_arg(ap, double);

        foreach (AbstractIndicator *indicator, indicator_map.values("instrument")) {
            QObject *obj = (QObject*) indicator;
            if (QString("ParabolicSAR") == obj->metaObject()->className()) {
                ParabolicSAR *psar = (ParabolicSAR*) indicator;
                if (psar->getSARStep() == SARStep && psar->getSARMaximum() == SARMaximum) {
                    ret = indicator;
                    break;
                }
            }
        }
        if (ret == nullptr) {
            ret = new ParabolicSAR(SARStep, SARMaximum, QuantTrader::instance);
            indicator_map.insert(instrument, ret);
            ret->setBarList(getBars(instrument, time_frame_str));
        }
    }
    va_end(ap);

    return ret;
}

void QuantTrader::onNewTick(int volume, double turnover, double openInterest, int time, double lastPrice, const QString &instrumentID)
{
    BarCollector *collector = collector_map.value(instrumentID, nullptr);
    if (collector != nullptr) {
        collector->onNewTick(volume, turnover, openInterest, time, lastPrice);
    }

    QList<AbstractStrategy *> strategies = strategy_map.values(instrumentID);
    foreach (AbstractStrategy *strategy, strategies) {
        strategy->onNewTick(volume, turnover, openInterest, time, lastPrice);
    }
}

