#include <QMultiMap>
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
    qRegisterMetaType<int>("ENUM_MA_METHOD");
    qRegisterMetaType<int>("ENUM_APPLIED_PRICE");

    QuantTrader::instance = this;

    barCollector_enumIdx = BarCollector::staticMetaObject.indexOfEnumerator("TimeFrame");
    MA_METHOD_enumIdx = MA::staticMetaObject.indexOfEnumerator("ENUM_MA_METHOD");
    APPLIED_PRICE_enumIdx = MQL5IndicatorOnSingleDataBuffer::staticMetaObject.indexOfEnumerator("ENUM_APPLIED_PRICE");

    loadQuantTraderSettings();
    loadTradeStrategySettings();

    saveBarTimer = new QTimer(this);
    saveBarTimer->setSingleShot(true);
    connect(saveBarTimer, SIGNAL(timeout()), this, SLOT(resetSaveBarTimer()));
    foreach (auto & collector, collector_map) {
        connect(saveBarTimer, SIGNAL(timeout()), collector, SLOT(saveBars()));
    }
    resetSaveBarTimer();

    pExecuter = new org::ctp::ctp_executer("org.ctp.ctp_executer", "/ctp_executer", QDBusConnection::sessionBus(), this);
    pWatcher = new org::ctp::market_watcher("org.ctp.market_watcher", "/market_watcher", QDBusConnection::sessionBus(), this);
    connect(pWatcher, SIGNAL(newMarketData(QString, uint, double, int, double, double)), this, SLOT(onMarketData(QString, uint, double, int)));
}

QuantTrader::~QuantTrader()
{
    qDebug() << "~QuantTrader";
}

void QuantTrader::loadQuantTraderSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ctp", "quant_trader");

    settings.beginGroup("HistoryPath");
    kt_export_dir = settings.value("ktexport").toString();
    BarCollector::collector_dir = settings.value("collector").toString();
    settings.endGroup();

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
    static const auto meta_object_map = []() -> QMap<QString, const QMetaObject*> {
        QMap<QString, const QMetaObject*> map;
        map.insert("DblMaPsarStrategy", &DblMaPsarStrategy::staticMetaObject);
        // Register more strategies here
        return map;
    }();

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

        auto *strategy = qobject_cast<AbstractStrategy*>(object);
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

    // Load KT Export Data
    const QString kt_export_file_name = kt_export_dir + "/" + time_frame_str + "/" + getKTExportName(instrumentID) + getSuffix(instrumentID);
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
    const QString collector_bar_path = BarCollector::collector_dir + "/" + instrumentID + "/" + time_frame_str;
    QDir collector_bar_dir(collector_bar_path);
    QStringList filters;
    filters << "*.bars";
    collector_bar_dir.setNameFilters(filters);
    QStringList entries = collector_bar_dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    foreach (const QString &barfilename, entries) {
        QFile barfile(barfilename);
        barfile.open(QFile::ReadOnly);
        QDataStream stream(&kt_export_file);
        stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
        stream >> *barList;
    }

    if (collector_map.contains(instrumentID)) {
        connect(collector_map[instrumentID], SIGNAL(collectedBar(QString,int,Bar)), this, SLOT(onNewBar(QString,int,Bar)), Qt::DirectConnection);
    } else {
        qDebug() << "Warning! Missing collector for " << instrumentID << " !";
    }

    return barList;
}

static QVariant getParam(const QByteArray &typeName, va_list &ap)
{
    QVariant ret;
    int typeId = QMetaType::type(typeName);
    switch (typeId) {
    case QMetaType::Int:
    {
        int value = va_arg(ap, int);
        ret.setValue(value);
    }
        break;
    case QMetaType::Double:
    {
        double value = va_arg(ap, double);
        ret.setValue(value);
    }
        break;
    default:
        break;
    }
    return ret;
}

static bool compareValue(const QVariant &v1, const QVariant &v2)
{
    int typeId = v1.userType();
    if (typeId != v2.userType()) {
        return false;
    }
    switch (typeId) {
    case QMetaType::Int:
        return v1.toInt() == v1.toInt();
    case QMetaType::Double:
        return v1.toDouble() == v2.toDouble();
    default:
        break;
    }
    return false;
}

