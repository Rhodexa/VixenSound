/*  
  CTS7 VixenSound (Chiptune Studio 7 VixenSound) is a single-header polyphonic chiptune-oriented wavetable synthesizer, designed (mainly) for ATMEGA328P.
  Copyright (C) 2023 Rhodune! Lab.
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>


  Made with love, for you!
  /////////////////////////////////////////////////////////// Enough about the license // */

/*
  How does this work?

  CTS7 is based on a cyclic wavetable sampler system.
  
  It has 8 wavetable slots, each with a capacity of 32 point samples.
  Each sample is a single 8-bit integer.

  Depending on the sampler configuration, the samplers can use either 16, 32, 64, and even 128 samples by combining the slots.
  for example, using the 64 samples mode, a sampler pointing to Slot 0, will use both Slot 0 AND Slot 1 merged together to form a 64 entries wavetable.
  This is great for basses, which run at lower frequencies and requiere a higher quality sampling for smoother waves (that is, if you want smooth waves).
  This is done by modifying the 'wave_mask' of the sampler (voice[<index>].wave_mask).
  This is hard ti explain but easy mathematically.
  The sampler has something called a 'Phase Accumulator' or 'Phase Counter' (voice[<index>].phase). It is a 16-bit number that essentially counts up slow or real quick
  depending on the requiered frequency, and wraps around when it goes beyond 65535. Only the top byte of this two-byte integer are used for sampling. So the actual phase used is 
  an 8-bit number that counts up 256 times slower than the Phase Accumulator. This 8-bit number is AND'ed together with the wave_mask before using it to fetch to the wavetable sample.
  So, if you want the full 256 samples, you can set the wave_mask to 0xFF, if you want 128 samples, you set it to 0x7F; 64 samples, 0x3F; 32 samples, 0x1F; 16 samples, 0x0F; and so on.
  You can also exploit this to distort waves by setting the wave_mask to non-powers-of-two, like 0x1A, or 0x37. This will cause the wavetable pointer to jump around in weird ways.
  Beware that setting the sample count to a number higher than the following available samples at the location pointed by the wave_select property (voice[<index>].wave_select)
  will make the sampler try to fetch data from unwanted places.
    For example: Setting the wave_mask to 0xFF (256 samples) and the wave_select property to 32 (Slot 1) will make the sampler run from Slot 1 to Slot 7 and then it will go 'out of bounds'
    to wathever is after the wavetable slots. It will play just fine, but maybe not the wave you wanted. (This can be exploited to get werid unpredictable waves directly from arbitrary places within RAM)

  What can you touch? Why such a little function set?
  
  CTS7 is meant to be absolutely open to you. You can touch everything. I have provided you with some frequency setting tools because the math behind it is rather annoying...
  but other than that is up to you.
  Here are some things you may use often:
  
  > wave_ram[]                          This is where waves are meant to exist. It is a 256 byte long array or 8bit integers. It is splited in Slot 0 to 7, each 32 samples appart.
  > voice[<index>].wave_select          This is an offset into the wave_ram. It is added to the phase counter to get samples. Any number is valid. Using multiples of 32 will lead you to each slot (0 is slot 0, 32 is slot 1, 64 is slot 2, etc) 
  > voice[<index>].wave_mask            This is AND'ed together with voice[<index>].phase to get the sample pointer (wave_select is added after this process, so it doesn't get affected by it)  
  > voice[current_voice].amplitude_l/r  This controls the amplitude of the sound for the sampler/voice. Values range from 0 to 255. Note there are two 'amplitude': 'amplitude_l' and 'amplitude_r'


  The compilation flags can be used to choose how many samplers are created, and how they access the output ports (Pins 3 and 11).
  There are basically three modes of compilation:
  - (FULL_STEREO Mode)      All samplers can be sent to both Left and Right outputs, with independent volume control for each output (Similar to panning).
  - (ALTERNATE_STEREO Mode) Samplers can only be sent to one output; half samplers will be connected to the Left output, and the other half will be connected to the Right output permantly at compile time
  - (DISABLE_STEREO Mode)   All samplers are directed to Left output (Pin 3), and the Right output (Pin 11) is not computed at all, and remains physically disconnected.

*/



/* COMPILATION SETTINGS FLAGS — Change these to suit yours needs */
/* define them BEFORE including the file, as it's the first thing CTS will check for (I know this is isn't very elegant, but it is what it is for such a random project lol)*/

