/* trproc.h - Transputer routines */

#if  !defined( __TRANSPUTER__ )
#define __TRANSPUTER__

#include <time.h>

#define	base			0xfffac0
#define inreg           base+1			/* C012 */
#define outreg          ((base)+3)
#define instat          ((base)+5)
#define outstat         ((base)+7)
#define reset           ((base)+17)		/* CPLD */
#define analyse         ((base)+19)
#define errflag         reset

#define inp(addr) (*(unsigned char*) (addr))              
#define outp(addr,val) (*(unsigned char*) (addr) = (val)) 


long trdummy;
volatile int t_found;
unsigned char *taddress;

jmp_buf env;

/*  this is the Transputer Identification code */
static char t_ident[] =
{23,                            /* length of bootstrap            * ls 2 bits
				 * 1, so its a data     * packet for 1/2
				 * speed links    */
 0xB1,                          /* AJW 1, allow for Iptr store    */
 0xD1,                          /* STL 1                          */
 0x24, 0xF2,                    /* MINT                           */
 0x21, 0xFC,                    /* STHF                           */
 0x24, 0xF2,                    /* MINT                           */
 0x21, 0xF8,                    /* STLF                           */
 0xF0,                          /* REV                            */
 0x60, 0x5C,                    /* LDNLP -4                       */
 0x2A, 0x2A, 0x2A, 0x4A,        /* LDC #AAAA                      */
 0xFF,                          /* OUTWORD                        */
 0x21, 0x2F, 0xFF,              /* START                          */
 0x02, 0x00};                   /* C004 read link                 */



/* ----------------------------------------------------------
   These functions handling the communication with the
   Transputer (i.e. read/write) by polling the input- and 
   output-status of the C011 Link-Adpater as well as
   changing the Endianess if needed.
   ----------------------------------------------------------
 */

/* send 'length' bytes of 'string' and change endianess to Litte Endian.
   !! NOT TO BE USED for sending executable binaries to the Transputer !!  */
void puttr(char *string,int length) {    
	
	int i;
	
	if (length>1)
	  string=string+(length-1); /* moving the pointer to the end */
	
	for (i=0; i<length; i++) {	
		while ((inp(outstat) & 1) != 1);
		outp(outreg,*string--);
	}
}

/* send 'length' bytes of 'string' BUT NO endian conversion.
   To used for uploading binaries to the Transputer
   byte by byte */
   
void puttrraw(char *string,int length) {           
	int i;
	for (i=0; i<length; i++) {
	  while ((inp(outstat) & 1)!=1);
	  outp(outreg,*string++); 
	} 
}

#define puttrchar(c); puttr(&c,1);
#define puttrshort(s); puttr((unsigned char *)(&s),2);
#define puttrint(i); trdummy=(unsigned long)i; puttr((unsigned char *)(&trdummy),4);
#define puttrlong(l); puttr((unsigned char *)(&l),4);
#define puttrfloat(f); puttr((unsigned char *)(&f),4);
#define puttrdouble(d); puttr((unsigned char *)(&d),8);


/* get 'length' bytes into 'string' with endian conversion.
   !! NOT TO BE USED for getting data from the Transputer
      other than 16, 32 or 64 bit vars !!
*/

void gettr(char *string,int length) {           /* read length Bytes  */
  int i;
  
  	if (length>1)
	  string=string+(length-1); /* moving the pointer to the end */
  
  for (i=0; i<length; i++) {
    while ((inp(instat) & 1)!=1);
      *string--=(char)inp(inreg); /* filling the buffer from end-to-start*/
	} 
}

/* get 'length' bytes into 'string' BUT NO endian conversion.
   To be used for getting data from the Transputer byte by byte 
*/
   
void gettrraw (char *string,int length) {           /* read length Bytes  */
  int i;
  for (i=0; i<length; i++) {
    while ((inp(instat) & 1)!=1);
    *string++=(char)inp(inreg); } 
}

#define gettrchar(c); gettr(&c,1);
#define gettrshort(s); gettr((unsigned char *)(&s),2);
#define gettrint(i); gettr((unsigned char *)(&trdummy),4); i=(int)trdummy;
#define gettrlong(l); gettr((unsigned char *)(&l),4);
#define gettrfloat(f); gettr((unsigned char *)(&f),4);
#define gettrdouble(d); gettr((unsigned char *)(&d),8);

void init_t(void) {

  int i; 
 
  outp(reset,1);
  outp(analyse,0);                              /* reset Transputer */
   for (i=0; i<500; i++);                        /* wait a millisecond or so */
  outp(reset,0);
  outp(instat, 0); 		/* disable interrupts */
  outp(outstat, 0);
}

/* ----------------------------------------------------------
   These 3 funtions are handling the Bus-Error 
   if a Transputer(-interface) wasn't found.
   
   To use, call 'checkTransputer'.
   If called with checkTransputer(0), it will just return
   1 for found or 0 for not found.
   
   When called with checkTransputer(1), it runs a small code
   on the Transputer and tries to identify its basic 
   type and returns
   
   0  - no Transputer found
   1  - That's a C004 Link matrix controller
   2  - T2xx 16bit Transputer found
   4  - T4xx/T8xx 32bit Transputer found
   -1 - Something unknown found :-/
   ---------------------------------------------------------- */

void exception_handler(short sr)
{
    t_found = 0;
    longjmp(env, 1);
}

long check_read_byte(void)
{
    void (*v_save)(void);

    t_found = 1;
    v_save = Setexc(2, exception_handler);
    if (!setjmp(env))
        * (volatile unsigned char *) taddress = 1;
    Setexc(2, v_save);
    return(0);
}

int checkTransputer(int checkType)
{
	int to_error = 0; /* time out error counter*/
    int bytes = 0;
	unsigned char buffer[10];

	
    taddress = (unsigned char *) base;
    Supexec(check_read_byte);
	if (t_found) {
	
		if (checkType) {
			
			init_t();
			puttrraw( t_ident, sizeof(t_ident));
			
			while (1) {
				clock_t timeout = clock() + 10; /* 200ns/TICK = 2ms*/
			
				while ((inp(instat) & 1)!=1) {

					if (clock() >= timeout) {
						to_error++;			
						if (to_error == 10)
							break;
					}
				}

				if ( (bytes >= 10) || (to_error == 10) ) {
					break;
				}
				
				buffer[bytes++]=(char)inp(inreg); /* read a byte */	
			}
			
			switch (bytes) {
			  case 1:
				return (1);
			  case 2:
				if ((buffer[0] == 0xAA) && (buffer[1] == 0xAA))
					return (2);
			  case 4:
				if ((buffer[0] == 0xAA) && (buffer[1] == 0xAA))
					return(4);
				
			  default:
					return(-1);
			}
		} else {
			return 1;
		}
	} else {	
		return 0; /* No T found*/
	}	
}

#endif