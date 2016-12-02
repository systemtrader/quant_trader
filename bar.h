#ifndef BAR_H
#define BAR_H

#include <QtGlobal>

class Bar {
public:
    uint   time;
    long   tick_volume; // to be compatible with MT5(although MT5 use 64bit long), and also for memory alignment
    double open;
    double high;
    double low;
    double close;
    double volume;

    Bar() { init(); }

    inline void init() {
        time = 0;
        tick_volume = 0;
        open = -1.5f;
        high = -1.0f;
        low = 100000.0f;
        close = -1.0f;
        volume = 0.0f;
    }
};

#endif // BAR_H

