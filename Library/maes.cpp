/*
 * maes.cpp
 *
 *  Created on: 14 May 2017
 *      Author: Carmen Chan-Zheng
 */

#include "maes.h"

namespace MAES{

void behaviour(UArg arg0, UArg arg1){
    Agent_Msg msg;
      while(1) {
              msg.receive(BIOS_WAIT_FOREVER);

          }
}
/*********************************************************************************************
*
*                         Class: Agent_AMS: Agent Management Services
*
********************************************************************************************
*********************************************************************************************
* Class: Agent_Management_Services
* Function: Agent_Management_Services Constructor
* Comment: Initialize list of agents in null
*          Initialize AMS task to size set by user
*          Be aware that allocatting dynamically the heap size should be larger
**********************************************************************************************/
    Agent_Management_Services::Agent_Management_Services(String name, int taskstackSize){
        int i=0;
        next_available=0;

        while (i<AGENT_LIST_SIZE){
            Agent_Handle[i]=(Task_Handle) NULL;
            Task_setEnv(Agent_Handle[i],(xdc_Ptr)0);
            i++;
        }
        AP.ptrAgent_Handle=Agent_Handle;
        AP.name=name;
    }
/*********************************************************************************************
* Class: Agent_Management_Services
* Function: Agent_Management_Services Constructor
* Comment: Initialize list of agents in null
*          Initialize AMS task to default 1024
**********************************************************************************************/
    Agent_Management_Services::Agent_Management_Services(String name){
        int i=0;
        next_available=0;

        while (i<AGENT_LIST_SIZE){
            Agent_Handle[i]=(Task_Handle) NULL;
            Task_setEnv(Agent_Handle[i],(xdc_Ptr)0);
            i++;
        }
        AP.ptrAgent_Handle=Agent_Handle;
        AP.name=name;
    }
 /*********************************************************************************************
* Class: Agent_Management_Services
* Function: init();
* Comment: Create AMS task with default stack of 512
**********************************************************************************************/
    bool Agent_Management_Services::init(){

            Task_Handle temp;
            char task_stack[512];
            Mailbox_Handle mailbox_handle;
            Mailbox_Params mbxParams;
            Task_Params taskParams;

            /*Creating mailbox*/
            Mailbox_Params_init(&mbxParams);
            mailbox_handle= Mailbox_create(12,5,&mbxParams,NULL);

            /*Creating task*/
            Task_Params_init(&taskParams);
            taskParams.stack=task_stack;
            taskParams.stackSize = 512;
            taskParams.priority = -1;
            taskParams.instance->name=AP.name;
            taskParams.arg0=(UArg)mailbox_handle;
            AP.AMS_aid = Task_create(behaviour, &taskParams, NULL);

            if (AP.AMS_aid!=NULL){
            /*Initializing all the previously created task*/
                temp=Task_Object_first();
                while (temp!=NULL){
                    if(temp==AP.AMS_aid) break; //Don't register AMS aid
                    register_agent(temp);
                    temp=Task_Object_next(temp);
                }

                return true;
            }
            else return false;
   }

/*********************************************************************************************
* Class: Agent_Management_Services
* Function: init();
* Comment: Create AMS task with user custom stack size
*          Be aware of heap size
**********************************************************************************************/
   bool Agent_Management_Services::init(int stackSize){

           Task_Handle temp;
           char* task_stack=new char[stackSize];
           Mailbox_Handle mailbox_handle;
           Mailbox_Params mbxParams;
           Task_Params taskParams;

           /*Creating mailbox*/
           Mailbox_Params_init(&mbxParams);
           mailbox_handle= Mailbox_create(12,5,&mbxParams,NULL);

           /*Creating task*/
           Task_Params_init(&taskParams);
           taskParams.stack=task_stack;
           taskParams.stackSize = stackSize;
           taskParams.priority = -1;
           taskParams.instance->name=AP.name;
           taskParams.arg0=(UArg)mailbox_handle;
           AP.AMS_aid = Task_create(behaviour, &taskParams, NULL);

           if (AP.AMS_aid!=NULL){
           /*Initializing all the previously created task*/
               temp=Task_Object_first();
               while (temp!=NULL){
                   if(temp==AP.AMS_aid) break; //Don't register AMS aid
                   register_agent(temp);
                   temp=Task_Object_next(temp);
               }

               return true;
           }
           else return false;
  }
/*********************************************************************************************
* Class: Agent_Management_Services
* Function: int register_agent(Task_Handle aid)
* Comment: register agent to the platform. If list is full, delete agent and mailbox
**********************************************************************************************/
    int Agent_Management_Services::register_agent(Task_Handle aid){
        if (aid==NULL) return HANDLE_NULL;
        if(next_available<AGENT_LIST_SIZE){
            Task_setEnv(aid,(xdc_Ptr)AP.AMS_aid);
            Agent_Handle[next_available]=aid;
            next_available++;
            return NO_ERROR;
        }

        else{
            UArg arg0,arg1;
            Task_getFunc(aid,&arg0,&arg1);
            Mailbox_Handle m=(Mailbox_Handle) arg0;
            Mailbox_delete(&m);
            Task_delete(&aid);
            return LIST_FULL;
        }

    }
/*********************************************************************************************
* Class: Agent_Management_Services
* Function:  int deregister_agent(Task_Handle aid);
* Comment: deregister agent o the platform. It searches inside of the list, when found,
* the rest of the list is shifted to the right and the agent is removed.
**********************************************************************************************/
    int Agent_Management_Services::deregister_agent(Task_Handle aid){
        int i=0;
        UArg arg0,arg1;

          while(i<AGENT_LIST_SIZE){
              if(Agent_Handle[i]==aid){
                  Task_getFunc(aid,&arg0,&arg1);
                  Mailbox_Handle m=(Mailbox_Handle) arg0;
                  Mailbox_delete(&m);
                  Task_delete(&aid);
                  while(i<AGENT_LIST_SIZE-1){
                      Agent_Handle[i]=Agent_Handle[i+1];
                      i++;
                  }
                  Agent_Handle[AGENT_LIST_SIZE-1]=NULL;
                  next_available--;
                  break;
              }
              i++;
          }

          if (i==AGENT_LIST_SIZE) return NOT_FOUND;
          else return NO_ERROR;
   }
/*********************************************************************************************
* Class: Agent_Management_Services
* Function:  bool search();
* Return: Bool
* Comment: Search AID within list and return true if found.
*          next_available is used instead of AGENT_LIST_SIZE since it will optimize
*          search
**********************************************************************************************/
    bool Agent_Management_Services::search(Task_Handle aid){
        int i=0;

          while(i<next_available){
              if (Agent_Handle[i]==aid) break;
              i++;
          }

          if (i==next_available) return false;
          else return true;
   }

/*********************************************************************************************
* Class: Agent_Management_Services
* Function:  bool search();
* Return: Bool
* Comment: Search AID within list and return true if found.
*          next_available is used instead of AGENT_LIST_SIZE since it will optimize
*          search
**********************************************************************************************/
    bool Agent_Management_Services::search(String name){
        int i=0;

          while(i<next_available){
              if (!strcmp(Task_Handle_name(Agent_Handle[i]),name)) break;
              i++;
          }

          if (i==next_available) return false;
          else return true;
   }
/*********************************************************************************************
* Class: Agent_Management_Services
* Function:  Task_Handle* list()
* Return: Pointer to the of the agents in the platform.
* Comment:
**********************************************************************************************/
    Task_Handle* Agent_Management_Services::return_list(){
        return AP.ptrAgent_Handle;
    }

/*********************************************************************************************
* Class: Agent_Management_Services
* Function:  int return_list_size();
* Return: Int
* Comment: Returns size of the list of agents registered in the AP
**********************************************************************************************/
    int Agent_Management_Services::return_list_size(){
        return next_available;
    }


/*********************************************************************************************
* Class: Agent_Management_Services
* Function: void modify(Task_Handle aid,Mailbox_Handle new_AP);
* Return: void
* Comment: modify AP mailbox_handle (in case to migrate another AP)
**********************************************************************************************/
    void Agent_Management_Services::modify(Task_Handle aid, Mailbox_Handle new_AP){
        Task_setEnv(aid,(xdc_Ptr)new_AP);
    }

/*********************************************************************************************
* Class: Agent_Management_Services
* Function: AP_Description* get_description();
* Return: AP_Description
* Comment: Returns description of the platform
**********************************************************************************************/
    AP_Description* Agent_Management_Services::get_description(){
        return &AP;
    }

/*********************************************************************************************
* Class: Agent_Management_Services
* Function: AP_Description* get_description();
* Return: Task Handle of the AMS
* Comment:
**********************************************************************************************/
    Task_Handle Agent_Management_Services::get_AMS_AID(){
        return AP.AMS_aid;
    }

/*********************************************************************************************
*
*                                  Class: Agent_Build
*
**********************************************************************************************

*********************************************************************************************
* Class: Agent_Build
* Function: Agent_Build constructor
* Comments: task_stack_size: set to default value 512
*           msg_queue_size: set to default value 5
*           msg_size: set to predefined MsgObj size
**********************************************************************************************/
    Agent_Build::Agent_Build(String name,
                             int pri,
                             Task_FuncPtr b){

        /*Mailbox init*/
        msg_size=12;  //String: 4, Int: 4, String: 4
        msg_queue_size=5;

        /*Task init*/
        agent_name=name;
        priority=pri;
        behaviour=b;
        task_stack_size=512;
        //task_stack=new char[task_stack_size];
        task_handle=NULL;
        Task_setEnv(task_handle,NULL);
        mailbox_handle=NULL;
    }

/*********************************************************************************************
* Class: Agent_Build
* Function: Agent_Build constructor
* Comments: task_stack_size: set to default value 512
*           msg_queue_size: set to default value 5
*           msg_size: set to predefined MsgObj size
**********************************************************************************************/
    Agent_Build::Agent_Build(String name,
                             Task_FuncPtr b){

        /*Mailbox init*/
        msg_size=12;  //String: 4, Int: 4, String: 4
        msg_queue_size=5;

        /*Task init*/
        agent_name=name;
        priority=1;
        behaviour=b;
        task_stack_size=512;
        task_handle=NULL;
        Task_setEnv(task_handle,NULL);
        mailbox_handle=NULL;
    }
/*********************************************************************************************
 * Class: Agent_Build
 * Function: void create_agent()
 * Return type: NULL
 * Comments: Create the task and mailbox associated with the agent. The mailbox handle is
 *           embedded in task's handle env variable.
*********************************************************************************************/
    Task_Handle Agent_Build::create_agent(){
            /*Creating mailbox*/
            Mailbox_Params_init(&mbxParams);
            mailbox_handle= Mailbox_create(msg_size,msg_queue_size,&mbxParams,NULL);

            /*Creating task*/
            Task_Params_init(&taskParams);
            taskParams.stack=task_stack;
            taskParams.stackSize =task_stack_size;
            taskParams.priority = priority;
            taskParams.instance->name=agent_name;
            taskParams.arg0=(UArg)mailbox_handle;
            task_handle = Task_create(behaviour, &taskParams, NULL);

            return task_handle;

    }

/*********************************************************************************************
 * Class: Agent_Build
 * Function: void create_agent(int taskstackSize)
 * Return type: NULL
 * Comments: Create the task and mailbox associated with the agent. The mailbox handle is
 *           embedded in task's handle env variable.
 *           With user custom taskstackSize. Be aware of heap size
*********************************************************************************************/
    Task_Handle Agent_Build::create_agent(int taskstackSize){

            task_stack_size= taskstackSize;
            task_stack_dyn=new char[task_stack_size];
            /*Creating mailbox*/
            Mailbox_Params_init(&mbxParams);
            mailbox_handle= Mailbox_create(msg_size,msg_queue_size,&mbxParams,NULL);

            /*Creating task*/
            Task_Params_init(&taskParams);
            taskParams.stack=task_stack_dyn;
            taskParams.stackSize =task_stack_size;
            taskParams.priority = priority;
            taskParams.instance->name=agent_name;
            taskParams.arg0=(UArg)mailbox_handle;
            task_handle = Task_create(behaviour, &taskParams, NULL);
            //AP.register_agent(task_handle);
            return task_handle;

    }
/*********************************************************************************************
 * Class: Agent_Build
 * Function: create_agent()
 * Return type: String
 * Comments: Returns agent's name
*********************************************************************************************/
    String Agent_Build::get_name(){
        return agent_name;
    }

/*********************************************************************************************
* Class: Agent_Build
* Function:  get_task_handle(){
* Return type: Task Handle
* Comments: Returns agent's task handle
*********************************************************************************************/
    Task_Handle Agent_Build::get_AID(){
        return task_handle;
    }

/*********************************************************************************************
* Class: Agent_Build
* Function: get_mailbox_handle()
* Return type: Mailbox Handle
* Comments: Returns agent's mailbox handle
*********************************************************************************************/
    Mailbox_Handle Agent_Build::get_mailbox(){
        return mailbox_handle;
    }

/*********************************************************************************************
* Class: Agent_Build
* Function: get_prio()
* Return type: int
* Comments: Returns agent's priority
*********************************************************************************************/
    int Agent_Build::get_prio(){
        return priority;
    }

/*********************************************************************************************
* Class: Agent_Build
* Function: agent_sleep(uint32 ticks)
* Return type: NULL
* Comments: Agent sleep in ticks
*********************************************************************************************/
   void Agent_Build::agent_sleep(Uint32 ticks){
        Task_sleep(ticks);
    }
/*********************************************************************************************
*
*                                  Class: Agent_Msg
*
*********************************************************************************************

*********************************************************************************************
* Class: Agent_Msg
* Function: Agent_Msg Constructor
* Comment: Construct Agent_Msg Object.
*          Msg object shall be created in the task function, therefore,
*          the Agent_msg object is assigned to the handle of the calling task.
*          The object contains information of the task handle, mailbox and the name
**********************************************************************************************/
   Agent_Msg::Agent_Msg(){
      self_handle=Task_self();
      clear_all_receiver();
      next_available=0;
   }

/*********************************************************************************************
* Class: Agent_Msg
* Function: add_receiver(Agent_Build agent)
* Return type: Boolean. True if receiver is added successfully.
* Comment: Add receiver to list of receivers by using the agent build
**********************************************************************************************/
  int Agent_Msg::add_receiver(Agent_Build agent){

      Mailbox_Handle m=agent.get_mailbox();
      if (m==NULL) return HANDLE_NULL;
      if(next_available<MAX_RECEIVERS){
          receivers[next_available]=m;
          next_available++;
          return NO_ERROR;
      }

       else return LIST_FULL;
   }
/*********************************************************************************************
* Class: Agent_Msg
* Function: add_receiver(Task_Handle aid)
* Return type: Boolean. True if receiver is added successfully.
* Comment: Add receiver to list of receivers by using the agent's aid
**********************************************************************************************/
   int Agent_Msg::add_receiver(Task_Handle aid){
       UArg arg0,arg1;
       Task_getFunc(aid,&arg0, &arg1);
       Mailbox_Handle m=(Mailbox_Handle) arg0;
       if (m==NULL) return HANDLE_NULL;
       if(next_available<MAX_RECEIVERS){
           receivers[next_available]=m;
           next_available++;
           return NO_ERROR;
       }

        else return LIST_FULL;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: add_receiver(Agent_Build agent)
* Return type: Boolean. True if receiver is added successfully.
* Comment: Add receiver to list of receivers by using the agent's mailbox
**********************************************************************************************/
    int Agent_Msg::add_receiver(Mailbox_Handle m){

     if (m==NULL) return HANDLE_NULL;
     if(next_available<MAX_RECEIVERS){
         receivers[next_available]=m;
         next_available++;
         return NO_ERROR;
     }

      else return LIST_FULL;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: remove_receiver(Agent_Build agent)
 Return type: Boolean. True if receiver is removed successfully. False if it is not encountered
* Comment: Remove receiver in list of receivers. It searches inside of the list, when found,
* the rest of the list is shifted to the right and the receiver is removed.
 **********************************************************************************************/
    int Agent_Msg::remove_receiver(Agent_Build agent){

        Mailbox_Handle m=agent.get_mailbox();
        int i=0;
        while(i<MAX_RECEIVERS){
            if(receivers[i]==m){
                while(i<MAX_RECEIVERS-1){
                    receivers[i]=receivers[i+1];
                    i++;
                }
                receivers[MAX_RECEIVERS-1]=NULL;
                next_available--;
                break;
            }
            i++;
        }

        if (i==MAX_RECEIVERS) return NOT_FOUND;
        else return NO_ERROR;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: remove_receiver(Task_Handle aid)
* Return type: Boolean. True if receiver is removed successfully. False if it is not encountered
* Comment: Remove receiver in list of receivers. It searches inside of the list, when found,
* the rest of the list is shifted to the right and the receiver is removed.
**********************************************************************************************/
   int Agent_Msg::remove_receiver(Task_Handle aid){
       UArg arg0,arg1;
       Task_getFunc(aid,&arg0, &arg1);
       Mailbox_Handle m=(Mailbox_Handle) arg0;
        int i=0;

        while(i<MAX_RECEIVERS){
            if(receivers[i]==m){
                while(i<MAX_RECEIVERS-1){
                    receivers[i]=receivers[i+1];
                    i++;
                }
                receivers[MAX_RECEIVERS-1]=NULL;
                next_available--;
                break;
            }
            i++;
        }
        if (i==MAX_RECEIVERS) return NOT_FOUND;
        else return NO_ERROR;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: remove_receiver(Mailbox_Handle m)
* Return type: Boolean. True if receiver is removed successfully. False if it is not encountered
* Comment: Remove receiver in list of receivers. It searches inside of the list, when found,
* the rest of the list is shifted to the right and the receiver is removed.
**********************************************************************************************/
  int Agent_Msg::remove_receiver(Mailbox_Handle m){

      int i=0;
      while(i<MAX_RECEIVERS){
           if(receivers[i]==m){
               while(i<MAX_RECEIVERS-1){
                   receivers[i]=receivers[i+1];
                   i++;
               }
               receivers[MAX_RECEIVERS-1]=NULL;
               next_available--;
               break;
           }
           i++;
       }
       if (i==MAX_RECEIVERS) return NOT_FOUND;
       else return NO_ERROR;
   }
/*********************************************************************************************
* Class: Agent_Msg
* Function: clear_all_receiver();
* Return type: Boolean. True if receiver is removed successfully.
* Comment: Remove receiver in list of receivers
**********************************************************************************************/
    void Agent_Msg::clear_all_receiver(){
        int i=0;
        while (i<MAX_RECEIVERS){
            receivers[i]=NULL;
            i++;
        }
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: receive_msg(Uint32 timeout)
* Return type: Boolean. TRUE if successful, FALSE if timeout
* Comment: Receiving msg in its queue. Block call. The mailbox is obtained from the
*          task handle of the calling function of this object.
**********************************************************************************************/
    bool Agent_Msg::receive(Uint32 timeout){
        UArg arg0,arg1;
        Task_getFunc(self_handle,&arg0, &arg1);
        Mailbox_Handle m=(Mailbox_Handle) arg0;
        return Mailbox_pend(m, (xdc_Ptr) &MsgObj, timeout);
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: send()
* Return type: Boolean. TRUE if successful, FALSE if timeout
* Comment: Send msg to specific mailbox.
*          Set the MsgObj handle to sender's handle.
**********************************************************************************************/
    bool Agent_Msg::send(Mailbox_Handle m){
        MsgObj.handle=self_handle;
        return Mailbox_post(m, (xdc_Ptr)&MsgObj, BIOS_NO_WAIT);

    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: send()
* Return type: Boolean. TRUE if all msgs are sent successfully to all receivers
*                       FALSE if there were an error.
* Comment: Iterate over the list. If there is an error for any receiver, will return false
*          if there is any error.
**********************************************************************************************/
    bool Agent_Msg::send(){
        int i=0;
        bool no_error=true;
        MsgObj.handle=self_handle;

        while (i<next_available){
            if(!Mailbox_post(receivers[i], (xdc_Ptr)&MsgObj, BIOS_NO_WAIT)) no_error=false;
            i++;
        }

        return no_error;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: set_msg_type(int type)
* Return type: NULL
* Comment: Set message type according to FIPA ACL
**********************************************************************************************/
    void Agent_Msg::set_msg_type(int msg_type){
        MsgObj.type=msg_type;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: set_msg_body(String body)
* Return type: NULL
* Comment: Set message body according to FIPA ACL
**********************************************************************************************/
    void Agent_Msg::set_msg_body(String msg_body){
        MsgObj.body=msg_body;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: get_msg_type()
* Return type: int
* Comment: Get message type
**********************************************************************************************/
    int Agent_Msg::get_msg_type(){
        return MsgObj.type;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: get_msg_body()
* Return type: String
* Comment: Get message body
**********************************************************************************************/
    String Agent_Msg::get_msg_body(){
        return MsgObj.body;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: get_sender()
* Return type: String
* Comment: Get sender name
**********************************************************************************************/
    String Agent_Msg::get_sender(){
        return Task_Handle_name(MsgObj.handle);
    }


/*********************************************************************************************
* Class: Agent_Msg
* Function: broadcast_AP(Agent_Management_Services AP)
* Return type: bool. Returns True if was sent correctly to all
* Comment: broadcast message to all agents of the platform
**********************************************************************************************/
    bool Agent_Msg::broadcast_AP(Agent_Management_Services AP){
        int size=AP.return_list_size();
        int i=0;
        MsgObj.handle=self_handle;
        UArg arg0,arg1;
        Mailbox_Handle m;
        bool no_error=true;

        while(i<size){
            //System_printf("test %x\n",AP.return_list()[i]);
            //System_flush();
            Task_getFunc(AP.return_list()[i],&arg0, &arg1);
            m=(Mailbox_Handle) arg0;
            if(!Mailbox_post(m, (xdc_Ptr)&MsgObj, BIOS_NO_WAIT))no_error=false;
            i++;
        }
        return no_error;

    }


    void Agent_Msg::print(){
        int i=0;
        while (i<MAX_RECEIVERS){

            System_printf("size %d, %x\n",next_available,receivers[i]);
            System_flush();
            i++;
        }
    }

    void Agent_Management_Services::print(){
        int i=0;
    //        Mailbox_Handle m;
    //        while (i<AGENT_LIST_SIZE){
    //            m=(Mailbox_Handle) Task_getEnv(Agent_Handle[i]);
    //            System_printf("mail: %x, aid: %x\n",m,Agent_Handle[i]);
    //            System_flush();
    //            i++;
    //        }

        while (i<AGENT_LIST_SIZE){
            System_printf("aid: %x\n", return_list()[i]);
            System_flush();
            i++;
        }
    }

};

