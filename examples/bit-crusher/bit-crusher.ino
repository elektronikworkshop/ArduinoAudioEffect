/* Bitcrusher effect
 *  
 *  Audio input:    A0
 *  Audio output:   11
 *  Potentiometer:  A1
 *  
 */

#include <EwAudioEffect.h>

EwAudioEffect effect;

const uint8_t MaxSampleSkip = 32;
uint8_t skipCounter = 0;

void setup()
{  
  effect.begin();
}

void loop()
{
  while (not effect.audioAvail()) {
      /* Wait for next audio sample to arrive */
  }

  int8_t sample = effect.audioRead();
  uint8_t pot = effect.controlRead(EwAudioEffect::CTRL_CH_1);

  uint8_t m = map(pot, 0, 255, 0, MaxSampleSkip);
  
  if (skipCounter >= m) {
    skipCounter = 0;
    effect.audioWrite(sample);
  } else {
    skipCounter++;
  }
}
