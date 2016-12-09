#include <QCoreApplication>

#include "quant_trader.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QuantTrader quantTrader;
    return a.exec();
}
