#include <stdio.h>
#include "mqtt.h"
#include "uart.h"
#include "csv.h"
#include "mailbox.h"
#include <errno.h>
#include <string.h>



int main(int argc, const char *argv[])
{        

    mbs = create_mail_box_system();

    register_to_mail_system(mbs,"download",download_th);
    register_to_mail_system(mbs,"sock",sock_th);
    register_to_mail_system(mbs,"data",data_collect_th);


    wait_all_end(mbs);
    destroy_mail_box_system(mbs);

    return 0;
}


