/*  GameBoy VGM interpreter. THIS IS NOT A GAMEBOY SOUND EMULATOR
 *  This code should essentialy try to interpret GameBoy VGM data and play it using CTS7 / VixenSound.
 *  VixenSound has no Noise channel, so, a simple (and not thoroughly designed) placeholer is been implemented.
 */
#include "GameboySoundDriver.h"




// Select your track here
#include "vgm_data_6.h"





void setup(){
  GBSD::begin();
  pinMode(13, OUTPUT);
}

#define read8(x) pgm_read_byte_near(vgm_data + x)
#define read16(x) (read8(x) & (read8(x+1) << 8))
uint16_t cursor = 0;
uint16_t pause = 0;
unsigned long vgm_frame_millis;
unsigned long gbsd_frame_millis;
uint16_t noise_index = 0;
void loop(){
  if(cursor > 28000) cursor = 0;
  if(!pause){
    byte cmd = read8(cursor); cursor++;    
    /* Short pause Command 0x7n n = pause n frames */
    if(cmd & 0xF0 == 0x70) pause = (cmd & 0xf) + 1;      
    else{
      switch(cmd){
        /* GB Write (cursor+2) to (cursor+1) */
        case 0xB3:
          GBSD::write(read8(cursor), read8(cursor+1));
          cursor += 2;
          break;
        case 0x61:
          pause = (read8(cursor) + (read8(cursor+1) << 8));          
          cursor += 2;
          break;
        case 0x62:
          pause = 735;
          break;
        case 0x63:
          pause = 882;
          break;
        digitalWrite(13, HIGH);
      }
    }
  }    
  unsigned long temp_millis = micros();
  if(temp_millis - vgm_frame_millis >= 15){
    vgm_frame_millis = temp_millis;
    GBSD::clock();
    if(pause) pause--;    
  }
  CTS7::wave_ram[CTS7::SLOT_5 + ((noise_index++) & 0x1F)] = read8(noise_index);
}
