/*  
 *   This file contains all the stuff necessary to drive VixenSound understanding the GB.
 *   Based on The GameBoy's Programming Manual. It's nicely detailed.
 *   
 *   This is not an emulator nor it plans to be.
 *   This is just a GameBoy Sound Registers INTERPRETER.
 *   To be used in conjuction with a VGM 'DATA' interpreter for GB systems
 */
 
// The GB has four voice channels capable of stereo.
// Voice Count 4 should give us nice quality
// Using Full Stereo for L/R gating should also make panning simple,
// although it means a bit more overhead for the Sound Processing ISR.
#define CTS7_SET_VOICE_COUNT_4
#define CTS7_SET_HALF_REF_CLOCK
#include "CTS7.h"

namespace GBSD{
  using namespace CTS7;
  byte waveform_ch_amplitude = 0;
  
  struct Envelope {
    byte starting_volume = 64;
    byte period = 10;
    byte mode = 0;
    byte counter;
  } envelope[3];

  struct LengthCounter {
    byte enabled = 0;
    byte length_load = 0;
    byte counter;
  } length_counter[4];

  struct Sweeper {
    byte period = 0;
    byte negate = 0;
    byte shift = 0;
    uint16_t shadow_frq = 0;
    byte enabled;
    byte value;
  } sweeper;
  
  void begin(){
    CTS7::begin();
    
    /*
      Prepare sound RAM
      The Gameboy has two PWM channels, capable of 12.5%, 25%, 50% and 75% duty-cycles.    
      These come from the documentation, duty cycles seem to have different phases.
    */
    byte level = 80;
    byte pulse125 = B01000000;     // 12.5% _-______
    byte pulse250 = B11000000;     // 25.0% --______
    byte pulse500 = B11110000;     // 50.0% ----____
    byte pulse750 = B00111111;     // 75.0% __------    
    for(int i = 0; i < 32; i++){
      wave_ram[SLOT_0 + i] = (pulse125 & (0x80 >> (i>>2))) ? level : 0;
      wave_ram[SLOT_1 + i] = (pulse250 & (0x80 >> (i>>2))) ? level : 0;
      wave_ram[SLOT_2 + i] = (pulse500 & (0x80 >> (i>>2))) ? level : 0;
      wave_ram[SLOT_3 + i] = (pulse750 & (0x80 >> (i>>2))) ? level : 0;
    }
    
    /* Prepare PWM voices to use 32 samples per cycle. */
    voice[0].wave_mask = 31;
    voice[1].wave_mask = 31;
    
    /* The Waveform channel also uses 32 samples */    
    voice[2].wave_mask = 31;
    voice[2].wave_select = 0x80;

    /* Noise channel ugly stuff */    
    voice[3].wave_mask = 31;
    voice[3].wave_select = 0xA0;
  }

  byte current_frame = 0;
  void clock(){
    if(current_frame == 5){
      if(envelope[2].period){
        if(envelope[2].counter) envelope[2].counter--;
        else{
          envelope[2].counter = envelope[2].period;
          if(voice[3].amplitude) voice[3].amplitude += envelope[2].mode ? 1 : -1;    
        }
      }
    }
    else if(current_frame == 6){
      if(envelope[0].period){
        if(envelope[0].counter) envelope[0].counter--;
        else{
          envelope[0].counter = envelope[0].period;
          if(voice[0].amplitude) voice[0].amplitude += envelope[0].mode ? 1 : -1;    
        }
      }
    }
    else if(current_frame == 7){
      if(envelope[1].period){
        if(envelope[1].counter) envelope[1].counter--;
        else{
          envelope[1].counter = envelope[1].period;
          if(voice[1].amplitude) voice[1].amplitude += envelope[1].mode ? 1 : -1;    
        }
      }
      current_frame = 255;
    }
    current_frame++;
  }

  unsigned int fnum[3];
  void write(byte reg, uint16_t data){
    reg += 0x10;
    if ((reg & 0xF0) == 0x30){
      wave_ram[SLOT_4 + (reg & 0xF)*2] = data&0xF0;
      wave_ram[SLOT_4 + (reg & 0xF)*2 + 1] = data<<4;
    }
    else{
      switch(reg){
        case 0x11:
          length_counter[0].length_load  = data & 0x3F;
          voice[0].wave_select = (data >> 1) & 0x60;
          break;
        case 12:
          envelope[0].starting_volume = data & 0xF0;
          envelope[0].period = data & 0x7;
          envelope[0].mode = (data & 0x8);
          break;
        case 0x13:
          fnum[0] = (fnum[0] & 0x700) | data;
          break;
        case 0x14:
          fnum[0] = (fnum[0] & 0x0FF) | ((data & 0x7) << 8);          
          if(data&0x80){
            voice[0].amplitude = envelope[0].starting_volume;
            voice[0].phase = 0;
            voice[0].tune = getFnumber(32768/(2048-fnum[0]));
          }else voice[0].amplitude = 0;
          break;
          
        case 0x16:
          voice[1].wave_select = (data >> 1) & 0x60;
          break;
        case 0x17:
          envelope[1].starting_volume = data & 0xF0;
          break;
        case 0x18:
          fnum[1] = (fnum[1] & 0x700) | data;
          break;
        case 0x19:
          fnum[1] = (fnum[1] & 0x0FF) | ((data & 0x7) << 8);
          if(data & 0x80){
            voice[1].amplitude = envelope[1].starting_volume;
            voice[1].phase = 0;
            voice[1].tune = getFnumber(32768/(2048-fnum[1]));
          } else voice[1].amplitude = 0;
          break;
          
        case 0x1C:
          if(data == 0x00) waveform_ch_amplitude = 0;
          else if(data == 0x20) waveform_ch_amplitude = 60;
          else if(data == 0x40) waveform_ch_amplitude = 30;
          else if(data == 0x60) waveform_ch_amplitude = 15;
          break;
        case 0x1D:
          fnum[2] = (fnum[2] & 0x700) | data;
          break;
        case 0x1E:
          fnum[2] = (fnum[2] & 0x0FF) | ((data & 0x7) << 8);
          if(data&0x80){
            voice[2].amplitude = waveform_ch_amplitude;
            voice[2].phase = 0;  
            voice[2].tune = getFnumber(8190/(2048-fnum[2]));         
          } else voice[2].amplitude = 0;
          break;

        case 0x22:
          voice[3].tune = data;
          break;
        case 0x21:
          envelope[2].starting_volume = (data & 0xF0) >> 2;
          break;
        case 0x23:
          if(data & 0x80){
            voice[3].amplitude = envelope[2].starting_volume;
          } else voice[3].amplitude = 0;
          break;
        default:
          break;
      }
    }
  }
}
