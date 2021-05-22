#include <Arduino.h>
#include <M5Stack.h>

#define NUMPIXELS 16
// Parameters
#define AUTOSTART false // start the metronome automatically upon power up and reset
#define INITBPM 96      // Default BPM to start with
#define BEATS 4         // Beats per bar (measure, cycle)
#define SUBBEAT 12      // Divide per beat into sub steps
#define UPBEAT false    // True - Accent on the off/upbeat, False - on/downbeat

#include "Beat_Gen.h" // This class deals with timing
#include "Tap2Bpm.h"
#include "M5_Ring_Metro.h"

const bool init_start = AUTOSTART;
unsigned int bpm = INITBPM;
unsigned int beats_p_bar = BEATS;
unsigned int steps_p_beat = SUBBEAT;
unsigned int totalpixels = NUMPIXELS;
bool upbeat = UPBEAT;
unsigned int steps_offset = steps_p_beat / 2 * upbeat;
char m = 'S', m_trapexit = 'S'; // R - running. S - standby. T - tapping

Beat_gen myBeat;
M5_Ring_Metro myRing;
Tap2Bpm myTapper(5);

void setup()
{
  M5.begin();

  myBeat.setBeats(bpm, beats_p_bar, steps_p_beat);
  // LCD display
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.print("VisualNome by DyLaron\n");
  myRing.begin(NUMPIXELS, beats_p_bar);
  myRing.setTicksRGB();
  myRing.showBPM(bpm);
  myRing.showDots();

  if (init_start)
  {
    myBeat.start(millis());
    m = 'R';
    Serial.print("Started (Auto-run)\n");
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
  M5.update(); // need to call update()
  bool buttonDrop = M5.BtnB.wasPressed();
  bool buttonRelease = M5.BtnB.wasReleased();
  bool longpress = M5.BtnB.pressedFor(1600);
  if (M5.BtnA.wasPressed() && bpm > 40)
  {
    bpm--;
    myRing.showBPM(bpm);
    myBeat.setBeats(bpm, beats_p_bar, steps_p_beat);
  }
  if (M5.BtnC.wasPressed() && bpm < 240)
  {
    bpm++;
    myRing.showBPM(bpm);
    myBeat.setBeats(bpm, beats_p_bar, steps_p_beat);
  }
  switch (m)
  {
  case 'S':
    if (longpress) // Going to tap mode, while stopping at Y to catch the button release
    {
      m = 'Z';
      m_trapexit = 'T';
      myRing.showMsg("TAP", TFT_GREEN);
    }
    else if (buttonRelease)
    {
      myBeat.start(M5.BtnB.lastChange());
      myRing.showBPM(bpm);
      m = 'R';
    }
    break;

  case 'R':
    // check if it's time on the beat
    if (myBeat.checktime())
    {
      unsigned int onbeat = (myBeat.current_step() == steps_offset) + ((myBeat.current_step() + steps_offset) % steps_p_beat == 0);
      myRing.updateRGB(myBeat.progress_pct(), onbeat);
      myRing.showDots();
      if (onbeat == 2)
        M5.Speaker.tone(261, 60);
      else if (onbeat == 1)
        M5.Speaker.tone(196, 30);
    }
    if (buttonDrop)
    {
      myBeat.stop();
      m = 'Z';
      m_trapexit = 'S';
    }
    break;

  case 'T':
    if (buttonRelease)
    {
      bool doneTapping = myTapper.tapNow(M5.BtnB.lastChange());
      myRing.showMsg(String(myTapper.getRemainingTaps()), TFT_GREEN);
      if (doneTapping)
      {
        if (myTapper.checkBPM())
        {
          bpm = myTapper.getBPM();
          myRing.showBPM(bpm);
          myBeat.setBeats(bpm, beats_p_bar, steps_p_beat);
          myBeat.start(M5.BtnB.lastChange());
          m = 'R';
        }
        else
        {
          myRing.showMsg("TAP", TFT_GREEN);
          // tap_count = 0;
        }
        myTapper.reset();
      }
    }
    if (longpress)
    {
      m = 'Z';
      m_trapexit = 'S';
    }
    break;

  case 'Z': //trap to catch a release action
    if (buttonRelease)
      m = m_trapexit;
    break;

  default:
    break;
  }
  delay(10);
}