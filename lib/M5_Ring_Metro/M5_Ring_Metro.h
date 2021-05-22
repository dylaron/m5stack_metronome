#ifndef M5_RING_METRO_H
#define M5_RING_METRO_H

#include <Arduino.h>
#include <M5Stack.h>

class M5_Ring_Metro
{
private:
    void initXY();
    int screen_width = 320,
        screen_height = 240;
    int xc, yc;
    int numpixel;
    unsigned int beats, tick_interval;
    int x[24], y[24];
    int dot_r = 13;
    uint16_t rgb[24];
    uint16_t rgb_runner = TFT_BLUE,
             rgb_tick_1st = TFT_ORANGE,
             rgb_tick_rest = TFT_YELLOW,
             rgb_flash_strong = TFT_WHITE,
             rgb_flash_weak = TFT_LIGHTGREY;

public:
    M5_Ring_Metro(/* args */);
    void begin(int _dt, int _bt);
    void initXY(int _r);
    int getX(int _i);
    int getY(int _i);
    void showDots();
    void setTicksRGB();
    void updateRGB(float _progress, unsigned int _flash);
    void showMsg(String _msg, uint16_t _bgcolor);
    void centerCircle(uint16_t _bgcolor);
    void showBPM(int _b);
};

#endif