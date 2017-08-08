#include <xdc/std.h>
#include <xdc/runtime/System.h>
/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <xdc/runtime/Timestamp.h>
/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include "maes.h"

/* Board Header file */
#include "Board.h"
using namespace MAES;

#define TASKSTACKSIZE   512

char writing_stack[1024];
char reading_stack[1024];
char reading2_stack[1024];

Task_Handle w1,r1;
Semaphore_Handle S1;
Mailbox_Handle m1;
int init, stop;
/*
 *  ======== heartBeatFxn ========
 *  Toggle the Board_LED0. The Task_sleep is determined by arg0 which
 *  is configured for the heartBeat Task instance.
 */
void writing(UArg arg0, UArg arg1)
{
    while(1){
        Task_sleep(100);
     //   System_printf("Writing\n");
       // Log_write1(UIABenchmark_start, (xdc_IArg)"running");
        init=Timestamp_get32();
        //Mailbox_post(m1, 0, BIOS_WAIT_FOREVER);
        Semaphore_post(S1);


    }

}
void reading(UArg arg0, UArg arg1)
{
    while(1) {
        Semaphore_pend(S1, BIOS_WAIT_FOREVER);
       // Mailbox_pend(m1, 0, BIOS_WAIT_FOREVER);
        stop=Timestamp_get32();
        System_printf("Difference %d\n", stop-init);
        System_flush();
   //     Log_write1(UIABenchmark_stop, (xdc_IArg)"running");
//        System_printf("Agent1\n");
//        System_flush();
        GPIO_toggle(Board_LED0);

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

    Mailbox_Params_init(&mbxParams);
    m1= Mailbox_create(4,1,&mbxParams,NULL);
    S1=Semaphore_create(0, NULL, NULL);

    /*Construct writing*/
    Task_Params_init(&taskParams);
    taskParams.stack = writing_stack;
    taskParams.stackSize = 1024;
    taskParams.priority = 1;
    w1 = Task_create(writing, &taskParams, NULL);

    /*Construct reading*/
    Task_Params_init(&taskParams);
    taskParams.stack = reading_stack;
    taskParams.stackSize = 1024;
    taskParams.priority = 2;
    r1 = Task_create(reading, &taskParams, NULL);


    /* Start BIOS */
    BIOS_start();

    return (0);
}

