#include <QSettings>

#include "quant_trader.h"
#include "bar.h"
#include "bar_collector.h"
#include "indicator/ma.h"
#include "indicator/parabolicsar.h"
#include "strategy/DblMaPsar_strategy.h"

QuantTrader* QuantTrader::instance;

int barCollector_enumIdx;
int MA_METHOD_enumIdx;
int APPLIED_PRICE_enumIdx;

QuantTrader::QuantTrader(QObject *parent) :
    QObject(parent)
{
    QuantTrader::instance = this;

    barCollector_enumIdx = BarCollector::staticMetaObject.indexOfEnumerator("TimeFrame");
    MA_METHOD_enumIdx = MA::staticMetaObject.indexOfEnumerator("ENUM_MA_METHOD");
    APPLIED_PRICE_enumIdx = MQL5IndicatorOnSingleDataBuffer::staticMetaObject.indexOfEnumerator("ENUM_APPLIED_PRICE");

    loadQuantTraderSettings();
    loadTradeStrategySettings();

    pExecuter = new org::ctp::ctp_executer("org.ctp.ctp_executer", "/ctp_executer", QDBusConnection::sessionBus(), this);
    pWatcher = new org::ctp::market_watcher("org.ctp.market_watcher", "/market_watcher", QDBusConnection::sessionBus(), this);
    connect(pWatcher, SIGNAL(newTick(int,double,double,uint,double,QString)), this, SLOT(onNewTick(int,double,double,uint,double,QString)));
}

void QuantTrader::loadQuantTraderSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ctp", "quant_trader");
    settings.beginGroup("Collector");
    QStringList instrumentList = settings.childKeys();

    foreach (const QString &instrument, instrumentList) {
        QString time_frame_string_with_dots = settings.value(instrument).toString();
        QStringList time_frame_stringlist = time_frame_string_with_dots.split('|');
        BarCollector::TimeFrames time_frame_flags;
        foreach (const QString &tf, time_frame_stringlist) {
            int time_frame_value = BarCollector::staticMetaObject.enumerator(barCollector_enumIdx).keyToValue(tf.trimmed().toLatin1().data());
            BarCollector::TimeFrame time_frame = static_cast<BarCollector::TimeFrame>(time_frame_value);
            time_frame_flags |= time_frame;
        }

        BarCollector *collector = new BarCollector(instrument, time_frame_flags, this);
        collector_map[instrument] = collector;
        qDebug() << instrument << ":\t" << time_frame_flags << "\t" << time_frame_stringlist;
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
        QString instrument = settings.value("instrument").toString();
        QString time_frame = settings.value("timeframe").toString();

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

        const QMetaObject* strategy_meta_object = meta_object_map.value(strategy_name);
        QObject *object = strategy_meta_object->newInstance(Q_ARG(QString, group), Q_ARG(QString, instrument), Q_ARG(QString, time_frame), Q_ARG(QObject*, this));
        if (object == NULL) {
            qDebug() << "Instantiating strategy " << group << " failed!";
            continue;
        }

        AbstractStrategy *strategy = qobject_cast<AbstractStrategy*>(object);
        if (strategy == NULL) {
            qDebug() << "Cast strategy " << group << " failed!";
            delete object;
            continue;
        }

        strategy->setParameter(param1, param2, param3, param4, param5, param6, param7, param8, param9);
        strategy->setBarList(getBars(instrument, time_frame), collector_map[instrument]->getCurrentBar(time_frame));
        strategy_map.insert(instrument, strategy);

        if (position_map.contains(instrument)) {
            position_map[instrument] += strategy->getPosition();
        } else {
            position_map.insert(instrument, strategy->getPosition());
        }
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

/* 以下这段代码包括getSuffix函数是从Java移植过来的, 写成这样是为了尽量维持与VC++2010和c++11的兼容性 */
/* 如果不考虑兼容VC++2010, 可直接使用Range-based for loop */

// 上海期货交易所                                   燃油, 线材
const static QString SQ[] = {"fu", "wr"};
// 上海期货交易所 (夜盘)              铜,   铝,   锌,   铅,   镍,   锡,   金,   银,螺纹钢,热轧卷板,沥青,天然橡胶
const static QString SY[] = {"cu", "al", "zn", "pb", "ni", "sn", "au", "ag", "rb", "hc", "bu", "ru"};
// 大连商品交易所                                  玉米, 玉米淀粉, 纤维板,  胶合板, 鸡蛋, 线型低密度聚乙烯, 聚氯乙烯, 聚丙烯
const static QString DL[] = {"c",  "cs", "fb", "bb", "jd", "l",  "v",  "pp"};
// 大连商品交易所  (夜盘)          黄大豆1号, 黄大豆2号, 豆粕, 大豆原油, 棕榈油, 冶金焦炭, 焦煤, 铁矿石
const static QString DY[] = {"a",  "b",  "m",  "y",  "p",  "j",  "jm", "i"};
// 郑州商品交易所
const static QString ZZ[] = {"jr", "lr", "pm", "ri", "rs", "sf", "sm", "wh"};
// 郑州商品交易所 (夜盘)
const static QString ZY[] = {"cf", "fg", "ma", "oi", "rm", "sr", "ta", "zc", "tc"};	// zc原来为tc
// 中金所
const static QString ZJ[] = {"ic", "if", "ih", "t",  "tf"};

#define String const QString&
#ifndef _MSC_VER
#define in :
#define each
#endif
// 通过合约名获得文件的扩展名
static QString getSuffix(String instrument) {
    const QString instrumentLowerCase = instrument.left(instrument[1].isLetter() ? 2 : 1).toLower();
    for each (String instr in SQ) {
        if (instrumentLowerCase == instr) {
            return ".SQ";
        }
    }
    for each (String instr in SY) {
        if (instrumentLowerCase == instr) {
            return ".SY";
        }
    }
    for each (String instr in DL) {
        if (instrumentLowerCase == instr) {
            return ".DL";
        }
    }
    for each (String instr in DY) {
        if (instrumentLowerCase == instr) {
            return ".DY";
        }
    }
    for each (String instr in ZZ) {
        if (instrumentLowerCase == instr) {
            return ".ZZ";
        }
    }
    for each (String instr in ZY) {
        if (instrumentLowerCase == instr) {
            return ".ZY";
        }
    }
    for each (String instr in ZJ) {
        if (instrumentLowerCase == instr) {
            return ".ZJ";
        }
    }
    return ".notfound";
}
#undef String
#ifndef _MSC_VER
#undef in
#undef each
#endif

static QString getKTExportName(const QString &instrument) {
    QString month = instrument.right(2);
    QString name = instrument.left(instrument[1].isLetter() ? 2 : 1);
    return name + month;
}

QList<Bar>* QuantTrader::getBars(const QString &instrumentID, const QString &time_frame_str)
{
    int time_frame_value = BarCollector::staticMetaObject.enumerator(barCollector_enumIdx).keyToValue(time_frame_str.trimmed().toLatin1().data());
    if (bars_map.contains(instrumentID)) {
        if (bars_map[instrumentID].contains(time_frame_value)) {
            return &bars_map[instrumentID][time_frame_value];
        }
    }

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ctp", "quant_trader");
    settings.beginGroup("HistoryPath");
    QString kt_export_path = settings.value("ktexport").toString();
    QString collector_path = settings.value("collector").toString();
    settings.endGroup();

    // Load KT Export Data
    const QString kt_export_file_name = kt_export_path + "/" + time_frame_str + "/" + getKTExportName(instrumentID) + getSuffix(instrumentID);
    QFile kt_export_file(kt_export_file_name);
    kt_export_file.open(QFile::ReadOnly);
    QDataStream stream(&kt_export_file);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);

    QList<Bar> *barList = &bars_map[instrumentID][time_frame_value];
    QList<KTExportBar> ktBarList;
    stream.skipRawData(12);
    stream >> ktBarList;
    foreach (const KTExportBar &ktbar, ktBarList) {
        barList->append(ktbar);
    }

    // load Collector Bars
    const QString collector_bar_path = collector_path + "/" + time_frame_str + "/" + getKTExportName(instrumentID);
    QDir collector_bar_dir(collector_bar_path);
    QStringList entries = collector_bar_dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    foreach (const QString &barfilename, entries) {
        QFile barfile(barfilename);
        barfile.open(QFile::ReadOnly);
        QDataStream stream(&kt_export_file);
        stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
        stream >> *barList;
    }

    return barList;
}

