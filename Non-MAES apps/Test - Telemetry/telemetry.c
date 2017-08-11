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

#define TASKSTACKSIZE   2048

char currentstack[TASKSTACKSIZE];
char voltagestack[TASKSTACKSIZE];
char temperaturestack[TASKSTACKSIZE];
char measurement[TASKSTACKSIZE];

Task_Handle logcurrent_h, logvoltage_h, logtemperature_h, meas_h;
Mailbox_Handle logcurrent_m, logvoltage_m, logtemperature_m, meas_m;


UART_Handle uart;
UART_Params uartParams;


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

logger_info log_current, log_voltage,log_temperature;

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
        UART_write(uart, obj.payload, strlen(obj.payload));
        System_printf("%s",obj.payload);
        System_flush();
        Task_sleep(info->rate); // Sleep the rest until next period.
    }
}

void gen_meas(UArg arg0, UArg arg1){

    msg obj;
    float min,max,value;
    char msg[50];


    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.baudRate = 115200;
    uart = UART_open(Board_UART0, &uartParams);
    if (uart == NULL) System_abort("Error opening the UART");

    while(1){
        Mailbox_pend(meas_m,(xdc_Ptr)&obj, BIOS_WAIT_FOREVER);
        switch(obj.type){
            case CURRENT:
                min=0.1; //0mA
                max=1000; //1000mA
                value=min + rand() / (RAND_MAX / (max - min + 1) + 1);
                snprintf(msg,100,"\r\nCurrent measurement: %f\r\n",value);

                break;
            case VOLTAGE:
                min=0.5; //0V
                max=3.3;//3.3V
                value=min + rand() / (RAND_MAX / (max - min + 1) + 1);
                snprintf(msg,100,"\r\nVoltage measurement: %f\r\n",value);

                break;
            case TEMPERATURE:
                min=30; //0 C;
                max=100;//100C;
                value=min + rand() / (RAND_MAX / (max - min + 1) + 1);
                snprintf(msg,100,"\r\nTemperature measurement: %f\r\n",value);
          break;
            default:
                snprintf(msg,100,"\r\nNot understood\r\n");
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
    Board_initUART();
    /* Turn on user LED  */
    GPIO_write(Board_LED0, Board_LED_ON);

    /* SysMin will only print to the console when you call flush or exit */
    System_printf("Telemetry example\n");
    System_flush();

    /*Construct mailbox*/
    Mailbox_Params_init(&mbxParams);
    logcurrent_m= Mailbox_create(sizeof(msg),4,&mbxParams,NULL);
    logvoltage_m=Mailbox_create(sizeof(msg),4,&mbxParams,NULL);
    logtemperature_m=Mailbox_create(sizeof(msg),4,&mbxParams,NULL);
    meas_m=Mailbox_create(sizeof(msg),4,&mbxParams,NULL);

    /*Construct currentstack*/
    log_current.logger_id=logcurrent_m;
    log_current.rate=500;
    log_current.type=CURRENT;

    Task_Params_init(&taskParams);
    taskParams.stack = currentstack;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.priority = 4;
    taskParams.arg0=(UArg)&log_current;
    logcurrent_h = Task_create(logger, &taskParams, NULL);

    /*Construct voltagestack*/
    log_voltage.logger_id=logvoltage_m;
    log_voltage.rate=1000;
    log_voltage.type=VOLTAGE;

    Task_Params_init(&taskParams);
    taskParams.stack = voltagestack;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.priority = 3;
    taskParams.arg0=(UArg)&log_voltage;
    logvoltage_h = Task_create(logger, &taskParams, NULL);

    /*Construct temperaturestack*/
    log_temperature.logger_id=logtemperature_m;
    log_temperature.rate=2000;
    log_temperature.type=TEMPERATURE;

    Task_Params_init(&taskParams);
    taskParams.stack = temperaturestack;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.priority = 2;
    taskParams.arg0=(UArg)&log_temperature;
    logtemperature_h = Task_create(logger, &taskParams, NULL);

    /*Construct gen_meas*/
    Task_Params_init(&taskParams);
    taskParams.stack = measurement;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.priority = 5;
    meas_h = Task_create(gen_meas, &taskParams, NULL);

    /* Start BIOS */
    BIOS_start();

    return (0);
}

