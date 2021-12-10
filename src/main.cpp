/* 
 *  Play with OSC routing. Test input comes from OSCSLIPsend.py
 *  
 *   "C:\Program Files (x86)\Arduino\hardware\tools\arm\bin\arm-none-eabi-addr2line" -e 
 */


#include <OSCMessage.h>
#include <OSCBundle.h>
#include <SLIPEncodedSerial.h>
#include <CrashReport.h>


#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include "OSCAudioBase.h"

// GUItool: begin automatically generated code
//AudioSynthWaveform       waveform1;      //xy=654,472
OSCAudioSynthWaveform    waveform1("waveform1");
OSCAudioMixer4           mixer1("mixer1");
AudioOutputUSB           usb;           //xy=977,476
OSCAudioOutputPT8211_2   pt8211("pt8211");
AudioConnection          patchCord1(waveform1, 0, usb, 0);
AudioConnection          patchCord2(waveform1, 0, mixer1, 0);
AudioConnection          patchCord3(mixer1, 0, pt8211, 0);
AudioConnection          patchCord4(mixer1, 0, pt8211, 1);
//OSCAudioControlSGTL5000     sgtl5000_1("sgtl5000");     //xy=977,519
// GUItool: end automatically generated code


#define USE_SLIP

// set this to the hardware serial port you wish to use
#define HWSERIALPORT Serial7

#if defined(USE_SLIP)
SLIPEncodedSerial HWSERIAL(HWSERIALPORT);
#else
#define HWSERIAL HWSERIALPORT
#endif // defined(USE_SLIP)

void setup() {
	Serial.begin(115200);
  while(!Serial)
    ;
Serial.println("OSC dynamic test started!");
  if (CrashReport)
  {
    Serial.println(CrashReport);
    CrashReport.clear();
  }
	HWSERIAL.begin(115200);
  HWSERIAL.setTimeout(100);

  //-------------------------------
  AudioMemory(10);
  //sgtl5000_1.enable();
  //sgtl5000_1.volume(0.1);
  //-------------------------------
}


void routeAudio(OSCMessage& msg, int addressOffset)
{
  Serial.println("audio message!");
  OSCAudioBase::routeAll(msg,addressOffset);
}


void routeDynamic(OSCMessage& msg, int addressOffset)
{
#if defined(SAFE_RELEASE)  
  Serial.println("dynamic objects message!");
  OSCAudioBase::routeDynamic(msg,addressOffset);
#else
  Serial.println("dynamic objects not available!");
#endif // defined(SAFE_RELEASE)  
}


void listObjects(void)
{
  OSCAudioBase* obj=OSCAudioBase::getFirst();

  while (NULL != obj)
  {
    Serial.println(obj->name);
    HWSERIAL.println(obj->name); // for now use this simple print
    
    obj = obj->getNext();
  }
}


// work with SLIP-protocol serial port:
void loop()
{
  OSCBundle  bundle;
  OSCMessage *msg;
  int msgLen;
  int msgCount;
  char prt[200];
  
  while (!HWSERIAL.endofPacket())
  {
    
    msgLen = HWSERIAL.available();
    while (msgLen--)
    {
      char c = HWSERIAL.read();
      bundle.fill((uint8_t) c);
    }
  }
  Serial.println();

  if (!bundle.hasError())
  {
    msgCount = bundle.size();
    for (int i = 0; i < msgCount; i++) {
      msg = bundle.getOSCMessage(i);
      msg->getAddress(prt);
      Serial.println(prt);
      Serial.flush();
      msg->route("/teensy*/audio*",routeAudio); // see if this object can use the message
      msg->route("/teensy*/dynamic*",routeDynamic); // see if this object can use the message
      
    }
    Serial.println("---------------------");
    HWSERIAL.println("---------------------");
    listObjects();
    Serial.println("=====================");
    HWSERIAL.println("=====================");
  }else {
      OSCErrorCode error = bundle.getError();
      HWSERIAL.print("bundle error"); 
      OSCMessage errorMsg("/error/bundle");
      
      if (error == OSCErrorCode::BUFFER_FULL)
        //errorMsg.add("BUFFER_FULL");
        HWSERIAL.println("BUFFER_FULL");
      else if (error == OSCErrorCode::INVALID_OSC)
        //errorMsg.add("INVALID_OSC");
        HWSERIAL.println("INVALID_OSC");
      else if (error == OSCErrorCode::ALLOCFAILED)
        //errorMsg.add("ALLOCFAILED");
        HWSERIAL.println("ALLOCFAILED");
      else if (error == OSCErrorCode::INDEX_OUT_OF_BOUNDS)
        //errorMsg.add("INDEX_OUT_OF_BOUNDS");
        HWSERIAL.println("INDEX_OUT_OF_BOUNDS");
    //errorMsg.add("some more data");
    //HWSERIAL.beginPacket();
      //errorMsg.send(HWSERIAL);
      //HWSERIAL.endPacket();
      //HWSERIAL.println("bundle error");
  }
  //bundle.empty();
}
