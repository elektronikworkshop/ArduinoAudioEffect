/* Delay effect
 *  
 *  Audio input:   A0
 *  Delay value:   A1
 *  Freeze button: 2
 *  
 */

#include <EwAudioEffect.h>

EwAudioEffect effect;

uint8_t freezeButtonPin = 2;

const unsigned int N = 1424;
int writeIndex = 0;
int8_t ringbuffer[N];

void setup()
{
  pinMode(freezeButtonPin, INPUT_PULLUP);
  
  effect.begin();
}

/* The write buffer runs backwards */
unsigned int
wrapWriteIndex(unsigned int index)
{
  return index >= N ? N - 1 : index;
}

/* The read index is a positive offset to the write buffer
 *  and wraps therefore in the other direction.
 */
unsigned int
wrapReadIndex(unsigned int index)
{
  if (index >= N) {
    return index - N;
  } else {
    return index;
  }
}

void loop()
{
  while (not effect.audioAvail()) {
      /* Wait for next audio sample to arrive */
  }

  int8_t sample = effect.audioRead();
  uint8_t pot = effect.controlRead(EwAudioEffect::CTRL_CH_1);

  writeIndex = wrapWriteIndex(writeIndex - 1);

  /* We don't write to the ring buffer when the freeze button is
   *  active. This keeps the current audio data in the delay line
   *  and plays it back as a loop. With the potentiometer you
   *  can select then the start point in the buffer.
   */
  if (digitalRead(freezeButtonPin) == HIGH) {
    ringbuffer[writeIndex] = sample;
  }

  /* Compute read index from current write position and the desired
   *  delay value set through the potentiometer
   */
  unsigned int readIndex = writeIndex + map(pot, 0, 255, 0, N - 1);
  readIndex = wrapReadIndex(readIndex);

  /* Add current sample and sample from ring buffer */
  int16_t output;
  output  = sample;
  output += ringbuffer[readIndex];

  /* There are two routes we can follow to avoid overflow (and the
   * resulting number wrap):
   *  
   *   a) defensively scale the audio signal by the number of additions
   *   
   *   b) clipping the audio signal such that the signal can not wrap
   *      in case the addition should result in too big/small values
   * 
   * By flipping this define you can select between both.
   */
#if 1
  /* Scale by factor two since we're adding two audio signals */
  output >>= 1;
#else
  /* Clip audio in case the addition results in something greater
   *  than something that can be represented with an int8_t
   */
  output = output < -128 ? -128 : output;
  output = output >  127 ?  127 : output;
#endif

  /* Write audio to output */
  effect.audioWrite(output);
}
