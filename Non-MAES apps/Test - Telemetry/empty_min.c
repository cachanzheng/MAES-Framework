#include <xdc/std.h>
#include <xdc/runtime/System.h>
/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Clock.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Board Header file */
#include "Board.h"

#define TASKSTACKSIZE   1024

char logger_500ms[TASKSTACKSIZE];
char logger_5s[TASKSTACKSIZE];
char logger_60s[TASKSTACKSIZE];
char measurement[TASKSTACKSIZE];

Task_Handle log500ms_h, log5s_h, log60s_h, meas_h;
Mailbox_Handle log500ms_m, log5s_m, log60s_m, meas_m;
UART_Handle uart;

typedef enum meas_type{
    CURRENT,
    VOLTAGE,
    TEMPERATURE
}meas_type;

typedef struct logger_info{
    Mailbox_Handle logger_id;
    meas_type type;
    int rate; //in ms.
}logger_info;

typedef struct msg{
    Mailbox_Handle sender;
    meas_type type;
    String payload;
}msg;

logger_info log_500ms, log_5s,log_60s;

void logger(UArg arg0, UArg arg1){
    msg obj;
    logger_info *info;
    info=(logger_info *)arg0;

    obj.type=info->type;
    obj.sender=info->logger_id;

    while(1){
        Mailbox_post(meas_m, (xdc_Ptr)&obj, BIOS_WAIT_FOREVER);
        Mailbox_pend(info->logger_id, (xdc_Ptr) &obj, BIOS_WAIT_FOREVER);

        /**Logging*/
        System_printf("%s\n",obj.payload);
        System_flush();

        Task_sleep(info->rate); // Sleep the rest until next period.
    }
}

void gen_meas(UArg arg0, UArg arg1){

    msg obj;
    float min,max,value;
    char msg[50];

    UART_Params uartParams;

    /* Initialize UART */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.baudRate = 115200;
    uart = UART_open(Board_UART0, &uartParams);
    if(uart == NULL) {
        System_abort("UART open failed...\n");
    }
    else {
        System_printf("UART Initialized!\n");
    }
    System_flush();

    while(1){
        Mailbox_pend(meas_m,(xdc_Ptr)&obj, BIOS_WAIT_FOREVER);
        switch(obj.type){
            case CURRENT:
                min=0.1; //0mA
                max=1000; //1000mA
                value=min + rand() / (RAND_MAX / (max - min + 1) + 1);
                snprintf(msg,100,"Current measurement: %f",value);

                break;
            case VOLTAGE:
                min=0.5; //0V
                max=3.3;//3.3V
                value=min + rand() / (RAND_MAX / (max - min + 1) + 1);
                snprintf(msg,100,"Voltage measurement: %f",value);

                break;
            case TEMPERATURE:
                min=30; //0 C;
                max=100;//100C;
                value=min + rand() / (RAND_MAX / (max - min + 1) + 1);
                snprintf(msg,100,"Temperature measurement: %f",value);
          break;
            default:
                snprintf(msg,100,"Not understood");
                break;
        }

        obj.payload=msg;
        Mailbox_post(obj.sender, (xdc_Ptr)&obj, BIOS_WAIT_FOREVER);
    }

}
/*
 *  ======== main ========
 */
int main(void)
{
    Task_Params taskParams;
    Mailbox_Params mbxParams;
    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    /* Turn on user LED  */
    GPIO_write(Board_LED0, Board_LED_ON);

    /* SysMin will only print to the console when you call flush or exit */
    System_printf("Blinking Led Example with agents \n");
    System_flush();

    /*Construct mailbox*/
    Mailbox_Params_init(&mbxParams);
    log500ms_m= Mailbox_create(sizeof(msg),4,&mbxParams,NULL);
    log5s_m=Mailbox_create(sizeof(msg),4,&mbxParams,NULL);
    log60s_m=Mailbox_create(sizeof(msg),4,&mbxParams,NULL);
    meas_m=Mailbox_create(sizeof(msg),4,&mbxParams,NULL);

    /*Construct logger_500ms*/
    log_500ms.logger_id=log500ms_m;
    log_500ms.rate=500;
    log_500ms.type=CURRENT;

    Task_Params_init(&taskParams);
    taskParams.stack = logger_500ms;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.priority = 1;
    taskParams.arg0=(UArg)&log_500ms;
    log500ms_h = Task_create(logger, &taskParams, NULL);

    /*Construct logger_5s*/
    log_5s.logger_id=log5s_m;
    log_5s.rate=5000;
    log_5s.type=VOLTAGE;

    Task_Params_init(&taskParams);
    taskParams.stack = logger_5s;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.priority = 1;
    taskParams.arg0=(UArg)&log_5s;
    log5s_h = Task_create(logger, &taskParams, NULL);

    /*Construct logger_60s*/
    log_60s.logger_id=log60s_m;
    log_60s.rate=10000;
    log_60s.type=TEMPERATURE;

    Task_Params_init(&taskParams);
    taskParams.stack = logger_60s;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.priority = 1;
    taskParams.arg0=(UArg)&log_60s;
    log60s_h = Task_create(logger, &taskParams, NULL);

    /*Construct gen_meas*/
    Task_Params_init(&taskParams);
    taskParams.stack = measurement;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.priority = 2;
    meas_h = Task_create(gen_meas, &taskParams, NULL);

    /* Start BIOS */
    BIOS_start();

    return (0);
}

