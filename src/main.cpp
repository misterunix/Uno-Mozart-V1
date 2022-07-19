#include <Arduino.h>

#include <avr/pgmspace.h>
#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator
#include <ADSR.h>
#include <Line.h>
#include <mozzi_midi.h>
#include <mozzi_fixmath.h>
#include <tables/sin2048_int8.h>
#include <mozzi_rand.h>

#include "mozart.h"

// use #define for CONTROL_RATE, not a constant
#define CONTROL_RATE 128 // powers of 2 please

// audio sinewave oscillator
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin1(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin2(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin3(SIN2048_DATA);


// envelope generator
ADSR <CONTROL_RATE, AUDIO_RATE> envelope1;
ADSR <CONTROL_RATE, AUDIO_RATE> envelope2;
ADSR <CONTROL_RATE, AUDIO_RATE> envelope3;


#define LED 13 // shows if MIDI is being recieved

unsigned long int microTime;
unsigned long int microTimeEvent;
bool eventType;

byte gain1,gain2,gain3,gain4 = 0;

byte song[MEASURES];
byte currentMeasure;
byte currentBeat;




void HandleNoteOn(byte channel, byte note, byte velocity) 
{ 

	switch( channel )
	{
	case 0:
		aSin1.setFreq_Q16n16(Q16n16_mtof(Q8n0_to_Q16n16(note)));
		//aSin1.setFreq(mtof(float(note)));
		envelope1.noteOn();    
		break;
	case 1:
		aSin2.setFreq_Q16n16(Q16n16_mtof(Q8n0_to_Q16n16(note)));
		//aSin2.setFreq(mtof(float(note)));
		envelope2.noteOn();    
		break;
	case 2:
		aSin3.setFreq_Q16n16(Q16n16_mtof(Q8n0_to_Q16n16(note)));
		//aSin3.setFreq(mtof(float(note)));
		envelope3.noteOn();    
		break;
	}


}

void HandleNoteOff(byte channel, byte note, byte velocity) 
{ 
	switch( channel )
	{
	case 0:
		envelope1.noteOff();
		break;
	case 1:
		envelope2.noteOff();
		break;
	case 2:
		envelope3.noteOff();
		break;
		
	}
}

void BuildSong( void )
{
	int i;

	for(i=0;i<16;i++)
	{
		song[i] = rand(0,10);
	}
	currentMeasure = 0;
	currentBeat = 0;
	microTimeEvent = microTimeEvent + 10000000;
}



void setup() 
{
	pinMode(LED, OUTPUT);  

	startMozzi(CONTROL_RATE);
	
	envelope1.setADLevels(90,64);
	envelope2.setADLevels(90,64);
	envelope3.setADLevels(90,64);
	
	envelope1.setTimes(90,200,1000,200); 
	envelope2.setTimes(90,200,1000,200); 
	envelope3.setTimes(90,200,1000,200); 
	
	BuildSong(); 
	microTime = mozziMicros();
	microTimeEvent = microTime + 220000;
	eventType = 1;
	randSeed();
	// Serial.begin(9600);
}


void updateControl()
{
	byte v1,v2,v3;

	if( (microTime = mozziMicros()) >=  microTimeEvent )
	{
		if( eventType )
		{
			digitalWrite(LED,HIGH);
		}else
		{
			digitalWrite(LED,LOW);
		}
		eventType = !eventType;

		microTimeEvent = microTime + 220000;

		v1 = pgm_read_byte(&(measure[currentMeasure][song[currentMeasure]][0][currentBeat]));
		v2 = pgm_read_byte(&(measure[currentMeasure][song[currentMeasure]][1][currentBeat]));
		v3 = pgm_read_byte(&(measure[currentMeasure][song[currentMeasure]][2][currentBeat]));

		/*
	Serial.print(currentMeasure); Serial.print(" ");
	Serial.print(song[currentMeasure]); Serial.print(" ");
	Serial.print(currentBeat); Serial.print(" ");

	Serial.print(v1); Serial.print(" ");
	Serial.print(v2); Serial.print(" ");
	Serial.println(v3);
	*/
		
		if( v1 != 255 ) 
		{
			HandleNoteOff(0, v1, 64); // only channel matters
			HandleNoteOn(0, v1, 64);
		}

		if( v2 != 255 ) 
		{
			HandleNoteOff(1, v2, 64); // only channel matters
			HandleNoteOn(1, v2, 64);
		}
		
		if( v3 != 255 ) 
		{
			HandleNoteOff(2, v3, 64); // only channel matters
			HandleNoteOn(2, v3, 64);
		}

		currentBeat++;
		if( currentBeat == 6 )
		{
			HandleNoteOff(0, 0, 0);
			HandleNoteOff(1, 0, 0);
			HandleNoteOff(2, 0, 0);
			currentMeasure++;
			currentBeat = 0;
		}

		if( currentMeasure == 16 )
		{
			HandleNoteOff(0, 0, 0);
			HandleNoteOff(1, 0, 0);
			HandleNoteOff(2, 0, 0);
			BuildSong();
		}

	}

	envelope1.update();
	envelope2.update();
	envelope3.update();


}


int updateAudio()
{
	long out;
	out = ( envelope1.next() * aSin1.next() + 
	envelope2.next() * aSin2.next() + 
	envelope3.next() * aSin3.next() ) >> 8;
	return( (int)out );
}


void loop() 
{
	audioHook(); // required here
} 