//#define CTS7_SET_VOICE_COUNT_2     // - If more than one is defined, the higher number has priority. If none are defined, defaults to 6
//#define CTS7_SET_VOICE_COUNT_4     // More voices lowers sound quality and also lowers the highest possible note. 
//#define CTS7_SET_VOICE_COUNT_6     // My recommendation is to either use 4 in full or alternate stereo modes,
//#define CTS7_SET_VOICE_COUNT_8     // or 6 in alternate stereo mode (Default)
//#define CTS7_SET_VOICE_COUNT_10
//#define CTS7_SET_FULL_STEREO       // - Allows each voice to access both left and right channels (Requieres more computing time)
//#define CTS7_SET_DISABLE_STEREO    // - Completely disconnects right channel, freeing Pin_3 and some compute time. Prioritized over CTS7_SET_FULL_STEREO
//#define CTS7_SET_HALF_REF_CLOCK    // - Halves sample rate, lowers sound quality and highest possible note in favor of compute time
//#define CTS7_SET_HALF_WAVERAM      // - Ditches the last 4 slots of Wave RAM, freeing 128bytes of system RAM. (In case you ever need it)


#ifndef _CTS7_H_
#define _CTS7_H_

#ifdef CTS7_SET_DISABLE_STEREO
  #ifdef CTS7_SET_FULL_STEREO
    #undef CTS7_SET_FULL_STEREO
  #endif
#endif

/* FUNCTION SET — In case you need to re-check what built-in functions are available */
namespace CTS7{  
  /* Initialize VixenSound */
  void begin();

  /* UTILITIES */  
  /* Creates a sine wave in the specified Wavetable RAM slot. Slot 0 is automatically filled with 64 samples at startup.
     SLOT: Index of the slot to fill. 0 — 7.
     SIZE: Options: 
        LOAD_64: Load 64 samples of a sine wave (Default)
        LOAD_32: Load 32 samples of a sine wave
     64 samples will fill two slots, so writing LOAD_64 to slot 0 will fill slots 0 and 1 */     
    void loadSineWave(byte slot, byte size);
    
    /* Returns the frequency for a given pitch number (0 means A4 — 440Hz) */
    float getFrequency(float pitch);
    
    /* Returns the tuning number for a given frequency */
    uint16_t getFnumber(float frequency);
  
    /* Does the conversion directly from pitch number to tuning number */
    uint16_t pitchToFnumber(float pitch);
}

namespace CTS7{  
  /* OPTIONS */
  enum {LOAD_64, LOAD_32};
  enum {
    SLOT_0 = 0,   SLOT_1 = 32,
    SLOT_2 = 64,  SLOT_3 = 96,
    SLOT_4 = 128, SLOT_5 = 160,
    SLOT_6 = 192, SLOT_7 = 224,
  };
  
  /* CORE CONSTANTS — Results from hardcoding these are undefined */
  const byte LEFT_OUTPUT  = 11;   /* No, you can`t change this pins, I only have them here for consistency. They are internally hardwired to Timer 2 */
  const byte RIGHT_OUTPUT = 3;
  #ifdef CTS7_SET_VOICE_COUNT_10
    const byte VOICE_COUNT = 10;
  #else
  #ifdef CTS7_SET_VOICE_COUNT_8
    const byte VOICE_COUNT = 8;
  #else
  #ifdef CTS7_SET_VOICE_COUNT_6
    const byte VOICE_COUNT = 6;
  #else
  #ifdef CTS7_SET_VOICE_COUNT_4
    const byte VOICE_COUNT = 4;
  #else
    #ifdef CTS7_SET_VOICE_COUNT_2
      const byte VOICE_COUNT = 2;
    #else
      const byte VOICE_COUNT = 6;
    #endif
  #endif
  #endif
  #endif
  #endif
  const byte LUT_QUARTER_SINE[] PROGMEM = {0x80, 0x8B, 0x98, 0xA4, 0xB0, 0xBB, 0xC6, 0xD0, 0xD9, 0xE2, 0xE9, 0xEF, 0xF5, 0xF9, 0xFC, 0xFE, 0xFF};
  auto& DAC_LEFT  = OCR2A;
  auto& DAC_RIGHT = OCR2B;
  #ifdef CTS7_SET_HALF_REF_CLOCK
    const unsigned int HALF_REF_CLOCK = 15625;
    #else    
    const unsigned int HALF_REF_CLOCK = 31250;
  #endif
  #ifdef CTS7_SET_HALF_WAVERAM
    const unsigned int WAVE_RAM_SIZE = 128;
    #else    
    const unsigned int WAVE_RAM_SIZE = 256;
  #endif
  
