/**
 * MIDI Pedal.
*/
#include <USBComposite.h>
#include <HardwareSerial.h>
#include <wirish.h>

USBMIDI midi;

/* Pedal input pin */
const int analogInputPin = PA0;

static int pedalValue = 0;
static int analogValue = 0;

/**
 * MIDI event packet definitation.
*/
typedef struct
{
	uint8_t header;
	uint8_t byte1;
	uint8_t byte2;
	uint8_t byte3;
} midiEventPacket_t;

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  Serial1.write(event.header);
  Serial1.write(event.byte1);
  Serial1.write(event.byte2);
  Serial1.write(event.byte3);
}

int analogValueToMidiValue(int analogValue)
{
  return constrain(map(analogValue, 0, 3800, 0, 127), 0 ,127);
}

#define FILTER_N 48
#define FILTER_DELAY_MS 1

int filter(int value)
{
  int count=0;
  int new_value = analogRead(analogInputPin);
  while (value != new_value) {
    count++;
    if (count >= FILTER_N) {
      return new_value;
    }
    delay(FILTER_DELAY_MS);
    new_value = analogRead(analogInputPin);
  }
  return value;
}

void setup() {
  pinMode(analogInputPin, INPUT_ANALOG);
  /* Serial for MIDI OUTPUT */
  Serial1.begin(31250);
  /* Serial for DEBUG */
  Serial2.begin(115200);
  USBComposite.setProductId(0x0031);
  midi.begin();
  while (!USBComposite);
  Serial2.println("Setup done.");
}

void loop() {
  int tmpValue;
  analogValue = filter(analogValue);
  tmpValue = analogValueToMidiValue(analogValue);
  if (tmpValue != pedalValue) {
    pedalValue = tmpValue;
    /* Send to USB MIDI */
    midi.sendControlChange(0, 64, pedalValue);
    /* Send MIDI OUTPUT */
    controlChange(0, 64, pedalValue);
  }
}
