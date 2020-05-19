#include "utils.h"

void hexcpy(char * out, unsigned char * in, size_t insz)
{
  int i;
  char* buf = out;
  for (i = 0; i < insz; i++)
  {
      buf += sprintf(buf, "%02x", in[i]);
  }
  buf += sprintf(buf, "\0");
}
