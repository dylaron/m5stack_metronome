#include "M5_Ring_Metro.h"

M5_Ring_Metro::M5_Ring_Metro(/* args */)
{
    this->xc = this->screen_width / 2;
    this->yc = this->screen_height / 2;
}

void M5_Ring_Metro::initXY(int _r)
{
    float raddev = 2 * 3.1416 / this->numpixel;
    for (int i = 0; i < this->numpixel; i++)
    {
        this->x[i] = (int)(this->xc + _r * sin(raddev * i));
        this->y[i] = (int)(this->yc - _r * cos(raddev * i));
    }
}

void M5_Ring_Metro::begin(int _dt, int _bt)
{
    this->numpixel = _dt;
    this->beats = _bt;
    this->tick_interval = _dt / _bt;
    this->initXY(94);
}

int M5_Ring_Metro::getX(int _i)
{
    return this->x[_i];
}

int M5_Ring_Metro::getY(int _i)
{
    return this->y[_i];
}

void M5_Ring_Metro::showDots()
{
    for (int j = 0; j < this->numpixel; j++)
    {
        M5.Lcd.fillCircle(this->x[j], this->y[j], this->dot_r, rgb[j]);
    }
};

void M5_Ring_Metro::setTicksRGB()
{
    rgb[0] = this->rgb_tick_1st;
    for (int i = this->tick_interval; i < this->numpixel; i += this->tick_interval)
    {
        rgb[i] = this->rgb_tick_rest;
    }
};

void M5_Ring_Metro::updateRGB(float _progress, unsigned int _flash)
{
    uint16_t _rgb_tmp = TFT_BLACK;
    // flashing on accent
    if (_flash == 2)
        _rgb_tmp = this->rgb_flash_strong;
    else if (_flash == 1)
        _rgb_tmp = this->rgb_flash_weak;
    for (int i = 0; i < this->numpixel; i++)
        this->rgb[i] = _rgb_tmp;
    this->setTicksRGB();
    this->rgb[0] = this->rgb_tick_1st;
    this->rgb[(uint)(this->numpixel * _progress)] = this->rgb_runner;
};

void M5_Ring_Metro::centerCircle(uint16_t _bgcolor)
{
    M5.Lcd.fillCircle(this->xc, this->yc, 52, _bgcolor);
};

void M5_Ring_Metro::showMsg(String _msg, uint16_t _bgcolor)
{
    this->centerCircle(_bgcolor);
    M5.Lcd.setCursor(this->xc - 27, this->yc - 14);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(4);
    M5.Lcd.setTextColor(TFT_YELLOW, _bgcolor);
    M5.Lcd.print(_msg);
}

void M5_Ring_Metro::showBPM(int _b)
{
    this->showMsg(String(_b), TFT_DARKCYAN);
}