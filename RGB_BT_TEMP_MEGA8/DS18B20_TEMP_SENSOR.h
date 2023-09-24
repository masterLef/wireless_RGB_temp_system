#ifndef DS18B20_TEMP_SENSOR
#define DS18B20_TEMP_SENSOR

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//==========================setup connection=======================
#define DS18B20_PORT PORTD   //APO EDW RY8MIZEIS
#define DS18B20_DDR DDRD	 //TO PIN TOU uC
#define DS18B20_PIN PIND	 //POU 8A STELNEI/PERNEI DATA
#define DS18B20_DQ PD2		 // STO/APO TO MESAIO PIN TOU SENSOR

//commands
#define DS18B20_CMD_CONVERTTEMP 0x44
#define DS18B20_CMD_RSCRATCHPAD 0xbe
#define DS18B20_CMD_WSCRATCHPAD 0x4e
#define DS18B20_CMD_CPYSCRATCHPAD 0x48
#define DS18B20_CMD_RECEEPROM 0xb8
#define DS18B20_CMD_RPWRSUPPLY 0xb4
#define DS18B20_CMD_SEARCHROM 0xf0
#define DS18B20_CMD_READROM 0x33
#define DS18B20_CMD_MATCHROM 0x55
#define DS18B20_CMD_SKIPROM 0xcc
#define DS18B20_CMD_ALARMSEARCH 0xec

//stop any interrupt on read
#define DS18B20_STOPINTERRUPTONREAD 1


//==================================ds18b20 init============================
uint8_t ds18b20_reset(void)
{
	uint8_t check_val;

	//low for 480us
	DS18B20_DDR |= (1<<DS18B20_DQ);		 // kane to pin tou DDRn pou 8es --> exodo
	DS18B20_PORT &= ~ (1<<DS18B20_DQ);	 //kai kanto --> 0
	_delay_us(480);

										 //release line and wait for 60uS
	DS18B20_DDR &= ~(1<<DS18B20_DQ);	 //input
	_delay_us(60);						 //auto borw na to valw kai ligo parapanw px 62usec

	//get value from DS18B20 and wait 420us
	check_val = (DS18B20_PIN & (1<<DS18B20_DQ)); //afou perasan ta 60usec diavase thn check_val 
	_delay_us(420);

	// an h check_val == 1 meta apo 480usec tote proekypse sfalma
	// an h check_val == 0 meta apo 480usec tote ola kala
	return check_val;
}
//==========================================================================

//==================================write one bit===========================
void ds18b20_writebit(uint8_t write_bit)
{
	//kanto low gia 1us
	DS18B20_DDR |= (1<<DS18B20_DQ);			//kane to pin tou DDRn pou 8es --> exodo
	DS18B20_PORT &= ~ (1<<DS18B20_DQ);		//kai kanto --> 0
	_delay_us(1);

	// an 8eloume na steiloume '1', eleu8erwse thn grammh,  alliws h grammh menei '0' 
	// (logw ths pull-up otan eleu8erwnetai h grammh phgaine sta 5v ara logic '1')
	if(write_bit==1)
	{
			DS18B20_DDR &= ~(1<<DS18B20_DQ); //input
	}

	//perimene 60uS and kai eleu8erwse th grammh
	_delay_us(60);
	DS18B20_DDR &= ~(1<<DS18B20_DQ); //input
}
//============================read_one_bit==================================
uint8_t ds18b20_readbit(void)
{
	uint8_t read_bit=0;

	//low for 1uS
	DS18B20_DDR |= (1<<DS18B20_DQ);			//kane to pin tou DDRn pou 8es --> exodo
	DS18B20_PORT &= ~ (1<<DS18B20_DQ);		//kai kanto -->0
	_delay_us(1);

	//release line and wait for 14uS
	DS18B20_DDR &= ~(1<<DS18B20_DQ);		//kane to pin tou DDRn pou 8es --> eisodo
	_delay_us(14);

	//An to data pin einai '1' tote shmainei oti to BS18B20 esteile 1 alliws esteile 0
	if(DS18B20_PIN & (1<<DS18B20_DQ))
	{
			read_bit = 1;
	}
	//perimene gia 45usec and epestrepse thn timh pou edwse to BS18B20
	_delay_us(45);
	return read_bit;
}
//=============================write_one_byte===============================
void ds18b20_writebyte(uint8_t byte){
	uint8_t i=8;
	while(i--){
		ds18b20_writebit(byte&1);
		byte >>= 1;
	}
}
//==========================================================================

//==================================read one byte===========================
uint8_t ds18b20_readbyte(void){
	uint8_t i=8, byte=0;
	while(i--){
		byte >>= 1;
		byte |= (ds18b20_readbit()<<7);
	}
	return byte;
}
//==========================================================================

//===================================get_temperature========================
double ds18b20_gettemp(void) {
	uint8_t temperature_l;
	uint8_t temperature_h;
	double retd = 0;

	#if DS18B20_STOPINTERRUPTONREAD == 1
	cli();
	#endif

	ds18b20_reset(); //reset
	ds18b20_writebyte(DS18B20_CMD_SKIPROM);			 //skip ROM
	ds18b20_writebyte(DS18B20_CMD_CONVERTTEMP);		 //start temperature conversion

	while(!ds18b20_readbit());						 //wait until conversion is complete

	ds18b20_reset();								 //reset
	ds18b20_writebyte(DS18B20_CMD_SKIPROM);			 //skip ROM
	ds18b20_writebyte(DS18B20_CMD_RSCRATCHPAD);		 //read scratchpad where the temperature is saved

	temperature_l = ds18b20_readbyte();				 //read Temperature Low byte from scratchpad
	temperature_h = ds18b20_readbyte();				 //read Temperature High byte from scratchpad

	#if DS18B20_STOPINTERRUPTONREAD == 1
	sei();
	#endif

	//convert the 12 bit value obtained
	retd = (( ( temperature_h << 8 ) + temperature_l ) * 0.0625) - 6; //Auto pou afairw to telos einai dikh mou pros8hkh 
																		//  gia ry8mish ths 8ermokrasias sta DS18B20.
																		// kanwntas sygrish me kanoniko 8ermometro 
																		// metavallw auth thn timh mexri to DS18B20 na 
																		// deixei thn idia 8ermokrasia me to kanoniko 8ermometro
	return retd;
}
//==========================================================================
#endif