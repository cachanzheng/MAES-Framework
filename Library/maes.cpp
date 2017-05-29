/*
 * maes.cpp
 *
 *  Created on: 14 May 2017
 *      Author: Carmen Chan-Zheng
 */

#include "maes.h"

namespace MAES{

/*********************************************************************************************
*
*                         Class: Agent_AMS: Agent Management Services
*
********************************************************************************************/
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
            Task_setEnv(Agent_Handle[i],NULL);
            i++;
        }
        AP.ptrAgent_Handle=Agent_Handle;
        AP.name=name;

    }

/*********************************************************************************************
* Class: Agent_Management_Services
* Function: init();
* Comment: Create AP and register all agents. No special behaviour
**********************************************************************************************/
   void Agent_Management_Services::init(){
    /*Initializing all the previously created task*/
    Task_Handle temp;
    temp=Task_Object_first();

    while (temp!=NULL){
        register_agent(temp);
        temp=Task_Object_next(temp);
    }
  }
 /*********************************************************************************************
* Class: Agent_Management_Services
* Function: init();
* Comment: Create AMS task with default stack of 1024
*          In case that user needs to add a service or special action to the AMS
**********************************************************************************************/
    bool Agent_Management_Services::init(Task_FuncPtr action){

            Task_Handle temp;
            Mailbox_Handle mailbox_handle;
            Mailbox_Params mbxParams;
            Task_Params taskParams;
            /*Creating mailbox*/
            Mailbox_Params_init(&mbxParams);
            mailbox_handle= Mailbox_create(12,5,&mbxParams,NULL);

            /*Creating task*/
            Task_Params_init(&taskParams);
            taskParams.stack=task_stack;
            taskParams.stackSize = 1024;
            taskParams.priority = Task_numPriorities-1;//Assigning max priority
            taskParams.instance->name=AP.name;
            taskParams.arg0=(UArg)mailbox_handle;
            AP.AMS_aid = Task_create(action, &taskParams, NULL);

            if (AP.AMS_aid!=NULL){
            /*Initializing all the previously created task*/
                temp=Task_Object_first();
                while (temp!=NULL){
                    if(temp==AP.AMS_aid) break;
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
   bool Agent_Management_Services::init(Task_FuncPtr action,int taskstackSize){

           Task_Handle temp;
        //   char* task_stack=new char[stackSize];
           Mailbox_Handle mailbox_handle;
           Mailbox_Params mbxParams;
           Task_Params taskParams;

           /*Creating mailbox*/
           Mailbox_Params_init(&mbxParams);
           mailbox_handle= Mailbox_create(12,5,&mbxParams,NULL);

           /*Creating task*/
           Task_Params_init(&taskParams);
           taskParams.stack=new char[taskstackSize];
           taskParams.stackSize = taskstackSize;
           taskParams.priority = Task_numPriorities-1;
           taskParams.instance->name=AP.name;
           taskParams.arg0=(UArg)mailbox_handle;
           AP.AMS_aid = Task_create(action, &taskParams, NULL);

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

        if (!search(aid)){ //If it is not found then register
            if(next_available<AGENT_LIST_SIZE){
                Task_setEnv(aid,AP.name);
                Agent_Handle[next_available]=aid;
                next_available++;
                return NO_ERROR;
            }

            else return LIST_FULL;
        }
        else return DUPLICATED;

    }
/*********************************************************************************************
* Class: Agent_Management_Services
* Function: int register_agent(Agent_Build agent)
* Comment: register agent to the platform. If list is full, delete agent and mailbox
**********************************************************************************************/
    int Agent_Management_Services::register_agent(Agent_Build agent){
        Task_Handle aid;
        aid=agent.get_AID();

        if(!search(aid)){
            if (aid==NULL) return HANDLE_NULL;
            if(next_available<AGENT_LIST_SIZE){
                Task_setEnv(aid,AP.name);
                Agent_Handle[next_available]=aid;
                next_available++;
                return NO_ERROR;
            }

            else return LIST_FULL;
        }
        else return DUPLICATED;

    }
/*********************************************************************************************
* Class: Agent_Management_Services
* Function:  int kill_agent(Agent_Build agent);
* Comment: Kill agent on the platform. It searches inside of the list, when found,
* the rest of the list is shifted to the right and the agent is removed.
**********************************************************************************************/
    int Agent_Management_Services::kill_agent(Agent_Build agent){
        Task_Handle aid=agent.get_AID();
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
* Function:  int kill_agent(Task_Handle aid);
* Comment: Kill agent on the platform. It searches inside of the list, when found,
* the rest of the list is shifted to the right and the agent is removed.
**********************************************************************************************/
    int Agent_Management_Services::kill_agent(Task_Handle aid){
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
* Function:  int deregister_agent(Agent_Build agent);
* Comment: Kill agent on the platform. It searches inside of the list, when found,
* the rest of the list is shifted to the right and the agent is removed.
**********************************************************************************************/
    int Agent_Management_Services::deregister_agent(Agent_Build agent){
        Task_Handle aid=agent.get_AID();
        int i=0;

        while(i<AGENT_LIST_SIZE){
              if(Agent_Handle[i]==aid){
                  suspend(aid);
                  Task_setEnv(aid,NULL);
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
* Function:  int deregister_agent(Task_Handle aid);
* Comment: Kill agent on the platform. It searches inside of the list, when found,
* the rest of the list is shifted to the right and the agent is removed.
**********************************************************************************************/
    int Agent_Management_Services::deregister_agent(Task_Handle aid){
        int i=0;

        while(i<AGENT_LIST_SIZE){
              if(Agent_Handle[i]==aid){
                      Task_setEnv(aid,NULL);
                      suspend(aid);
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
* Function: bool (Task_Handle aid,String new_AP)
* Return:
* Comment: Modifies Agent Platform name of the agent (Used if needed to migrate)
*          Search AID within list and return true if modified correctly
**********************************************************************************************/
    bool Agent_Management_Services::modify_agent(Task_Handle aid,String new_AP){

        if(search(aid)){
            Task_setEnv(aid, new_AP);
            return true;
        }

        else return false;
    }

/*********************************************************************************************
* Class: Agent_Management_Services
* Function: bool modify_agent(Agent build agent, String new_AP);
* Return:
* Comment: Modifies Agent Platform name of the agent (Used if needed to migrate)
*          Search AID within list and return true if modified correctly
**********************************************************************************************/
    bool Agent_Management_Services::modify_agent(Agent_Build agent,String new_AP){

        if(search(agent)){
            Task_setEnv(agent.get_AID(), new_AP);
            return true;
        }

        else return false;
    }

/*********************************************************************************************
* Class: Agent_Management_Services
* Function:  bool search();
* Return: Bool
* Comment: Search AID within list and return true if found.
*          next_available is used instead of AGENT_LIST_SIZE since it will optimize
*          search
**********************************************************************************************/
    bool Agent_Management_Services::search(Agent_Build agent){
        int i=0;

          while(i<next_available){
              if (Agent_Handle[i]==agent.get_AID()) break;
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
* Function:void suspend(Agent_Build agent)
* Return:  NULL
* Comment: Suspend Agent. Set it to inactive by setting priority to -1
**********************************************************************************************/
    void Agent_Management_Services::suspend(Agent_Build agent){
        Task_setPri(agent.get_AID(), -1);
   }

/*********************************************************************************************
* Class: Agent_Management_Services
* Function:void suspend(Task_Handle aid)
* Return:  NULL
* Comment: Suspend Agent. Set it to inactive by setting priority to -1
**********************************************************************************************/
    void Agent_Management_Services::suspend(Task_Handle aid){
        Task_setPri(aid, -1);
   }
/*********************************************************************************************
* Class: Agent_Management_Services
* Function:void restore(Agent_Build agent)
* Return:  NULL
* Comment: Restore Agent.
**********************************************************************************************/
    void Agent_Management_Services::resume(Agent_Build agent){
        Task_setPri(agent.get_AID(), agent.get_priority());
   }

/*********************************************************************************************
* Class: Agent_Management Services
* Function: void wait (uint32 ticks)
* Return type: NULL
* Comments: When called within agent's function it will make agent sleeps defined ticks
*********************************************************************************************/
   void Agent_Management_Services::wait(Uint32 ticks){
        Task_sleep(ticks);
    }
/*********************************************************************************************
* Class: Agent_Management Services
* Function: void yield()
* Return type: NULL
* Comments: It yields the processor to another readied agent of equal priority.
*           It lower priorities are on readied, the current task won't be preempted
*********************************************************************************************/
  void Agent_Management_Services::agent_yield(){
       Task_yield();
   }

/*********************************************************************************************
* Class: Agent_Management_Services
* Function:void get_mode
* Return:  NULL
* Comment: get running mode of agent
**********************************************************************************************/
    int Agent_Management_Services:: get_mode(Agent_Build agent){
        Task_Mode mode;
        mode=Task_getMode(agent.get_AID());

        switch(mode){
        case Task_Mode_READY:
            return ACTIVE;

        case Task_Mode_BLOCKED:
            return WAITING;

        case Task_Mode_INACTIVE:
            return SUSPENDED;

        case Task_Mode_TERMINATED:
            return TERMINATED;

        default: return NULL;

        }

      }
/*********************************************************************************************
* Class: Agent_Management_Services
* Function:  Task_Handle* list()
* Return: Pointer to the of the agents in the platform.
* Comment:
**********************************************************************************************/
    Task_Handle* Agent_Management_Services::get_all_subscribers(){
        return AP.ptrAgent_Handle;
    }

/*********************************************************************************************
* Class: Agent_Management_Services
* Function:  int number_of_subscribers()
* Return: Int
* Comment: Returns size of the list of agents registered in the AP
**********************************************************************************************/
    int Agent_Management_Services::number_of_subscribers(){
        return next_available;
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
* Function: get_AMS_AID();
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
                             Task_FuncPtr b,
                             int pri){


        /*Task init*/
        agent_name=name;
        priority=pri;
        behaviour=b;
        aid=NULL;
        Task_setEnv(aid,NULL);

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

        /*Task init*/
        agent_name=name;
        priority=1;
        behaviour=b;
        aid=NULL;
        Task_setEnv(aid,NULL);

    }
/*********************************************************************************************
 * Class: Agent_Build
 * Function: void create_agent()
 * Return type: NULL
 * Comments: Create the task and mailbox associated with the agent. The mailbox handle is
 *           embedded in task's handle env variable.
*********************************************************************************************/
    Task_Handle Agent_Build::create_agent(){

            Task_Params taskParams;
            Mailbox_Handle mailbox_handle;
            Mailbox_Params mbxParams;

            /*Creating mailbox
             * Msg size is 12 and default queue size is 3*/
            Mailbox_Params_init(&mbxParams);
            mailbox_handle= Mailbox_create(12,5,&mbxParams,NULL);

            /*Creating task*/
            Task_Params_init(&taskParams);
            taskParams.stack=task_stack;
            taskParams.stackSize =1024;
            taskParams.priority = priority;
            taskParams.instance->name=agent_name;
            taskParams.arg0=(UArg)mailbox_handle;
            aid = Task_create(behaviour, &taskParams, NULL);
            return aid;

    }

 /*********************************************************************************************
 * Class: Agent_Build
 * Function: void create_agent(int taskstackSize)
 * Return type: NULL
 * Comments: Create the task and mailbox associated with the agent. The mailbox handle is
 *           embedded in task's handle env variable.
 *           With user custom taskstackSize. Be aware of heap size
*********************************************************************************************/
    Task_Handle Agent_Build::create_agent(int taskstackSize, int queueSize){
//
            Task_Params taskParams;
            Mailbox_Handle mailbox_handle;
            Mailbox_Params mbxParams;

            /*Creating mailbox
            * Msg size is 12 and default queue size is 5*/
            Mailbox_Params_init(&mbxParams);
            mailbox_handle= Mailbox_create(12,queueSize,&mbxParams,NULL);

            /*Creating task*/
            Task_Params_init(&taskParams);
            taskParams.stack=new char[taskstackSize];
            taskParams.stackSize =taskstackSize;
            taskParams.priority = priority;
            taskParams.instance->name=agent_name;
            taskParams.arg0=(UArg)mailbox_handle;
            aid = Task_create(behaviour, &taskParams, NULL);


            return aid;

    }
/*********************************************************************************************
* Class: Agent_Build
* Function: void destroy_agent
* Return type: NULL
* Comment: Auto destroy, Make sure is de-registered first in AP
*********************************************************************************************/
    void Agent_Build::destroy_agent(){

        UArg arg0,arg1;
        Task_getFunc(aid,&arg0, &arg1);

        Task_delete(&aid);
        Mailbox_Handle m=(Mailbox_Handle) arg0;
        Mailbox_delete(&m);
    }

/*********************************************************************************************
 * Class: Agent_Build
 * Function: get_name()
 * Return type: String
 * Comments: Returns agent's name
*********************************************************************************************/
    String Agent_Build::get_name(){
        return agent_name;
    }

/*********************************************************************************************
 * Class: Agent_Build
 * Function: get_AP()
 * Return type: String
 * Comments: Returns AP name where agent is living
*********************************************************************************************/
    String Agent_Build::get_AP(){
        return (String) Task_getEnv(aid);
    }
/*********************************************************************************************
 * Class: Agent_Build
 * Function: get_priority()
 * Return type: Int
 * Comments: Returns Agent's priority
*********************************************************************************************/
    int Agent_Build::get_priority(){
        return priority;
    }

/*********************************************************************************************
* Class: Agent_Build
* Function:  get_task_handle(){
* Return type: Task Handle
* Comments: Returns agent's task handle
*********************************************************************************************/
    Task_Handle Agent_Build::get_AID(){
        return aid;
    }

/*********************************************************************************************
* Class: Agent_Build
* Function:  bool isRegistered()
* Return type: NULL
* Comments: Returns true if agent is registered to any platform
*********************************************************************************************/
    bool Agent_Build::isRegistered(){
        if ((String) Task_getEnv(aid)!=NULL) return true;
        else return false;
    }
/*********************************************************************************************
*
*                                  Class: Agent_Msg
*
********************************************************************************************
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
* Function: Mailbox_Handle get_mailbox_from_aid(Agent_Build agent); Private
* Return type: Mailbox_Handle
* Comment: Obtain mailbox information from agent aid
**********************************************************************************************/
  Mailbox_Handle Agent_Msg::get_mailbox(Task_Handle aid){
      UArg arg0,arg1;
      Task_getFunc(aid,&arg0, &arg1);
      return (Mailbox_Handle) arg0;

  }

/*********************************************************************************************
* Class: Agent_Msg
* Function: bool isRegistered(Task_Handle aid); Private
* Return type: Boolean
* Comment: if returns false, sender or receiver is not registered in the same platform
**********************************************************************************************/
  bool Agent_Msg::isRegistered(Task_Handle aid){
      if(Task_getEnv(aid)==Task_getEnv(self_handle)) return true;
      else return false;
}
/*********************************************************************************************
* Class: Agent_Msg
* Function: add_receiver(Task_Handle aid)
* Return type: Boolean. True if receiver is added successfully.
* Comment: Add receiver to list of receivers by using the agent's aid
**********************************************************************************************/
   int Agent_Msg::add_receiver(Task_Handle aid){

        if(isRegistered(aid)){
           if (aid==NULL) return HANDLE_NULL;

           if(next_available<MAX_RECEIVERS){
               receivers[next_available]=aid;
               next_available++;
               return NO_ERROR;
           }

           else return LIST_FULL;

       }
       else return NOT_FOUND;
   }


/*********************************************************************************************
* Class: Agent_Msg
* Function: remove_receiver(Task_Handle aid)
* Return type: Boolean. True if receiver is removed successfully. False if it is not encountered
* Comment: Remove receiver in list of receivers. It searches inside of the list, when found,
* the rest of the list is shifted to the right and the receiver is removed.
**********************************************************************************************/
   int Agent_Msg::remove_receiver(Task_Handle aid){

        int i=0;
        while(i<MAX_RECEIVERS){
            if(receivers[i]==aid){
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
        return Mailbox_pend(get_mailbox(self_handle), (xdc_Ptr) &MsgObj, timeout);
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: send()
* Return type: Boolean. TRUE if successful, FALSE if timeout
* Comment: Send msg to specific mailbox.
*          Set the MsgObj handle to sender's handle.
**********************************************************************************************/
    int Agent_Msg::send(Task_Handle aid){

        if(isRegistered(aid)){
            Mailbox_Handle m;
            UArg arg0,arg1;
            Task_getFunc(aid,&arg0, &arg1);
            m= (Mailbox_Handle) arg0;
            MsgObj.handle=self_handle;
            if(Mailbox_post(m, (xdc_Ptr)&MsgObj, BIOS_NO_WAIT)) return NO_ERROR;
            else return TIMEOUT;
        }
        else return NOT_FOUND;

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
            if(isRegistered(receivers[i])){
                if(!Mailbox_post(get_mailbox(receivers[i]), (xdc_Ptr)&MsgObj, BIOS_NO_WAIT))
                    no_error=false;
            }
            i++;
        }

        return no_error;
    }
 /*********************************************************************************************
 * Class: Agent_Msg
 * Function: broadcast_AP(Task_Handle *AP_list)
 * Return type: bool. Returns True if was sent correctly to all
 * Comment: broadcast message to all agents of the platform
 **********************************************************************************************/
     bool Agent_Msg::broadcast_AP(Task_Handle* list){
         int i=0;
         MsgObj.handle=self_handle;
         UArg arg0,arg1;
         Mailbox_Handle m;
         bool no_error=true;


         while(list[i]!=NULL){
             Task_getFunc(list[i],&arg0, &arg1);
             m=(Mailbox_Handle) arg0;
             if(!Mailbox_post(m, (xdc_Ptr)&MsgObj, BIOS_NO_WAIT))no_error=false;
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
    Task_Handle Agent_Msg::get_sender(){
        return MsgObj.handle;
    }
};