AbstractIndicator* QuantTrader::registerIndicator(const QString &instrumentID, const QString &time_frame_str, QString indicator_name, ...)
{
    static const auto meta_object_map = []() -> QMap<QString, const QMetaObject*> {
        QMap<QString, const QMetaObject*> map;
        map.insert("MA", &MA::staticMetaObject);
        map.insert("ParabolicSAR", &ParabolicSAR::staticMetaObject);
        // Register more indicators here
        return map;
    }();

    const QMetaObject * metaObject = meta_object_map.value(indicator_name, nullptr);
    if (metaObject == nullptr) {
        qDebug() << "Indicator " << indicator_name << " not exist!";
        return nullptr;
    }

    const int class_info_count = metaObject->classInfoCount();
    int parameter_number = 0;
    for (int i = 0; i < class_info_count; i++) {
        QMetaClassInfo classInfo = metaObject->classInfo(i);
        if (QString(classInfo.name()).compare("parameter_number", Qt::CaseInsensitive) == 0) {
            parameter_number = QString(classInfo.value()).toInt();
            qDebug() << parameter_number;
        }
    }

    va_list ap;
    va_start(ap, indicator_name);

    auto names = metaObject->constructor(0).parameterNames();
    auto types = metaObject->constructor(0).parameterTypes();
    QList<QVariant> params;
    for (int i = 0; i < parameter_number; i++) {
        params.append(getParam(types[i], ap));
    }
    va_end(ap);

    qDebug() << params;

    foreach (AbstractIndicator *indicator, indicator_map.values(instrumentID)) {
        QObject *obj = dynamic_cast<QObject*>(indicator);
        if (indicator_name == obj->metaObject()->className()) {
            bool match = true;
            for (int i = 0; i < parameter_number; i++) {
                QVariant value = obj->property(names[i]);
                if (!compareValue(params[i], value)) {
                    match = false;
                }
            }
            if (match) {
                return indicator;
            }
        }
    }

    QVector<QGenericArgument> args;
    args.reserve(10);

    int int_param[10];
    int int_idx = 0;
    double double_param[10];
    int double_idx = 0;

#define Q_ENUM_ARG(type, data) QArgument<int >(type, data)

    for (int i = 0; i < parameter_number; i++) {
        int typeId = params[i].userType();
        switch (typeId) {
        case QMetaType::Int:
            int_param[int_idx] = params[i].toInt();
            args.append(Q_ENUM_ARG(types[i].constData(), int_param[int_idx]));
            int_idx ++;
            break;
        case QMetaType::Double:
            double_param[double_idx] = params[i].toDouble();
            args.append(Q_ARG(double, double_param[double_idx]));
            double_idx ++;
            break;
        default:
            args.append(QGenericArgument());
            break;
        }
    }
    args.append(Q_ARG(QObject*, this));

    QObject * obj =
    metaObject->newInstance(args.value(0), args.value(1), args.value(2),
                            args.value(3), args.value(4), args.value(5),
                            args.value(6), args.value(7), args.value(8), args.value(9));

    if (obj == 0) {
        qDebug() << "newInstance returns 0!";
        return nullptr;
    }

    auto* ret = dynamic_cast<AbstractIndicator*>(obj);

    indicator_map.insert(instrumentID, ret);
    ret->setBarList(getBars(instrumentID, time_frame_str), collector_map[instrumentID]->getCurrentBar(time_frame_str));
    ret->update();

    return ret;
}

void QuantTrader::resetSaveBarTimer()
{
    static const auto time_points = []() -> QList<QTime> {
        QList<QTime> tlist;
        tlist << QTime(2, 32) << QTime(10, 17) << QTime(11, 32) << QTime(15, 17);
        return tlist;
    }();

    const int size = time_points.size();
    int i;
    for (i = 0; i < size; i++) {
        int diff = QTime::currentTime().msecsTo(time_points[i]);
        if (diff > 1000) {
            saveBarTimer->start(diff);
            break;
        }
    }
    if (i == size) {
        int diff = QTime::currentTime().msecsTo(time_points[0]);
        saveBarTimer->start(diff + 86400000);   // diff should be negative, there are 86400 seconds in a day
    }
}

void QuantTrader::onMarketData(const QString& instrumentID, uint time, double lastPrice, int volume)
{
    BarCollector *collector = collector_map.value(instrumentID, nullptr);
    bool isNewTick = false;
    if (collector != nullptr) {
        isNewTick = collector->onMarketData(time, lastPrice, volume);
    }

    auto strategyList = strategy_map.values(instrumentID);
    int new_position_sum = 0;
    foreach (auto *strategy, strategyList) {
        if (isNewTick) {    // 有新的成交价格
            strategy->onNewTick(time, lastPrice);
        }
        new_position_sum += strategy->getPosition();
    }
    if (position_map[instrumentID] != new_position_sum) {
        position_map[instrumentID] = new_position_sum;
        pExecuter->setPosition(instrumentID, new_position_sum);
    }
}

void QuantTrader::onNewBar(const QString &instrumentID, int time_frame, const Bar &bar)
{
    bars_map[instrumentID][time_frame].append(bar);
    auto strategyList = strategy_map.values(instrumentID);
    foreach (auto *strategy, strategyList) {
        strategy->checkIfNewBar();
    }
}
