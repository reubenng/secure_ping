/*
	Library for interfacing with the ENC28J60 Ethernet IC
	Written by Patrick Berry:pb2g13@soton.ac.uk
*/

#include <avr/io.h>
#include <util/delay.h>
#include <config.h>
#include <ENC28J60.h>
//PIN SETUP
#define _PORT 			PORTB
#define _DDR			DDRB
#define _PIN 			PIND
#define CS_PIN_NUMBER	2		//arduino 	10
#define SCK_PIN_NUMBER	5		//			13
#define SO_PIN_NUMBER	3		//			11
#define SI_PIN_NUMBER	4		//			12
//PIN SETUP

#define DEL 50

//Pin Access Macros

#define _CS_PIN 	(0x01<<CS_PIN_NUMBER)
#define _SCK_PIN	(0x01<<SCK_PIN_NUMBER)
#define _SO_PIN 	(0x01<<SO_PIN_NUMBER)
#define _SI_PIN		(0x01<<SI_PIN_NUMBER)


#define SCK_LO _PORT &=~_SCK_PIN
#define SCK_HI _PORT |=_SCK_PIN

#define SO_LO _PORT &=~_SO_PIN
#define SO_HI _PORT	|=	_SO_PIN

#define CS_LO _PORT &=~ _CS_PIN
#define CS_HI SCK_LO;SO_LO; _PORT |= _CS_PIN;_delay_us(DEL);

#define SI _PIN & _SI_PIN
//Pin Access Macros


//Buffer Sizes			0x0000|    Transmit Buffer   |RX_BUFF_START|	Receive Buffer		|RX_BUFF_END|	Transmit Buffer		|0X1FFF
//Any memory (between 0x0000 and 0x1FFF) not defined as rx buffer is tx buffer (automatic)
#define RX_BUFF_START_H	0x10
#define RX_BUFF_START_L 0x00
#define RX_BUFF_END_H	0x1F
#define RX_BUFF_END_L	0xFF

//TX_BUFF values used for host access only, not for initialisation
#define TX_BUFF_START_H	0x00
#define TX_BUFF_START_L 0x00	//must be 0x00 for this implementation- see send_packet ETXND calculation
#define TX_BUFF_END_H	0x0F
#define TX_BUFF_END_L	0xFF
//Buffer Sizes


void SPI_init(){
	_DDR |= _CS_PIN | _SCK_PIN | _SO_PIN;
	_DDR &= ~_SI_PIN;
	_PORT |= _SI_PIN;
	CS_HI;
	SO_LO;
}

void byte_out(uint8_t* ptr){									//for number of bytes specified
	int i=0;
	for(i=8;i>0;i--){									//for each bit 			
		SCK_LO;
		_delay_us(DEL);
		if(*ptr & (0x01<<(i-1)))SO_HI;
		else SO_LO;									//set SO pin to value of bit
		_delay_us(DEL/2);								//pulse clock
		SCK_HI;		
		_delay_us(DEL);								//pulse clock
	}
	SO_LO;
	return;
}

void byte_in(uint8_t* ptr){								//for number of bytes specified
	*ptr=0;												//clear existing data in buffer
	int i=0;
	for(i=8;i>0;i--){									//for each bit
		SCK_LO;
		_delay_us(DEL);
		_delay_us(DEL/2);
		*ptr|=(SI)? (0x01<<(i-1)):0;						
		SCK_HI;
		_delay_us(DEL);
	}
	return;
}

void RCR(uint8_t* ptr, cont_reg cr){ //ptr to register for result, uses 1 register
	if((cr<0x80)){					//if target is not universal register
		*ptr=0xA0|(ECON1&0x1F);		//write BFC(ECON1) command
		CS_LO;
		byte_out(ptr);				//send command
		*ptr=0x03;					//write argument- clear bank select bits
		byte_out(ptr);				//send arg
		CS_HI;
		*ptr=0x80|(ECON1&0x1F);		//write BFS(ECON1) command
		CS_LO;
		byte_out(ptr);				//send command
		*ptr=cr>>5;					//write arg
		byte_out(ptr);				//send arg
		CS_HI;
	}
	*ptr=0x00|(cr&0x1F);	
	CS_LO;
	byte_out(ptr);
	byte_in(ptr);
	CS_HI;
	return;
}

