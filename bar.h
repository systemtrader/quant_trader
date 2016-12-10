#ifndef BAR_H
#define BAR_H

#include <QDebug>
#include <QMetaType>

#define TimeType int
typedef unsigned short      WORD;

struct KTExportBar {
    TimeType m_time;        //时间,UCT
    float   m_fOpen;        //开盘
    float   m_fHigh;        //最高
    float   m_fLow;         //最低
    float   m_fClose;       //收盘
    float   m_fVolume;      //成交量(手)
    float   m_fAmount;      //成交额(元)/持仓(未平仓合约，仅期货有效)
    WORD    m_wAdvance;     //上涨家数(仅大盘有效)
    WORD    m_wDecline;     //下跌家数(仅大盘有效)
    float   amount;
    float   settle;
};

class Bar {
public:
    uint   time;
    double open;
    double high;
    double low;
    double close;
    long   tick_volume; // to be compatible with MT5(although MT5 use 64bit long)
    long   volume;      // to be compatible with MT5(although MT5 use 64bit long)

    Bar();
    Bar(const Bar &other);
    ~Bar();

    Bar(const KTExportBar &ktbar);

    void init();
    bool isNewBar();
};

Q_DECLARE_METATYPE(Bar)

QDebug operator<<(QDebug dbg, const Bar &bar);

#endif // BAR_H

