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
using namespace MAES;

/*Construction functions*/
void writing(UArg arg0, UArg arg1);
void writing2(UArg arg0, UArg arg1);
void reading(UArg arg0, UArg arg1);
void reading2(UArg arg0, UArg arg1);

/*Constructing Agents*/
Agent_Build Reading("Reading Agent",reading);
Agent_Build Reading2("Reading Agent 2",reading2);
Agent_Build Reading3("Reading Agent 3",reading2);
Agent_Build Writing("Writing Agent",writing);
Agent_Build Writing2("Writing Agent 2",writing2);

/*Creating_Platform*/
Agent_Management_Services AP("Texas Instruments");

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
//    AP.print();
//    System_printf("--- \n");
//        System_flush();

    AP.register_agent(Reading.create_agent());
    AP.register_agent(Writing.create_agent());
    AP.register_agent(Reading2.create_agent());
    AP.register_agent(Reading3.create_agent());

    BIOS_start();
    return (0);
}

void reading(UArg arg0, UArg arg1)
{

    Agent_Msg msg;
    while(1) {
            msg.receive(BIOS_WAIT_FOREVER);
            GPIO_toggle(Board_LED0);
            System_printf("Receiver: %s, Sender: %s, Read: %d \n", Task_Handle_name(Task_self()),msg.get_sender(), msg.get_msg_type());
            System_flush();
        }
}

void reading2(UArg arg0, UArg arg1)
{

    Agent_Msg msg;
    while(1) {
            //Mailbox_pend(Reading.get_mailbox_handle(), (xdc_Ptr) &msg, BIOS_WAIT_FOREVER);
            msg.receive(BIOS_WAIT_FOREVER);
            GPIO_toggle(Board_LED1);
            System_printf("Receiver: %s, Sender: %s, Read: %d \n", Task_Handle_name(Task_self()), msg.get_sender(), msg.get_msg_type());
            System_flush();


        }
}

void writing(UArg arg0, UArg arg1)
{
    Agent_Msg msg;

    int i=0;
    msg.set_msg_type(0);
    msg.set_msg_body(NULL);
    msg.add_receiver(Reading.get_AID());
    msg.add_receiver(Reading2.get_AID());
    msg.add_receiver(Reading3.get_AID());
    //AP.print();
      msg.print();



    System_printf("error: %s \n", Task_getEnv(Reading.get_AID()));
    System_flush();
    while(1) {

        if (msg.send()){
            msg.set_msg_type(i++);
        }
            Task_sleep(500);

    }

}

void writing2(UArg arg0, UArg arg1)
{
    Agent_Msg msg;

    int i=0;
    msg.set_msg_type(0);
    msg.set_msg_body(NULL);
 //   msg.add_receiver(Reading.get_mailbox());

    while(1) {

        if (msg.send()){
            msg.set_msg_type(i++);
        }
            Task_sleep(500);

    }

}

