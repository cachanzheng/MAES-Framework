/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>

/* Board Header file */
#include "Board.h"

/* Include Agent file*/
#include "maes.h"
#include <stdlib.h>
#include <stdio.h>
using namespace MAES;

/*Construction functions*/
void togglingbutton(UArg arg0, UArg arg1);



Agent_AID button1_AID, button2_AID;
Agent_Stack Agent1[1024];
Agent_Stack Agent2[1024];
Agent Button1("Agent 1", 2, Agent1,1024);
Agent Button2("Agent 2", 2, Agent2,1024);

class toggling: public CyclicBehaviour{
    void action(){
        msg.receive(BIOS_WAIT_FOREVER);
        GPIO_toggle(arg0);
        System_printf("Button %d pressed \n", msg.get_msg_int());
        System_flush();
    }
};

void togglingbutton(UArg arg0, UArg arg1){
    toggling b;
    b.arg0=arg0;
    b.execute();
};



/*Constructing platform and AID handle*/
Agent_Platform AP("Texas Instruments");



/*Callback functions for the button*/
void gpioButtonFxn0(unsigned int index)
{
    Agent_info agent;
    MsgObj msg;

    msg.content_int=0;
    agent=AP.get_Agent_description(button1_AID);
    Mailbox_post(agent.mailbox_handle, &msg, BIOS_NO_WAIT);
}

/*
 *  ======== gpioButtonFxn1 ========
 *  Callback function for the GPIO interrupt on Board_BUTTON1.
 *  This may not be used for all boards.
 */
void gpioButtonFxn1(unsigned int index)
{
    Agent_info agent;
    MsgObj msg;

    msg.content_int=1;

    agent=AP.get_Agent_description(button2_AID);
    Mailbox_post(agent.mailbox_handle, &msg, BIOS_NO_WAIT);
}

/*  ======== main ========
 */
int main()
{

    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();

    GPIO_write(Board_LED0, Board_LED_ON);

    /* SysMin will only print to the console when you call flush or exit */
    System_printf("Blinking Led Example with agents \n");
    System_flush();

    /* install Button callback */
    GPIO_setCallback(Board_BUTTON0, gpioButtonFxn0);

    /* Enable interrupts */
    GPIO_enableInt(Board_BUTTON0);

    /*
    *  If more than one input pin is available for your device, interrupts
    *  will be enabled on Board_BUTTON1.
    */
    if (Board_BUTTON0 != Board_BUTTON1) {
       /* install Button callback */
       GPIO_setCallback(Board_BUTTON1, gpioButtonFxn1);
       GPIO_enableInt(Board_BUTTON1);
    }

    /*Agents init*/
    AP.agent_init(Button1, togglingbutton, (UArg)Board_LED0,0, button1_AID);
    AP.agent_init(Button2, togglingbutton, (UArg)Board_LED1,0, button2_AID);
    AP.boot();

    BIOS_start();
    return (0);
}
