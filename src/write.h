#ifndef CS430_PNM_WRITE_H
#define CS430_PNM_WRITE_H

#include <stdio.h>

#include "pnm.h"

int writeHeader(pnmHeader header, FILE* outputFd);
int writeBody(pnmHeader header, pixel* pixels, FILE* outputFd);

#endif // CS430_PNM_WRITE_H
