/*
 * Copyright (c) 2015, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== empty_min.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
// #include <ti/drivers/I2C.h>
// #include <ti/drivers/SDSPI.h>
// #include <ti/drivers/SPI.h>
// #include <ti/drivers/UART.h>
// #include <ti/drivers/Watchdog.h>
// #include <ti/drivers/WiFi.h>

/* Board Header file */
#include "Board.h"

/* Include Agent file*/

#include "maes.h"
#include <stdlib.h>
#include <stdio.h>
using namespace MAES;

/*Construction functions*/
void writing(UArg arg0, UArg arg1);
void reading(UArg arg0, UArg arg1);
void reading2(UArg arg0, UArg arg1);

/*Constructing Agents*/
Agent Reading("Reading Agent",reading);
Agent Reading2("Reading2 Agent",reading2);
Agent Writing("Writing Agent",writing);



class test: public USER_DEF_COND{
public:
    bool register_cond(){
       int a;
       a=rand()%100;
       System_printf("result: %i\n",a);
       System_flush();
       return a>20;
    }
};
test test;
/*Creating_Platform*/
Agent_Platform AP("Texas Instruments",&test);
/*
 *  ======== main ========
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

    Writing.init_agent();
    Reading2.init_agent();
    Reading.init_agent();

    AP.init();

    BIOS_start();
    return (0);

}

void reading(UArg arg0, UArg arg1)
{

    Agent_Msg msg;
    while(1) {
        msg.receive(BIOS_WAIT_FOREVER);
        System_printf("Receiver: %s Sender: %s Received: %x \n", Task_Handle_name(AP.get_running_agent_aid()),Task_Handle_name(msg.get_sender()),msg.get_msg_type());
        System_flush();
        GPIO_toggle(Board_LED0);
    }
}

void reading2(UArg arg0, UArg arg1)
{

    Agent_Msg msg;
    while(1) {
        msg.receive(BIOS_WAIT_FOREVER);
        System_printf("Receiver: %s Sender: %s Received: %x \n", Task_Handle_name(AP.get_running_agent_aid()),Task_Handle_name(msg.get_sender()),msg.get_msg_type());
        System_flush();
        GPIO_toggle(Board_LED1);
     }
}

void writing(UArg arg0, UArg arg1)
{

    Agent_Msg msg;
    int i=0;
    const Task_Handle *t;
    msg.request_AP(REGISTER, Reading2.get_AID(), BIOS_WAIT_FOREVER);
    msg.add_receiver(&Reading);
    msg.add_receiver(Reading2.get_AID());
    t=AP.get_list_subscriber();

    while (i<AP.get_number_subscribers()){
        System_printf("%x \n",t[i]);
        System_flush();
        i++;

    }
    i=0;
   while(1) {
      msg.broadcast(BIOS_WAIT_FOREVER);
      AP.agent_wait(500);
      i++;
    //  msg.set_msg_type(i);


    }

}

