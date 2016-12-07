#include <QCoreApplication>

#include "bar.h"
#include "quant_trader.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qRegisterMetaType<Bar>();
    QuantTrader quantTrader;
    return a.exec();
}
