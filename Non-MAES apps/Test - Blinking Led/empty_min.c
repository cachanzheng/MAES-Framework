#include <xdc/std.h>
#include <xdc/runtime/System.h>
/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>


/* Board Header file */
#include "Board.h"

#define TASKSTACKSIZE   512

char writing_stack[1024];
char reading_stack[1024];
char reading2_stack[1024];

Task_Handle w1,r1,r2;
Semaphore_Handle S1,S2;

/*
 *  ======== heartBeatFxn ========
 *  Toggle the Board_LED0. The Task_sleep is determined by arg0 which
 *  is configured for the heartBeat Task instance.
 */
void writing(UArg arg0, UArg arg1)
{
    while(1){
        System_printf("Writing\n");
        Semaphore_post(S1);
        Semaphore_post(S2);

        Task_sleep(1000);

    }

}
void reading(UArg arg0, UArg arg1)
{
    while(1) {
        Semaphore_pend(S1, BIOS_WAIT_FOREVER);
        System_printf("Agent1\n");
        System_flush();
        GPIO_toggle(Board_LED0);
    }
}

void reading2(UArg arg0, UArg arg1)
{
  while(1) {
        Semaphore_pend(S2, BIOS_WAIT_FOREVER);
        System_printf("Agent2\n");
        System_flush();
        GPIO_toggle(Board_LED1);

     }
}



/*
 *  ======== main ========
 */
int main(void)
{
    Task_Params taskParams;

    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();

    /* Turn on user LED  */
    GPIO_write(Board_LED0, Board_LED_ON);

    /* SysMin will only print to the console when you call flush or exit */
    System_printf("Blinking Led Example with agents \n");
    System_flush();

    /*Construct semaphores*/
    S1=Semaphore_create(0, NULL, NULL);
    S2=Semaphore_create(0, NULL, NULL);


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

    /*Construct reading2*/
    Task_Params_init(&taskParams);
    taskParams.stack = reading2_stack;
    taskParams.stackSize = 1024;
    taskParams.priority = 2;
    r2 = Task_create(reading2, &taskParams, NULL);

    /* Start BIOS */
    BIOS_start();

    return (0);
}

