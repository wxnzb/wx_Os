#include "print.h"
void main(void)
{
//------------------------------------
  put_str("Welcome,\nI am kernel!");
//------------------------------------
  put_char('T');
  put_char('h');
  put_char('i');
  put_char('s');
  put_char(' ');
  put_char('.');
  put_char('\b');
  put_char('i');
  put_char('s');
  put_char('\n');
  put_char('k');
  put_char('e');
  put_char('r');
  put_char('n');
  put_char('e');
  put_char('l');
  put_char('!');
//-----------------------------
   put_char('\n');
  put_int(0x00000000);
  put_char('\n');
  put_int(0x12345678);
  put_char('\n');
  put_int(0xa1a20099);
  put_char('\n');
  while(1);
}