
#include "EwAudioEffect.h"

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

static EwAudioEffect *effect;

void
EwAudioEffect::begin()
{
  effect = this;

  // set adc prescaler  to 64 for 19kHz sampling frequency
  cbi(ADCSRA, ADPS2);
  sbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);

  sbi(ADMUX, ADLAR);  // 8-Bit ADC in ADCH Register
  sbi(ADMUX, REFS0);  // VCC Reference
  cbi(ADMUX, REFS1);
  cbi(ADMUX, MUX0);   // Set Input Multiplexer to Channel 0
  cbi(ADMUX, MUX1);
  cbi(ADMUX, MUX2);
  cbi(ADMUX, MUX3);

  // Timer2 PWM Mode set to fast PWM 
  cbi (TCCR2A, COM2A0);
  sbi (TCCR2A, COM2A1);
  sbi (TCCR2A, WGM20);
  sbi (TCCR2A, WGM21);
  cbi (TCCR2B, WGM22);

  // Timer2 Clock Prescaler to : 1 
  sbi (TCCR2B, CS20);
  cbi (TCCR2B, CS21);
  cbi (TCCR2B, CS22);

  // Timer2 PWM Port Enable
  sbi(DDRB, 3);                    // set digital pin 11 to output

  //cli();                         // disable interrupts to avoid distortion
  cbi (TIMSK0, TOIE0);              // disable Timer0 !!! delay is off now
  sbi (TIMSK2, TOIE2);              // enable Timer2 Interrupt
}

static boolean div32 = false;
static boolean div16 = false;
static volatile byte ibb = 0;  // NOP left over from original sketch
static uint8_t otherCh = 0;

static inline void
selectAdcCh(uint8_t channel)
{
  ADMUX &= ~((1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (1<<MUX0));
  ADMUX |= channel;
}

//******************************************************************
// Timer2 Interrupt Service at 62.5 KHz
// here the audio and pot signal is sampled in a rate of:  16Mhz / 256 / 2 / 2 = 15625 Hz
// runtime : xxxx microseconds

/**
 * The ADC channels are sampled as follows:
 * 
 *  0 1 0 2 0 3 0 4 0 5 0 6 0 7 0 1 0 ...
 * 
 * i.e. audio is sampled every second conversion, control channels share the second time
 * slot and are therefor sampled at lower rate.
 */
ISR(TIMER2_OVF_vect)
{
  /* We should do this division by adjusting the prescaler - leftover from original sketch */
  div32 = !div32;                      // divide timer2 frequency / 2 to 31.25kHz
  if (not div32) {
    return;
  }

  div16 = !div16;

  /* Sample audio channel and control channels alternately:
   *  -> audio is sampled at 15.6 kHz
   *  -> control channels are sampled at 15.6 kHz / 7 ~= 2.23 kHz
   */
  if (div16) {
    effect->m_adc[EwAudioEffect::ADC_CH_AUDIO] = ADCH;
    effect->m_avail = true;

    /* Select non-audio channel which will be sampled in the next period */
    selectAdcCh(otherCh + 1);
  } else {
    effect->m_adc[otherCh + 1] = ADCH;
    /* Step to next non-audio channel for the next non-audio sampling period */
    otherCh++;
    otherCh = otherCh >= EwAudioEffect::ADC_NUM_CH - 1 ? 0 : otherCh;
    /* Switch ADC to audio channel, since the next period is audio */
    selectAdcCh(EwAudioEffect::ADC_CH_AUDIO);
  }

  /* Some NOPs for ADC setup time */
  ibb++;
  ibb--; 
  ibb++; 
  ibb--;

  /* Start next conversion */
  sbi(ADCSRA, ADSC);
}

