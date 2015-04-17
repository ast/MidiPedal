#include <TimerOne.h>

// Prototypes
void timeout();

volatile boolean didTimeout = false;

#define NUM_BUTTONS  3
#define MIDI_CHANNEL 1

void setup()
{
  // Input & Pullup
  DDRD  &= (0<<6);
  PORTD |= (1<<6);
  
  DDRF  &= (0<<7) & (0<<6);
  PORTF |= (1<<7) | (1<<6);
  
  // Interrupt timer
  Timer1.initialize(1000); // Scan once every 1ms
  Timer1.attachInterrupt(timeout); // Interrupt service function
}

void timeout() {
  didTimeout = true; // Semaphore
}

boolean c[NUM_BUTTONS] = {false};
boolean p[NUM_BUTTONS] = {false};

static inline boolean cc(byte control, byte value) {
  usbMIDI.sendControlChange(control, value, MIDI_CHANNEL);
  return true;
}

void loop()                  
{
  // Wait here until timeout
  while(!didTimeout);
  didTimeout = false;
 
  // Read current button state
  // PIND
  c[0] = !(PIND & _BV(6)); // 1 = on, 0 = off
  // PINF
  c[1] = !(PINF & _BV(7));
  c[2] = !(PINF & _BV(6));
  
  for(int i = 0; i < NUM_BUTTONS; i++) {
    // Abusing the ternary operator for function call side effect.
    c[i] != p[i] ? cc(0x10 + i, 127 * c[i]) : false;
  }
  
  // Save current button states (for edge detection).
  memcpy(p, c, 3);
  
  // Discard incoming messages
  while (usbMIDI.read());
}
