#ifndef _EW_EFFECT_H_
#define _EW_EFFECT_H_

#include <Arduino.h>

// Didsabled since it currently only contains the following typedef
//#include <EwAudio.h>
typedef int8_t Sample;

extern "C" void TIMER2_OVF_vect(void) __attribute__ ((signal));
  
class EwAudioEffect
{
public:
  enum {
    ADC_CH_AUDIO = 0,
    ADC_CH_1,
    ADC_CH_2,
    ADC_CH_3,
    ADC_CH_4,
    ADC_CH_5,
    ADC_CH_6,
    ADC_CH_7,

    ADC_NUM_CH,
  };
  typedef enum
  {
    CTRL_CH_1 = ADC_CH_1,
    CTRL_CH_2,
    CTRL_CH_3,
    CTRL_CH_4,
    CTRL_CH_5,
    CTRL_CH_6,
    CTRL_CH_7,
  } ControlChannel;
  
  EwAudioEffect()
    : m_adc{0}
    , m_avail(false)
  {}

  void begin();

  bool audioAvail() const
  {
    return m_avail;
  }
  int8_t audioRead()
  {
    m_avail = false;
    return (int8_t)m_adc[ADC_CH_AUDIO] - 128;
  }
  void audioWrite(int8_t sample)
  {
    OCR2A = sample + 128;
  }

  uint8_t controlRead(ControlChannel channel) const
  {
    return m_adc[channel];
  }
private:
  friend void :: TIMER2_OVF_vect(void);
 
  volatile uint8_t m_adc[ADC_NUM_CH];
  volatile bool m_avail;
};

#endif /* _EW_EFFECT_H_ */