void WCR(uint8_t* ptr, cont_reg cr){
	uint8_t v=*ptr;
	if((cr<0x80)){					//if target is not universal register
		*ptr=0xA0|(ECON1&0x1F);		//write BFC(ECON1) command			//BF
		CS_LO;
		byte_out(ptr);				//send command
		*ptr=0x03;					//write argument- clear bank select bits
		byte_out(ptr);				//send arg
		CS_HI;
		*ptr=0x80|(ECON1&0x1F);		//write BFS(ECON1) command			//9F
		CS_LO;
		byte_out(ptr);				//send command
		*ptr=cr>>5;					//write argument - set correct bank select bits
		byte_out(ptr);				//send arg
		CS_HI;
	}
	*ptr=0x40|(cr&0x1F);				//write WCR(cr) command (ignoring bank specifier)
	CS_LO;
	byte_out(ptr);					//send command
	*ptr=v;							//write argument - new control register value
	byte_out(ptr);					//send argument
	CS_HI;
	return;
}

void BFS(uint8_t* ptr, cont_reg cr){	//ptr to >|| value || free ||
	uint8_t v=*ptr;
	if((cr<0x80)){					//if target is not universal register
		*ptr=0xA0|(ECON1&0x1F);		//write BFC(ECON1) command
		CS_LO;
		byte_out(ptr);				//send command
		*ptr=0x07;					//write argument- clear bank select bits
		byte_out(ptr);				//send arg
		CS_HI;
		*ptr=0x80|(ECON1&0x1F);		//write BFS(ECON1) command
		CS_LO;
		byte_out(ptr);				//send command
		*ptr=cr>>5;					//write argument - set correct bank select bits
		byte_out(ptr);				//send arg
		CS_HI;
	}
	*ptr=0x80|(cr&0x1F);	
	CS_LO;
	byte_out(ptr);
	*ptr=v;
	byte_out(ptr);
	CS_HI;
	return;
}

void BFC(uint8_t* ptr, cont_reg cr){	//ptr to >|| value || free ||
	uint8_t v=*ptr;
	if((cr<0x80)){					//if target is not universal register
		*ptr=0xA0|(ECON1&0x1F);		//write BFC(ECON1) command
		CS_LO;
		byte_out(ptr);				//send command
		*ptr=0x07;					//write arg
		byte_out(ptr);				//send arg
		CS_HI;
		*ptr=0x80|(ECON1&0x1F);		//write BFS(ECON1) command
		CS_LO;
		byte_out(ptr);				//send command
		*ptr=cr>>5;					//write arg
		byte_out(ptr);				//send arg
		CS_HI;
	}
	*ptr=0xA0|(cr&0x1F);	
	CS_LO;
	byte_out(ptr);
	*ptr=v;
	byte_out(ptr);
	CS_HI;
	return;
}

void RBM(uint8_t* ptr){ //pointer to >|| data size || data
	uint8_t s=*ptr;
	ptr++;
	*ptr=0x3A;
	CS_LO;
	byte_out(ptr);
	for(;s>0;s--){
		byte_in(ptr);
		ptr++;
	}
	CS_HI;
	return;
}

void WBM(uint8_t* ptr){ // pointer to >|| data size || data
	uint8_t s=*ptr;
	*ptr=0x7A;
	CS_LO;
	byte_out(ptr);
	*ptr=s;
	ptr++;
	for(;s>0;s--){
		byte_out(ptr);
		ptr++;
	}
	CS_HI;
	return;
}

void SRC(void){
	uint8_t c=0xFF;
	CS_LO;
	byte_out(&c);
	CS_HI;
	return;
}


