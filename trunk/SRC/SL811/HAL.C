#include "common.h"
#include "HAL.H"

//  u_int8 idata ttt[20];
extern   u_int8   DBUF[BUFFER_LENGTH];

  u_int8 SL811Read(  u_int8 a)
{  
	  u_int8 nVal;
	  //u_int32   *exAddress;
		//exAddress = SL811_ADDR_PORT;

	*((volatile u_int32 *)(SL811_ADDR_PORT))=a;
	//exAddress=SL811_DATA_PORT;
	nVal = *((volatile u_int32 *)(SL811_DATA_PORT));

	return nVal;
}

void SL811Write(  u_int8 a,   u_int8 d)
{  
	//u_int32   *exAddress;
	//exAddress = SL811_ADDR_PORT;

	*((volatile u_int32 *)(SL811_ADDR_PORT))=a;
	//exAddress=SL811_DATA_PORT;
	*((volatile u_int32 *)(SL811_DATA_PORT)) = d;

}

void SL811BufRead(  u_int8 addr,   u_int8 *s,   u_int8 c)
{	
	  u_int8 i;
	  //u_int8   *exAddress;
	//exAddress=SL811_ADDR_PORT;
	*((volatile u_int32 *)(SL811_ADDR_PORT)) = addr;
	//exAddress=SL811_DATA_PORT;
	for(i=0;i<c;i++)
		{
		*s++ = *((volatile u_int32 *)(SL811_DATA_PORT));
		}
}

void SL811BufWrite(  u_int8 addr,   u_int8 *s,   u_int8 c)
{	
	//  u_int8 temp;
	  //u_int8   *exAddress;
	//exAddress = SL811_ADDR_PORT;
	
	*((volatile u_int32 *)(SL811_ADDR_PORT)) = addr;
	//exAddress=SL811_DATA_PORT;
	while (c--) 
		{
		*((volatile u_int32 *)(SL811_DATA_PORT)) = *s++;
		}
	
}

