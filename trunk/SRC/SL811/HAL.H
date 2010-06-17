#define SL811_ADDR_PORT		0x0ff00
#define SL811_DATA_PORT		0x0ff01

  u_int8 SL811Read(  u_int8 a);
void SL811Write(  u_int8 a,   u_int8 d);
void SL811BufRead(  u_int8 addr,   u_int8 *s,   u_int8 c);
void SL811BufWrite(  u_int8 addr,   u_int8 *s,   u_int8 c);