void ENC_init(uint8_t* ptr){
	SPI_init();
	SRC();
	//Receive Buffer Start
	*ptr=RX_BUFF_START_H;
	WCR(ptr,ERXSTH);

	*ptr=RX_BUFF_START_L;
	WCR(ptr,ERXSTL);

	//Receive Buffer End
	*ptr=RX_BUFF_END_H;
	WCR(ptr,ERXNDH);
	*ptr=RX_BUFF_END_L;
	WCR(ptr,ERXNDL);	

	//Receive filters
	*ptr=0x00;			//promiscuous mode
	WCR(ptr,ERXFCON);


/*	wait disabled for testing
	while(!(*ptr&0x01)){	//while CLKRDY bit not set
		RCR(ptr,ESTAT);		
	}
	*/
	_delay_us(500);


	//Enable MAC RX, Pause control frame TX & RX
	*ptr=0x0D;
	WCR(ptr,MACON1);

	//Enable full duplex, automatic Pad & CRC, disable proprietary header & huge frames
	*ptr=0xF1;
	WCR(ptr,MACON3);

	*ptr=0x01;
	*ptr=0x00;
	WPR(ptr,PHCON1);


	//disable defer transmission & no backoff (apply to half duplex only)
	*ptr=0x00;
	WCR(ptr,MACON4);

	//set maximum frame length
	*ptr=0x00;
	WCR(ptr,MAMXFLH);
	*ptr=0xFF;
	WCR(ptr,MAMXFLL);

	//set back-to-back inter packet gap
	*ptr=0x15;
	WCR(ptr,MABBIPG);

	//set non back-to-back inter packet gap
	*ptr=0x12;
	WCR(ptr,MAIPGL);

	//enable auto increment, disable power saving modes
	*ptr=0x80;
	WCR(ptr,ECON2);


	//set local MAC address
	WCR(ptr+1,MAADR1);
	WCR(ptr+2,MAADR2);
	WCR(ptr+3,MAADR3);
	WCR(ptr+4,MAADR4);
	WCR(ptr+5,MAADR5);
	WCR(ptr+6,MAADR6);

	return;

}

void WPR(uint8_t* ptr, phy_reg addr){
	WCR(ptr,MIWRH);
	*ptr=addr;
	WCR(ptr,MIREGADR);
	WCR((ptr+1),MIWRL);
	return;
}

void RPR(uint8_t* ptr, phy_reg addr){
	*ptr=addr;
	WCR(ptr,MIREGADR);
	*ptr=0x01;
	BFS(ptr,MICMD);
	while((*ptr)&0x1){
		RCR(ptr,MISTAT);
	}
	*ptr=0x01;
	BFC(ptr,MICMD);
	RCR(ptr,MIRDH);
	RCR((ptr+1),MIRDL);
	return;
}


void send_packet(uint8_t* ptr){

	uint8_t s= (*ptr)+0x0E;		//add size of MAC address & type fields to size

		
	*ptr=TX_BUFF_START_H;		
	WCR(ptr,ETXSTH);			//set transmit start pointer (high byte)
	WCR(ptr,ETXNDH);			//set TX end point (high byte)
	WCR(ptr,EWRPTH);			//set buffer write pointer (high byte)
	WCR(ptr,ERDPTH);			//set buffer read pointer (high byte)

	(*ptr)=TX_BUFF_START_L;
	WCR(ptr,ETXSTL);			//set transmit start pointer & write pointer to start of transmit buffer
	WCR(ptr,EWRPTL);			//set buffer write pointer (low byte)
	WCR(ptr,ERDPTL);

	*ptr+=(s+1);
	WCR(ptr,ETXNDL);			//set transmit end pointer (low byte)

	//(*ptr)++;
	//WCR(ptr,ERDPTL);			//set read pointer for status vector

	
	uint8_t t= *(ptr+1);
	*ptr=0x01;
	*(ptr+1)=0x00;				//per-packet control byte
	WBM(ptr);					//write per-packet control byte

	*ptr=s;
	*(ptr+1)=t;
	WBM(ptr);					//write packet to buffer

	(*ptr)=0x08;
	BFC(ptr,EIR);				//clear EIR:TXIF

	//to enable interrupt when sent:
	//*ptr=0x88;
	//BFS(ptr,EIE);				//set EIE:INTIE & EIE:TXIE

	(*ptr)=0x08;
	BFS(ptr,ECON1);

	while(*ptr & 0x08){
		RCR(ptr,ECON1);
		_delay_ms(2);
	}


	//*ptr=0x07;
	//RBM(ptr);
	

	return;
}