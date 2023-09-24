#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>  	 //library pou periexei usart interrupt

//#include "LCD4BIT.h"			 //library gia LCD_4_BIT
#include "light_ws2812.h"  		 //library gia ws2812
#include "light_ws2812.c"  		 //library gia ws2812
#include "DS18B20_TEMP_SENSOR.h" //library gia 1-wire Temp Sensor
#include <stdlib.h>				 //library pou periexei thn atoi() gia metatroph character array se integer number


volatile unsigned char interface_var; // auth h global variable einai o syndetikos krikos metaxy interrupt kai kyriws programmatos
									  //etsi wste to neo mode pou epilex8hke apo xrhsth na diavastei apo kyriws programma
									  
volatile unsigned char inter_interface_var= 0;

void usart_init(void)
{
	UCSRB = (1<<TXEN) | (1<<RXEN); // enable receive transmit of usart
	UCSRC = (1<<UCSZ1) | (1<<UCSZ0) | (1<<URSEL);
	UBRRL = 0x33; // Gia baudrate = 9600 kai Fosc=16MHz to UBRR value = 104 (0x68)
}
void usart_send(unsigned char ascii)
{
	while(!(UCSRA & (1<<UDRE)));
	UDR = ascii;
}
unsigned char usart_receive(void)
{
	while (!(UCSRA & (1<<RXC)));
	return UDR;
}
void inform(char message[]) // Molis o xrhsths epilexei mode, to programma enhmerwnei ton xrhsth gia thn epilogh tou
{
	 int a = 0;
	
	usart_send('*'); // To keimeno metaxy *A... kai... * 
	usart_send('A'); //diavazetai apo to terminal tou app (to gramma A borei na allax8ei apo to app)
	while(message[a] != '\0')
	{
		usart_send(message[a]);
		a++;
	}
	usart_send('*');
	
	//LCD_string2x16(message);
	//_delay_ms(80);
	//LCD_clear_screen();
}
struct cRGB led[1]; //8elw na leitourgw ena ws2812

