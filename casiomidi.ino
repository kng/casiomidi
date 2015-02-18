/* CasioMidi, a retrofit for the Casio SA-1 (and probably others)
 * Replacing the M6387 with a separate key scanner and turning it into MIDI
 * Made by: Daniel Ekman, Jan 2015
 * Board: Arduino UNO (m328p)
 * Compiles to 4 478 bytes in Arduino 1.0.6
 */

#include <Keypad.h>
#include <ardumidi.h>

const byte ROWS = 7; // 7 total rows, 6 is you omit the demo button
const byte COLS = 8;
char keys[ROWS][COLS] = {
  {65,66,67,68,69,70,71,72}, // keys 0-8
  {73,74,75,76,77,78,79,80}, // keys 9-16
  {81,82,83,84,85,86,87,88}, // keys 17-24
  {89,90,91,92,93,94,95,96}, // keys 25-32
  {48,49,50,51,52,'>','+','*'}, // tone 0-4, temp +, vol +, select
  {53,54,55,56,57,'#','<','-'}, // tone 5-9, stop, temp-, vol-,
  {'!','!','!','!','!','!','!','!'}, // demo button, this row can be completely omitted and save a extra digital io pin
};
byte rowPins[ROWS] = {A3, A2, A1, A0, 12, 11, 10}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {9, 8, 7, 6, 5, 4, 3, 2}; //connect to the column pinouts of the keypad

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

const byte ledPin = 13;

unsigned char velo = 127;    // default velocity, 0-127
unsigned char chan = 0;      // default midi channel
unsigned char prog = 0;      // default instrument

void setup(){
  Serial.begin(31250);    // or 115200 for use with f.ex: hairless midi http://projectgus.github.io/hairless-midiserial/
  pinMode(ledPin, OUTPUT);      // sets the led pin as output
  digitalWrite(ledPin, LOW);            // Turns the LED on.
  kpd.setHoldTime(100);               // hold time is not used here yet
  kpd.setDebounceTime(5);           // turn down to get better response, up to get better debounce
}

void loop(){
  if (kpd.getKeys()){
    for (int i=0; i<LIST_MAX; i++){   // Scan the whole key list.
      if ( kpd.key[i].stateChanged ){   // Only find keys that have changed state.
        switch (kpd.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
          case PRESSED:
            if(kpd.key[i].kchar >= 65 && kpd.key[i].kchar <= 96){    // filter out notes
              midi_note_on(chan, kpd.key[i].kchar , velo);
            } else {
              if(kpd.key[i].kchar >= 48 && kpd.key[i].kchar <= 57){  // program change buttons
                prog = kpd.key[i].kchar - 48;
                midi_program_change(chan, prog);
              }
              if(kpd.key[i].kchar == '+' && velo < 127) velo++;      // volume up
              if(kpd.key[i].kchar == '-' && velo > 0) velo--;        // volume down
              if(kpd.key[i].kchar == '>' && prog < 127){             // increase instrument program
                prog++;
                midi_program_change(chan, prog);
              }
              if(kpd.key[i].kchar == '<' && prog > 0){                // decrease instrument program
                prog++;
                midi_program_change(chan, prog);
              }
            }
          break;
          case RELEASED:
            if(kpd.key[i].kchar >= 65 && kpd.key[i].kchar <= 96){
              midi_note_off(chan, kpd.key[i].kchar, velo);
            }
          break;
          }
      }
    }
  }
}
