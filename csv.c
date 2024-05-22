#include <stdio.h>
#include <stdlib.h>
#include "uart.h"

/* extern struct datatype; */
/* extern struct solid; */
static int rows; 
int get_rows(FILE *fp)
{
    fseek(fp,0,SEEK_SET);
    char buffer[1000];
    while(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
            rows++;
    }
    printf("rows = %d\n",rows);
    return rows;
}

void check_lenth(FILE *fp)
{
    if(rows >= 1440)
    {
        printf("time more than one month, clear file\n");
        if(ftruncate(fileno(fp),0) == -1)
        {
            perror("perror clear file\n");
            return ;
        }
    }
}

void write_csv(FILE *fp, struct datatype *data)
{
    check_lenth(fp);
    /* int rows = get_rows(fp); */
    rows++;
    fprintf(fp,"%d,%d,%d,%.1f,%.1f,%.1f,%.1f,%.1f\n",rows, data->sun, (int)data->ph, data->temp - 3, data->humi + 20, data->humi, data->temp, data->elec);
    /* rows = get_rows(fp); */
    /* printf("rows = %d\n",rows); */
    fflush(fp);
    return ;
}

/* int main() */
/* { */
/*     FILE *fp = fopen("data.csv", "w+"); */
/*     if(fp == NULL) */
/*     { */
/*         fprintf(stderr,"fopen failed \n"); */
/*         exit(EXIT_FAILURE); */
/*     } */

/*     fprintf(fp,"id,sun,ph,air_temp,air_humi,sol_humi,sol_temp,sol_elec\n"); */
/*     struct datatype *data = (struct datatype *)malloc(sizeof(struct datatype)); */
/*     data->sd = (struct solid *)malloc(sizeof(struct solid)); */
/*     data->rain = 2; */
/*     data->sun = 1; */
  
/*     write_csv(fp,data); */

/*     fclose(fp); */

/*     printf("Hello world\n"); */
/*     return 0; */
/* } */

