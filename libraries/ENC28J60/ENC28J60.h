/*
	Library for interfacing with the ENC28J60 Ethernet IC
	Written by Patrick Berry:pb2g13@soton.ac.uk
*/

#ifdef __cplusplus
extern "C" {
#endif


typedef enum{
	//Universal
	EIE			=0xFB,
	EIR			=0xFC,
	ESTAT		=0xFD,
	ECON2		=0xFE,
	ECON1		=0xFF,

	//Bank 0 - 000xxxxx
	ERDPTL		=0x00,
	ERDPTH		=0x01,
	EWRPTL		=0x02,
	EWRPTH		=0x03,
	ETXSTL		=0x04,
	ETXSTH		=0x05,
	ETXNDL		=0x06,
	ETXNDH		=0x07,
	ERXSTL		=0x08,
	ERXSTH		=0x09,
	ERXNDL		=0x0A,
	ERXNDH		=0x0B,
	ERXRDPTL	=0x0C,
	ERXRDPTH	=0x0D,
	ERXWRPTL	=0x0E,
	ERXWRPTH	=0x0F,
	EDMASTL		=0x10,
	EDMASTH		=0x11,
	EDMANDL		=0x12,
	EDMANDH		=0x13,
	EDMADSTL	=0x14,
	EDMADSTH	=0x15,
	EDMACSL		=0x16,
	EDMACSH		=0x17,

	//Bank 1 - 001xxxxx
	EHT0		=0x20,
	EHT1		=0x21,
	EHT2		=0x22,
	EHT3		=0x23,
	EHT4		=0x24,
	EHT5		=0x25,
	EHT6		=0x26,
	EHT7		=0x27,
	EPMM0		=0x28,
	EPMM1		=0x29,
	EPMM2		=0x2A,
	EPMM3		=0x2B,
	EPMM4		=0x2C,
	EPMM5		=0x2D,
	EPMM6		=0x2E,
	EPMM7		=0x2F,
	EPMCSL		=0x30,
	EPMCSH		=0x31,
	EPMOL		=0x34,
	EPMOH		=0x35,
	ERXFCON		=0x38,
	EPKTCNT		=0x39,

	//Bank 2 - 010xxxxx
	MACON1		=0x40,
	MACON3		=0x42,
	MACON4		=0x43,
	MABBIPG		=0x44,
	MAIPGL		=0x46,
	MAIPGH		=0x47,
	MACLCON1	=0x48,
	MACLCON2	=0x49,
	MAMXFLL		=0x4A,
	MAMXFLH		=0x4B,
	MICMD		=0x52,
	MIREGADR	=0X54,
	MIWRL		=0x56,
	MIWRH		=0x57,
	MIRDL		=0X58,
	MIRDH		=0x59,

	//Bank 3 - 011xxxxx
	MAADR5		=0x60,
	MAADR6		=0x61,
	MAADR3		=0x62,
	MAADR4		=0x63,
	MAADR1		=0x64,
	MAADR2		=0x65,
	EBSTSD		=0x66,
	EBSTCON		=0x67,
	EBSTCSL		=0x68,
	EBSTCSH		=0x69,
	MISTAT		=0x6A,
	EREVID		=0x72,
	ECOCON		=0x75,
	EFLOCON		=0x77,
	EPAUSL		=0x78,
	EPAUSH		=0x79
}cont_reg;
/* Control Registers
	First 3 bits indicate bank:
		000	-	register in bank 0
		001 -	register in bank 1
		010 -	register in bank 2
		011 -	register in bank 3
		1xx	-	register can be accessed from any bank
	Correct bank must be selected in order access register

	Remaining 5 bits specify addresses of Control Register in bank

	See ENC28J60 page 12
*/

typedef enum{
	PHCON1 	= 	0x00,
	PHSTAT1	=	0x01,
	PHID1	=	0x02,
	PHID2	=	0x03,
	PHCON2	=	0x10,
	PHSTAT2	=	0x11,
	PHIE	=	0x12,
	PHIR	=	0x13,
	PHLCON	=	0x14
}phy_reg;

void SPI_init();
void byte_out(uint8_t*);
void byte_in(uint8_t*);

void RCR(uint8_t*, cont_reg);
/* Read Control Register
	args:
		uint8_t*	-	pointer to register where result will be stored
		cont reg	-	control register to read
*/

void WCR(uint8_t*, cont_reg);
/* Write Control Register
	args:
		uint8_t*	-	pointer to register containing value to be written
		cont_reg	- 	control register to write
*/


void BFS(uint8_t*, cont_reg);
/* Bit Field Set: sets bits of control registers only
	args:
		uint8_t*	-	pointer to bit pattern to set (AND)
		cont_reg	-	target register
*/

void BFC(uint8_t*, cont_reg);
/* Bit Field Clear: clears bits of control registers only
	args:
		uint8_t		-	pointer to bit pattern to clear (NOTAND)
		cont_reg	-	target register
*/

void WPR(uint8_t*, phy_reg);
void RPR(uint8_t*, phy_reg);

void RBM(uint8_t*);
/* Read Buffer Memory: reads from the location specified by ERDPT
If auto increment is enabled [ECON2:AUTOINC], ERDPT is incremented each time a byte is read
	args:
		uint8_t*	-	pointer to number of bytes to read, bytes will be written to the subsequent registers
			ie >|| 4  || byte 0 || byte 1 || byte 2 || byte 3
*/

void WBM(uint8_t*);
/* Write buffer memory: writes to the location specified by ERDPT
If auto increment is enabled [ECON2:AUTOINC], ERDPT is incremented each time a byte is written
	args:
		uint8_t*	-	pointer to number of bytes to write, bytes will be read from the subsequent registers
			ie >|| 4  || byte 0 || byte 1 || byte 2 || byte 3
*/

void ENC_init(uint8_t*);
/* Initialise ENC28J60 IC
	args:
		uint8_t* 	-	pointer to buffer to be used, first byte free, subsequent bytes specify local MAC address
*/

void send_packet(uint8_t*);	//MAX DATA SIZE 241 BYTES
/* Send ethernet packet
	args:
		uint8_t*	-	pointer to frame > || Size S (1) || Destination MAC (6) || Source MAC (6) || Type || Data (S) ||
		When function returns, pointer to 7 byte status vector
*/

#ifdef __cplusplus
}
#endif