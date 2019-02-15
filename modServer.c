#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define _POSIX_SOURCE 1 
#define FALSE 0
#define TRUE 1
#define POLY 0xA001
#define VER(x) #x

char serialtty[20];
volatile int STOP=FALSE; 
int fd,count=0;
void writeModbus(unsigned char *sendData,unsigned char nob);
void readModbus_M();
void readModbus_S();
extern unsigned short crc16( unsigned char *message, unsigned char nob );

void main(int argc,char *argv[])
 {
	   int countNull=0,i,time=1;
       unsigned char *inData=(char *) malloc(2);
	   char str[20],*x;
	   if(argc<5)
		{
			printf("Usage: ~/serialSend <SerialPort> <Protocol> <\"Payload\"> <Timeout in milliseconds> Version:%s\n",VER(v1.1));
			printf("Protocol-> M is For Modbus and S is for ASCII\n");
			exit(1);
		}
	   strcpy(serialtty,argv[1]);
	    if(!(strcmp(argv[2],"M")))
	  {
   		  strcpy(str,argv[3]);
    	   x=strtok(str," ");
		   while(x != NULL)
		   {
			  //  printf("x:%s length:%d\n",x,strlen(x));
		   	    inData[count]=strtol(x,NULL,16); 
		   		//printf(":%d:\n",strtol("1",NULL,16) );
		   	  //	printf(":%x\n:",inData[count]);
			    x=strtok(NULL," "); 
			    count++;
			}
	   		
	   			unsigned short crc_16=crc16(inData,count);
       			char crchi=(0x00FF&crc_16);
       			char crclo=(crc_16>>8);
       			inData[count++]=crchi;
      			inData[count++]=crclo;
   	  }
	   
	  if(!(strcmp(argv[2],"S")))
	  {		  
  			strcpy(inData,argv[3]);
			count=strlen(argv[3]);
	    	//	printf("Count:%d\n",count);
	  }
           
	  struct termios oldtio,newtio;
		time=atoi(argv[4]);
        fd = open(serialtty, O_RDWR | O_NOCTTY); 
       if (fd <0) {perror(serialtty); exit(-1); }
       tcgetattr(fd,&oldtio);         /*saving current serial(termios) settings)*/ 
       tcgetattr(fd,&newtio);
       newtio.c_cc[VMIN]=0;           /* Non blocking read with inter character timer */
       newtio.c_cc[VTIME]=time;          /* inter-character timer unused for modbus rtu > 3ms betwn two packets @9600 bps */
       newtio.c_cc[VINTR]=0;          /*interrupt characte:Ctrl-c muted */  
       //tcflush(fd, TCIFLUSH);         /*flush any data not readed by terminal*/ 
       tcsetattr(fd,TCSANOW,&newtio);
       writeModbus(inData,count);	   /*write to modbus*/
	    if(!(strcmp(argv[2],"M")))
      		 readModbus_M();                  /*Read response from modbus(hexa decimal frame)*/
        if(!(strcmp(argv[2],"S")))
		{ printf("Modbus: "); readModbus_S();}                  /*Read response from ioc plain ascii frame*/
		tcsetattr(fd,TCSANOW,&oldtio);
       close(fd);
	   free(inData);

 }

void writeModbus(unsigned char *sendData,unsigned char nob)
 {
       int writeStatus,i;
       writeStatus= write(fd, sendData,nob);
       //printf("send returnsts:%d\n", writeStatus);
 }

void readModbus_M()
 {
       int readStatus,countNull=0;    
       unsigned  char modbyte;
	   //printf("Modbus: ");
       while (STOP==FALSE) {                       /* loop for input */
          readStatus = read(fd,&modbyte,1);       /* returns after 5 chars have been input */
          if( readStatus != 0 )
        	  printf("%.2X ",modbyte);
          else countNull++;
          if (countNull >= 2) STOP=TRUE;
          //if (countNull >= ) STOP=TRUE;
        }
	   printf("\n");
       //printf("recv returnsts:%d\n", readStatus);
 }

void readModbus_S()
 {
       int readStatus,countNull=0;    
       unsigned  char modbyte;
         while (STOP==FALSE) {                       /* loop for input */
          readStatus = read(fd,&modbyte,1);       /* returns after 5 chars have been input */
          //printf("recv returnsts:%d\n", readStatus);
          //if( readStatus != 0 ) printf(" %.2x ",modbyte); else countNull++;
          if( readStatus != 0 ) printf("%c",modbyte); else countNull++;
          if (countNull >= 2) STOP=TRUE;
          //if (countNull >= ) STOP=TRUE;
        }
	   printf("\n");
       //printf("recv returnsts:%d\n", readStatus);
 } 

unsigned short crc16( unsigned char *message, unsigned char nob )
{
    unsigned char  i;
    unsigned short crc=0xFFFF;
    while (nob--)
    {
      crc ^= *message++;
      for (i = 8; i != 0; i--)
       {
            if (crc & 1)
                crc = (crc >> 1) ^ POLY;
            else
                crc >>= 1;
        }
    }
    //printf("%.4X\n",crc);
    return (crc);
}