  /* CORE VARIABLES */
  byte wave_ram[WAVE_RAM_SIZE];
  struct Voice {
    uint16_t phase, tune = 0;
    uint8_t wave_select = 0;
    uint8_t wave_mask = 0x3F;
    #ifdef CTS7_SET_FULL_STEREO
      union{
        uint8_t amplitude_l;
        uint8_t amplitude;
      };
      uint8_t amplitude_r;
    #else
      union{
        uint8_t amplitude_l;
        uint8_t amplitude_r;
        uint8_t amplitude;
      };
    #endif    
  } voice[VOICE_COUNT];  
  byte current_voice = 0;
  #ifdef CTS7_SET_DISABLE_STEREO  
    struct Mixer { union {uint16_t left; uint8_t right;}; } mixer;    
  #else
    struct Mixer { uint16_t left, right; } mixer;
  #endif
  
  void begin(){
    #ifdef CTS7_SET_HALF_REF_CLOCK
      TCCR2A = 0xA1;
    #else
      TCCR2A = 0xA3;
    #endif
    #ifdef CTS7_SET_DISABLE_STEREO
      TCCR2A = TCCR2A & 0xCF;
    #endif
    TCCR2B = 0x01;
    TIMSK2 = 0x01;
    pinMode(LEFT_OUTPUT,  OUTPUT);
    #ifndef CTS7_SET_DISABLE_STEREO      
      pinMode(RIGHT_OUTPUT, OUTPUT);
    #endif
    loadSineWave(SLOT_0, LOAD_64);
  }

  /* Function definitions */
  /* I know some programmers don't like single-lining functions... but I don't see any reason not to do it in this specific case */
  float getFrequency(float pitch){return 440*pow(1.059463, pitch);}
  uint16_t getFnumber(float frequency){return frequency/((HALF_REF_CLOCK/VOICE_COUNT)/pow(2, 14));}
  uint16_t pitchToFnumber(float pitch){return getFnumber(getFrequency(pitch));}
  void loadSineWave(byte slot = SLOT_0, byte size = LOAD_64){
    size+=1;
    for(byte i = 0; i < 16/size; i++){
      byte i2 = i*size;
      wave_ram[i   +slot] = pgm_read_byte_near(LUT_QUARTER_SINE + i2);
      wave_ram[i+16+slot] = pgm_read_byte_near(LUT_QUARTER_SINE + 16 - i2);
      wave_ram[i+32+slot] = 0x100 - pgm_read_byte_near(LUT_QUARTER_SINE + i2);
      wave_ram[i+48+slot] = 0x100 - pgm_read_byte_near(LUT_QUARTER_SINE + 16 - i2);
    }
  }
}

/* Here's where the magic happens, of course. */
ISR(TIMER2_OVF_vect){
  using namespace CTS7;
  
  if(current_voice == VOICE_COUNT){
    current_voice = 0;     
    #ifdef CTS7_SET_DISABLE_STEREO
      DAC_LEFT  = mixer.left >> 8;
      mixer.left  = 0;
    #else
      DAC_LEFT  = mixer.left >> 8;
      DAC_RIGHT = mixer.right >> 8;
      mixer.left  = 0;
      mixer.right = 0;
    #endif
  }
  
  byte wave_address; 
  byte out;
  voice[current_voice].phase += voice[current_voice].tune;
  wave_address = ((voice[current_voice].phase >> 8) & voice[current_voice].wave_mask) | voice[current_voice].wave_select;
  out = wave_ram[wave_address];
  mixer.left  += out*voice[current_voice].amplitude_l;
  #ifdef CTS7_SET_FULL_STEREO
    mixer.right += out*voice[current_voice].amplitude_r;
  #endif
  current_voice++;
  
  voice[current_voice].phase += voice[current_voice].tune;
  wave_address = ((voice[current_voice].phase >> 8) & voice[current_voice].wave_mask) | voice[current_voice].wave_select;
  out = wave_ram[wave_address];
  mixer.right += out*voice[current_voice].amplitude_r;
  #ifdef CTS7_SET_FULL_STEREO
    mixer.left  += out*voice[current_voice].amplitude_l;
  #endif
  current_voice++;
}

#endif
