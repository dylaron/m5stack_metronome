#include <Arduino.h>
#include <M5Stack.h>
#include <Fsm.h> // arduino-fsm Library by Jon Black

#include "Defines.h" // all configurable variables are there

#include "Beat_Gen.h" // This class deals with timing
#include "Tap2Bpm.h"
#include "M5_Ring_Metro.h"

const bool init_start = AUTOSTART;
unsigned int bpm = INITBPM;
unsigned int beats_p_bar = BEATS;
unsigned int steps_p_beat = SUBBEAT;
unsigned int totalpixels = NUMPIXELS;
bool upbeat = UPBEAT,
     long_pressed_fired = false,
     tapping_accepted = false,
     ignore_next_release = false;
unsigned int steps_offset = steps_p_beat / 2 * upbeat;
bool mute_status = false;
String mute_message[2] = {"SOUND", "MUTED"};

Beat_gen myBeat;
M5_Ring_Metro myRing;
Tap2Bpm myTapper(5);

// Trigger Events for the State Machine --------------------
#define BUTTON_PRESSED_EVENT 0
#define BUTTON_RELEASED_EVENT 1
#define BUTTON_LONGPRESS_EVENT 2
#define TAPPING_SUCC_EVENT 3

//State Machine States
void on_standby_enter()
{
  myRing.setTicksRGB();
  myRing.showBPM(bpm);
  myRing.showDots();
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(TFT_YELLOW, TFT_DARKGREY);
  M5.Lcd.setCursor(50, 220);
  M5.Lcd.print(" - ");
  M5.Lcd.setCursor(237, 220);
  M5.Lcd.print(" + ");
}
//----------------------------------------------------------
void on_active_enter()
{
  myBeat.start(M5.BtnB.lastChange());
  myRing.showBPM(bpm);
}
//----------------------------------------------------------
void on_active_state()
{
  // check if it's time on the beat
  if (myBeat.checktime())
  {
    unsigned int onbeat = (myBeat.current_step() == steps_offset) + ((myBeat.current_step() + steps_offset) % steps_p_beat == 0);
    myRing.updateRGB(myBeat.progress_pct(), onbeat);
    myRing.showDots();
    if (!mute_status)
    {
      if (onbeat == 2)
        M5.Speaker.tone(261, 60);
      else if (onbeat == 1)
        M5.Speaker.tone(196, 30);
    }
  }
}
//----------------------------------------------------------
void on_active_exit()
{
  ignore_next_release = true;
  myBeat.stop();
}
//----------------------------------------------------------
void on_tapping_enter()
{
  myRing.showMsg("TAP", TFT_GREEN);
  myTapper.reset();
  ignore_next_release = true;
}
//----------------------------------------------------------
void on_tapping_state()
{
  if (M5.BtnB.wasReleased())
    if (!ignore_next_release)
    {
      bool doneTapping = myTapper.tapNow(M5.BtnB.lastChange());
      myRing.showMsg(String(myTapper.getRemainingTaps()), TFT_GREEN);
      if (doneTapping)
      {
        if (myTapper.checkBPM())
        {
          bpm = myTapper.getBPM();
          Serial.println("New BPM = ");
          Serial.println(bpm);
          myBeat.setBeats(bpm, beats_p_bar, steps_p_beat);
          tapping_accepted = true;
        }
        else
        {
          myRing.showMsg("TAP", TFT_GREEN);
          myTapper.reset();
        }
      }
      else
        ignore_next_release = false;
    }
}

//State Machine Declaration
State state_standby(&on_standby_enter, NULL, NULL);
State state_active(&on_active_enter, &on_active_state, &on_active_exit);
State state_tapping(&on_tapping_enter, &on_tapping_state, NULL);
Fsm fsm_metronome(&state_standby);
//----------------------------------------------------------
void checkButtonsAndTrig(Button &_b, Fsm &_fsm)
{
  if (_b.wasPressed())
  {
    _fsm.trigger(BUTTON_PRESSED_EVENT);
    long_pressed_fired = false;
  }

  if (_b.pressedFor(1600)) // Going to tap mode
  {
    if (!long_pressed_fired)
    {
      _fsm.trigger(BUTTON_LONGPRESS_EVENT);
      long_pressed_fired = true;
      ignore_next_release = true;
    }
  }
  else if (_b.wasReleased())
  {
    if (!ignore_next_release)
      _fsm.trigger(BUTTON_RELEASED_EVENT);
    else
      ignore_next_release = false;
  }
}
//----------------------------------------------------------
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

  fsm_metronome.add_transition(&state_standby, &state_active, BUTTON_RELEASED_EVENT, NULL);
  fsm_metronome.add_transition(&state_standby, &state_tapping, BUTTON_LONGPRESS_EVENT, NULL);
  fsm_metronome.add_transition(&state_tapping, &state_standby, BUTTON_LONGPRESS_EVENT, NULL);
  fsm_metronome.add_transition(&state_active, &state_standby, BUTTON_PRESSED_EVENT, NULL);
  fsm_metronome.add_transition(&state_tapping, &state_active, TAPPING_SUCC_EVENT, NULL);
}
//----------------------------------------------------------
void loop()
{
  // put your main code here, to run repeatedly:
  M5.update(); // need to call update()
  fsm_metronome.run_machine();
  checkButtonsAndTrig(M5.BtnB, fsm_metronome);
  if (tapping_accepted)
  {
    fsm_metronome.trigger(TAPPING_SUCC_EVENT);
    tapping_accepted = false;
  }

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

  if (M5.BtnA.wasPressed() && M5.BtnC.wasPressed())
  {
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
    M5.Lcd.setCursor(260, 2);
    mute_status = !mute_status;
    M5.Lcd.print(mute_message[mute_status]);
  }
  delay(20);
}