int main(void)
{
  //====================================================================================================================
   unsigned char in=0, i=0;
   int red=0;
   int green=0;
   int blue=0;
   unsigned char check_mode ; // to check_mode einai mono gia thn arxh gia thn epilogh tou 1ou mode xrwmatos
   double dec = 0;
   
   char pinax[5]={0,0,0,0,0};							// char *matrix = NULL ;
   char matrix[5]={0,0,0,0,0};							// char *pinax = NULL ;
   char m_ascii[7] = {0,0,0,0,0,0,0};   //unsigned
   
   for( unsigned char t=0; t<10; t++)
   {
		dec = ds18b20_gettemp();
		_delay_ms(10);
   }
   dec = 0;
    char mode[17]="Give Mode 1 to 4"; // mhnyma pros xrhsth
    char mode1[17]= "=====Mode1======";    	//const char
    char mode2[17]= "=====Mode2======";		//const char
    char mode3[17]= "=====Mode3======";		//const char
    char mode4[17]= "=====Mode4======";		//const char
   //==================================================================================================================== 
  // LCD_init();  // Arxikopoihsh ths LCD (an den ginei auto h LCD den douleuei swsta)
   usart_init(); //Arxikopoihsh ths seiriakhs 8yras
   //====================================================================================================================
   led[0].r=50;led[0].g=0;led[0].b=0;		
	ws2812_setleds(led,1);
   _delay_ms(200);
   
   led[0].r=0;led[0].g=50;led[0].b=0;	
	ws2812_setleds(led,1);
   _delay_ms(200);
   
   led[0].r=0;led[0].g=0;led[0].b=50;		
	ws2812_setleds(led,1);
   _delay_ms(200);

   led[0].b=0;
	ws2812_setleds(led,1);
	
	inform(mode); // enhmerwse ton xrhsth/app gia ta modes pou borei na epilexei
	//====================================================================================================================
	check_mode  = usart_receive(); // perimene gia xarakthra apo xrhsth/app
	
	while( check_mode == 'R' || check_mode == 'G' || check_mode == 'B' ) // Elegxos xarakthra pou stal8hke apo app
	{										//an o xrhsths kinhsei mia apo tis bares anti na dialexei mode xrwmatos
			while(check_mode != 'A')// to programma petaei tous epomenous xarakthres mexri na vrei to 'A'
			{
				check_mode  = usart_receive(); 
			}
		check_mode  = usart_receive();	// kai xanaperimenei xarakthra. An einai 1,2,3 h 4 paei parakatw. An oxi 
	}							// tote xanakanei thn loopa mexri na dw8ei apo xrhsth-malaka, swstos xarakthras
	//====================================================================================================================			
	
	while(1) //Apo edw kai meta to programma baine se synexomeno loop opou,
	{ 
		//----------------------------------------------------------------------------------------------------------------
		if(check_mode == '1' || interface_var=='1') //(1) Ean epilex8hke apo xrhsth app o xarakthras '1' 
		{ inform(mode1);           				 //(2) enhmerwse ton xrhsth/app gia to mode pou epelexe
			while(1)
			{	for(i=0;i<255;i++)        //(3) Kane ena kyklo xrwmatwn gia green kai blue leds
				{
					led[0].b=i;led[0].g=255-i;
					ws2812_setleds(led,1);
					_delay_ms(5);
				}
				for(i=0;i<255;i++)
				{
					led[0].b=255-i;led[0].g=i;
					ws2812_setleds(led,1);
					_delay_ms(5);
				}
				
				UCSRB |= (1<<RXCIE);  // (4)ystera energopoihse (gia ligo) thn interrupt gia usart receive kai ...
				sei(); 
				if(interface_var =='2' || interface_var =='3' ||interface_var =='4') //(4.a)...efoson exei er8ei kapoios 
				{ 													//xarakthras ston buffer gia allagh mode (px'2')
					cli(); 								// (4.b)kleise thn interrupt
					UCSRB &= ~(1<<RXCIE);				// (4.c)apenergopoihse thn
					check_mode = interface_var;
					led[0].r=0;led[0].g=0;led[0].b=0; 	//(4.d) svhse ola ta leds
					ws2812_setleds(led,1);
					break;								//(4.e)kai vges apo thn while tou check_mode =='1' 
				} 										// gia na pas sto neo mode pou epelexe o xrhsths
				
				cli(); 					//(5)diaforetika ean den exei path8ei kapoio neo/allo mode 
				UCSRB &= ~(1<<RXCIE);	//(6)apenergopoihse thn interrupt gia usart kai kane allo ena kyklo ta g/b leds 
				
				if(interface_var =='T')  //(7)  An h interrupt diavase xarakthra 'T'
				{
					dec = ds18b20_gettemp(); 		// Zhta apo to DS18B20 na dwsei 8ermokrasia se double
					dtostrf(dec,5,2,m_ascii); 		// Kane ton double ari8mo se morfh string/ascii char matrix
					inform("::::::");				// typwse autes tis teleies
					inform(m_ascii);				// kai steile thn ascii 8ermokrasia ston xrhsth
					inform(":::::");				// kai typwse kai autes tis teleies
					interface_var = '1';			// gia sigouria xanavale sthn interface_var to mode pou eixe
				}
				
				
			} //(8) h diadikasia auth  8a epanalamvanetai gia to mode 1 mexri na path8ei diaforetiko mode apo tou app.
		}
		//----------------------------------------------------------------------------------------------------------------
		if(check_mode == '2' || interface_var=='2') // Ean epilex8hke arxika apo xrhsth tou app o xarakthras '2'
		{											 //h an o xrhsths epelexe neo mode leitourgias meta thn 1h epilogh
			inform(mode2);		 // enhmerwse ton xrhsth gia to mode pou epelexe		      

			while(1)
			{	for(i=0;i<255;i++)
				{
					led[0].b=i;led[0].r=255-i;
					ws2812_setleds(led,1);
					_delay_ms(5);
				}
				for(i=0;i<255;i++)
				{
					led[0].b=255-i;led[0].r=i;
					ws2812_setleds(led,1);
					_delay_ms(5);
				}
				
				UCSRB |= (1<<RXCIE);
				sei();
				
				if(interface_var =='1' || interface_var =='3' || interface_var =='4') 
				{ 														
					cli();
					UCSRB &= ~(1<<RXCIE);
					check_mode = interface_var;
					led[0].r=0;led[0].g=0;led[0].b=0;
					ws2812_setleds(led,1);	
					break;
				} 
				
				cli();
				UCSRB &= ~(1<<RXCIE);
				
				if(interface_var =='T')
				{
					dec = ds18b20_gettemp();
					dtostrf(dec,5,2,m_ascii);
					inform("::::::");
					inform(m_ascii);
					inform(":::::");
					interface_var = '2';
				}
			}
		}
		//----------------------------------------------------------------------------------------------------------------
		if(check_mode == '3' || interface_var=='3') // Ean epilex8hke arxika apo xrhsth tou app o xarakthras '3'
		{	inform(mode3);
			while(1)
			{	for(i=0;i<255;i++)
				{
					led[0].r=i;led[0].g=255-i;
					ws2812_setleds(led,1);
					_delay_ms(5);
				}
				for(i=0;i<255;i++)
				{
					led[0].r=255-i;led[0].g=i;
					ws2812_setleds(led,1);
					_delay_ms(5);
				}
				
				UCSRB |= (1<<RXCIE);
				sei();
				
				if(interface_var =='1' || interface_var =='2' || interface_var =='4') 
				{ 
					cli();
					UCSRB &= ~(1<<RXCIE);
					check_mode = interface_var;
					led[0].r=0;led[0].g=0;led[0].b=0;
					ws2812_setleds(led,1);
					break;
				} 
				
				cli();
				UCSRB &= ~(1<<RXCIE);
				
				if(interface_var =='T')
				{
					dec = ds18b20_gettemp();
					dtostrf(dec,5,2,m_ascii);
					inform("::::::");
					inform(m_ascii);
					inform(":::::");
					interface_var = '3';
				}
			}	
		}
		//----------------------------------------------------------------------------------------------------------------		
	    if(check_mode =='4' || interface_var=='4') // Ean epilex8hke arxika apo xrhsth tou app o xarakthras '4'
		{
			inform(mode4); // Enhmerwse xrhsth gia mode pou epelexe
			while(1)
			{
				i=0;
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
					in  = usart_receive(); // to programma preimenei xarakthra apo xrhsth. Mexri tote kollaei edw
					
					if(in =='T')
					{
						dec = ds18b20_gettemp();
						dtostrf(dec,5,2,m_ascii);
						inform("::::::");
						inform(m_ascii);
						inform(":::::");
						interface_var = '4';
					}
					
					if(in == '1' || in == '2' || in == '3') // An o xarakthras einai 1,2 h 3 allazei mode
					{		                       //afou to programma vgei apo to mode 4 8a prepei na xerei se poio mode 8a paei
						check_mode = in;     // etsi vazoume sto check_mode to neo mode pou epilex8hke apo xrhsth
						interface_var = in;  // kanw to idio kai gia interface_var
						in = 0; // mhdenise kai to in gia sigouria
						break; //kai vges apo mode 4
					}
					
					if(in == 'R' || in == 'G' || in == 'B')
					{ pinax[0] = in;
						do
						{
						  in  = usart_receive();   // Perimene mexri na er8ei xarakthras kai valton
						  pinax[i+1]= in;         	 // ston pinax[]  
						  i++;					
						}while(in !='A');        //mexri na vreis ton xarakthra 'A' vale sthn epomenh 8esh tou pinax ton epomeno xarak.
					}							// o kwdikos anagnwrishs twn xrwmatwn einai RxxxA h GxxxA h BxxxA opou xxx--> 0-255
				
					//LCD_string2x16(pinax);
					//_delay_ms(1000);
					//LCD_clear_screen();
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
												//Des apo to 1o stoixeio tou pinax[] an to xrwma pou 8a allax8ei einai R,G h B
				if((pinax[0]== 'R'))           // To R einai o xarakthras enarxhs ths string tou kokkinou led pou mas endiaferei
				{ i=0;  
					while((pinax[i+1] !='A'))    // Egw thelw na metatrepsw to ari8mhtiko kommati tou pinax[] se integer 
					{						      // px an er8oun apo uart oi characters R123A thelw oi chars '1' '2' '3' 
						matrix[i] = pinax[i+1];  // na metatrapoun se int--->123. Etsi efoson o prwtos char den einai ari8mos pote 
						i++; 					  // xekinw na vazw ston matrix[] to kommati tou pinax[] pou 8elw apo to 2o stoixeio	  
												  // tou pinax[] pou einai kai to prwto noumero pou me endiaferei mexri na diavasw 'A'
					}
					i=0;
					//LCD_string2x16(matrix);
					//_delay_ms(1000);
					//LCD_clear_screen();
				
					red = atoi(matrix);		    //twra metatrepw to ari8mhtiko kommati apo char array se integer me thn atoi()  
					led[0].r=red;              // kai vazw ton ari8mo auto pleon sthn struct pou ry8mizei thn fwteinothta tou R led
					ws2812_setleds(led,1);
				}
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				if((pinax[0]== 'G'))
				{i=0;
					while((pinax[i+1] !='A'))
					{
						matrix[i] =pinax[i+1]; 
						i++; 
								
					}
					i=0;
					//LCD_string2x16(matrix);
					//_delay_ms(1000);
					//LCD_clear_screen();
				
					green = atoi(matrix);				  
					led[0].g=green;
					ws2812_setleds(led,1);
				}
				//LCD_string2x16(pinax);
				//_delay_ms(1000);
				//LCD_clear_screen();
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				
				if((pinax[0]== 'B'))
				{i=0;
					while((pinax[i+1] !='A'))
					{
						matrix[i] =pinax[i+1]; 
						i++; 
								
					}
					i=0;
					//LCD_string2x16(matrix);
					//_delay_ms(1000);
					//LCD_clear_screen();
					
					blue = atoi(matrix);				  
					led[0].b=blue;
					ws2812_setleds(led,1);
				}
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				for (int f=0;f<5;f++) //Ka8arizw tous pinakes gia to epomeno loop
					{
					  pinax[f] = 0;
					  matrix[f] = 0;
					}
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			}//while(1) close	
		}//if(check_mode == '4') close
		
		if(check_mode =='T')   // An sthn arxh path8ei h 8ermokrasia anti gia kapoio mode xrwmatos 
		{
			/* for( unsigned char t=0; t<30; t++)
			{
				dec += ds18b20_gettemp();
				dec = dec/30;
				_delay_ms(100);
			}*/
			
			dec = ds18b20_gettemp(); // deixnei thn 8ermokrasia ston xrhsth
			dtostrf(dec,5,2,m_ascii);
			inform("::::::");
			inform(m_ascii);
			inform(":::::");
			//check_mode = 0;
			//interface_var= 0;
			
			UCSRB |= (1<<RXCIE);    //to programma energopoiei thn interrupt
			sei();
			while ( check_mode =='T' ) // kai elegxei mexri na path8ei apo ton xrhsth kapoio mode xrwmatos (1-4)
			{					// molis path8ei to mode apo xrhsth o xarakthras erxetai seiriaka ston serial buffer 
								//h interrupt xypnaei to pianei kai to vazei sthn interface_var
											
				if(interface_var =='1' || interface_var =='2' || interface_var =='3' || interface_var =='4') 
				{ 														
					cli(); // Ean h interface_var einai 1 h 2 h 3 h 4 tote to programma apenergopoiei thn interrupt
					UCSRB &= ~(1<<RXCIE);
					break; // vgainei apo thn if( check_mode == 'T') sthn kentrikh while (1) kai phgainei sto neo mode 
				}			//pou epilex8hke apo ton xrhsth
			}
		}
		 
	}//while(1) close
}// main close
//=======================================================================================================
ISR(USART_RXC_vect) // Gia na er8ei to programma sthn interrupt shmainei oti yparxei neos xarakthras
{					// ston buffer ths usart. Autos o xarakthras prepei na einai to neo mode  
	//cli();				//pou epelexe o xrhsths apo to app.
	interface_var = UDR;	//o neos xarakthras metaferetai sthn global variable mesw ths opoias 
	//sei();			 	//to programma apofasizei se poio mode 8a leitourghsei
}
//=======================================================================================================