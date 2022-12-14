#ifndef RF_RECEIVE_CPP
#define RF_RECEIVE_CPP

void RF_receiver::rf_loop(){
  if (mySwitch.available()) {
    stm->broadcastRfButton(mySwitch.getReceivedValue());
    mySwitch.resetAvailable();
    lastRequest = 0;
  }
};


void RF_receiver::bt_loop(){
  int processed_chars = 0;
  while(Serial1.available()) {
    if(lastPos > 1998){
      Serial.println("AAAAAAA");
      return;
    }
    bt_message[lastPos++] = Serial1.read();
  }  
  if(lastPos > 10){
    if(memcmp(bt_message, "OK+DISA:", 8)==0){      
      #ifdef DEBUG_RF
        Serial.print("processing disa");
      #endif
      processed_chars = parseDisa((byte*)bt_message);
    }else if(lastPos > 0){
      Serial.print((char)bt_message[0]);
      Serial.print(' ');
      processed_chars = 1;
    };
  }
  
  memmove(bt_message, bt_message+processed_chars, lastPos-processed_chars);
  lastPos -= processed_chars;

  if (Serial.available()) {      // If anything comes in Serial (USB),
    Serial1.write(Serial.read());   // read it and send it out Serial1 (pins 0 & 1)
  }
  if(lastRequest == 0 || millis() - lastRequest >15000){
      Serial1.print("AT+IMME1");;Serial.print("imme");     
      lastRequest = millis();
      bt_step = 1;
  }
  if(bt_step > 0 && millis() - lastRequest > 2000 ){
    if(bt_step == 1 ){
      Serial1.print("AT+ROLE1");;Serial.print("role");
    }
    if(bt_step == 2 ){
      Serial1.println("AT+DISA?");Serial.println("AT+DISA?");
    }
    lastRequest = millis();
    if(++bt_step > 2) bt_step = 0; 
  }

};

int RF_receiver::parseDisa(byte* str){
//  AT+IMME1 and AT+ROLE1 AT+DISA? 
//OK+DISA:⸮q⸮x  ⸮⸮⸮⸮ ⸮    ⸮q⸮x        
//P0: Device MAC (6Byte)
//P1: Device Type(1Byte) 
//P2: Device RSSI(1Byte)
//P3: Rest Data Length 
// (1Byte) 
//P4: Rest Data 

  char tmp[400];

  while(lastPos<17){
    #ifdef DEBUG_RF
      Serial.print("Totally to short: ");
      Serial.println(lastPos);
    #endif
    return 0;
  }
  if(lastPos<17+str[16]){
    #ifdef DEBUG_RF
      Serial.print("Need more bytes to process body: ");
      Serial.print(lastPos);
      Serial.print(" vs ");
      Serial.println(17+str[16]);
    #endif
    return 0;
  }

  sprintf(tmp, "%02X:%02X:%02X:%02X:%02X:%02X", str[13], str[12], str[11], str[10], str[9], str[8] );
  Serial.print("MAC: ");
  Serial.print(tmp);
  Serial.print(" ");

  sprintf(tmp, "%02X", str[14] );
  Serial.print("Type: "); 
  Serial.print(tmp);
  Serial.print(" ");

  sprintf(tmp, "%02X", str[15] );
  Serial.print("RssI: "); 
  Serial.print(tmp);
  Serial.print(" ");

  sprintf(tmp, "%04d", str[16] );
  Serial.print("Len: "); 
  Serial.print(tmp);
  Serial.print("    ");

  sprintf(tmp,"");
  if(17+str[16] >=200){
    Serial.print("Fatal length ");
    Serial.print(lastPos);
    Serial.print(" vs ");
    Serial.println(17+str[16]);
    return 0;
  }
  for(int i=17; i<17+str[16]; i++)
  {
    sprintf(tmp, "%s %02X",tmp, str[i]);
    
    #ifdef DEBUG_RF
      Serial.print('.');
    #endif
  }
  short val1;
  sprintf(tmp, "%s %s ",tmp, str+17)  ;
  Serial.println( tmp );
  
  byte* data = &str[17];
  if(str[16] == 22){
    switch(data[15]){
      case(0x0D):
        val1 = (signed short)( ( data[19] << 8 ) | data[18] );
        Serial.print("Temp: ");Serial.print(val1);
        stm->broadcastClimate(str[8], Type::TEMP, val1);
        val1 = (signed short)( ( data[21] << 8 ) | data[20] );
        Serial.print(" Hum: ");Serial.println(val1);
        stm->broadcastClimate(str[8], Type::HUM, val1);
        
        break;
      case(0x0A):
        val1 = (signed short)( ( data[19] << 8 ) | data[18] );
        Serial.print("Bat: ");Serial.println(val1);
        stm->broadcastClimate(str[8], Type::BAT, val1);
        
        break;
    }
  }else if(str[16]==17){
    val1 = (signed short)( ( data[10] << 8 ) | data[11] );       
    Serial.print("Temp: ");Serial.print(val1);
    stm->broadcastClimate(str[8], Type::TEMP, val1);
    Serial.print("Hum: ");Serial.print(data[12]);
    stm->broadcastClimate(str[8], Type::HUM, data[12]*10);
    Serial.print("Bat: ");Serial.print(data[13]);
    stm->broadcastClimate(str[8], Type::BAT, data[13]);
    val1 = (signed short)( ( data[14] << 8 ) | data[15] );       
    Serial.print("Bat mv: ");Serial.print(val1);
    stm->broadcastClimate(str[8], Type::BATM, val1);
        
  }

  
  return 17+str[16];
}

#endif