QuantTrader::~QuantTrader()
{
    qDebug() << "~QuantTrader";
}

AbstractIndicator* QuantTrader::registerIndicator(const QString &instrumentID, const QString &time_frame_str, const QString &indicator_name, ...)
{
    AbstractIndicator* ret = nullptr;
    va_list ap;
    va_start(ap, indicator_name);

    bool newCreate = false;
    if (indicator_name == "MA") {
        int period = va_arg(ap, int);
        int shift = va_arg(ap, int);
        int ma_method = va_arg(ap, int);
        int applied_price = va_arg(ap, int);

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
            ret = new MA(period, shift, static_cast<MA::ENUM_MA_METHOD>(ma_method), static_cast<MQL5IndicatorOnSingleDataBuffer::ENUM_APPLIED_PRICE>(applied_price), QuantTrader::instance);
            newCreate = true;
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
            newCreate = true;
        }
    }
    va_end(ap);

    if (newCreate) {
        indicator_map.insert(instrumentID, ret);
        ((MQL5Indicator*)ret)->OnInit();
        ret->setBarList(getBars(instrumentID, time_frame_str), collector_map[instrumentID]->getCurrentBar(time_frame_str));
        ret->update();
    }

    return ret;
}

void QuantTrader::onNewTick(int volume, double turnover, double openInterest, uint time, double lastPrice, const QString &instrumentID)
{
    BarCollector *collector = collector_map.value(instrumentID, nullptr);
    if (collector != nullptr) {
        collector->onNewTick(volume, turnover, openInterest, time, lastPrice);
    }

    QList<AbstractStrategy *> strategies = strategy_map.values(instrumentID);
    int new_position_sum = 0;
    foreach (AbstractStrategy *strategy, strategies) {
        strategy->onNewTick(volume, turnover, openInterest, time, lastPrice);
        new_position_sum += strategy->getPosition();
    }
    if (position_map[instrumentID] != new_position_sum) {
        position_map[instrumentID] = new_position_sum;
        pExecuter->setPosition(instrumentID, new_position_sum);
    }
}

