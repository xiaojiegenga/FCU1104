#ifndef _CSV_H_
#define _CSV_H_

#include <stdio.h>
#include <stdlib.h>
#include "uart.h"

int get_rows(FILE *fp);
void write_csv(FILE *fp, struct datatype *data);
void check_lenth(FILE *fp);

#endif
