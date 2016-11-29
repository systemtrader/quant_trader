#ifndef BAR_H
#define BAR_H

class Bar {
public:
    // int date;
    int time;
    double open;
    double high;
    double low;
    double close;
    // double volume;

    Bar() { init(); }

    inline void init() {
        // date = 0;
        time = 0;
        open = -1.5f;
        high = -1.0f;
        low = 100000.0f;
        close = -1.0f;
        /* volume(-1.0f) */
    }
};

#endif // BAR_H

