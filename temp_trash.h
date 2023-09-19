    if(current_frame & 1){
      length_clock_millis = current_millis;
      if(length_counter[0].counter & length_counter[0].enabled) length_counter[0].counter--;
      else {voice[0].amplitude = 0; voice[1].amplitude = 0;}
      if(length_counter[1].counter & length_counter[1].enabled) length_counter[1].counter--;
      else {voice[2].amplitude = 0; voice[3].amplitude = 0;}
      if(length_counter[2].counter & length_counter[2].enabled) length_counter[2].counter--;
      else {voice[6].amplitude = 0; voice[7].amplitude = 0;}      
    }
    if(current_frame & 2){
      sweep_clock_millis = current_millis;
      if((sweeper.shadow_frq + ((sweeper.negate*0xFFFF)^(sweeper.shadow_frq << sweeper.shift)) > 2047){
        voice[0].amplitude = 0;
        voice[1].amplitude = 0;
      }else{
        sweeper.shadow_frq += ((sweeper.negate*0xFFFF)^(sweeper.shadow_frq << sweeper.shift));
      }
    }
