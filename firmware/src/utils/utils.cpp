#include "utils.h"

void hexToChar(char * out, unsigned char * in, size_t insz)
{
  int i;
  char* buf = out;
  for (i = 0; i < insz; i++)
  {
      buf += sprintf(buf, "%02x", in[i]);
  }
  buf[insz*2+1]='\0';
}

unsigned char hexDigit(char ch){
  if(( '0' <= ch ) && ( ch <= '9' )){
    ch -= '0';
  }else{
    if(( 'a' <= ch ) && ( ch <= 'f' )){
      ch += 10 - 'a';
    }else{
      if(( 'A' <= ch ) && ( ch <= 'F' ) ){
        ch += 10 - 'A';
      }else{
        ch = 16;
      }
    }
  }
  return ch;
}

void charToHex(unsigned char * out, char * in, size_t outsz)
{
  for( int i = 0; i < outsz; ++i ){
    out[i]  = hexDigit( in[2*i] ) << 4;
    out[i] |= hexDigit( in[2*i+1] );
  }
}
