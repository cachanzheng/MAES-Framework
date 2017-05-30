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
*                                  Class: Agent
*
**********************************************************************************************
**********************************************************************************************
* Class: Agent
* Function: Agent constructor
**********************************************************************************************/
    Agent::Agent(String name,
                 Task_FuncPtr b){

        description.agent_name=name;
        behaviour=b;
        aid=NULL;
        Task_setEnv(aid,NULL);
    }
/*********************************************************************************************
 * Class: Agent
 * Function: bool init_agent()
 * Return type: Boolean
 * Comments: Create the task and mailbox associated with the agent. The Agent's description
 *           struct is stored in task's environment variable.
 *           Priority set to -1, only set when the agent is registered
*********************************************************************************************/
    bool Agent::init_agent(){

        Task_Params taskParams;
        Mailbox_Params mbxParams;

        description.AP=NULL;
        description.priority= 1;

        /*Creating mailbox: Msg size is 12 and default queue size is set to 3*/
        Mailbox_Params_init(&mbxParams);
        description.mailbox_handle= Mailbox_create(12,5,&mbxParams,NULL);

        /*Creating task*/
        Task_Params_init(&taskParams);
        taskParams.stack=task_stack;
        taskParams.stackSize =1024;
        taskParams.priority = -1;
        taskParams.instance->name=description.agent_name; //To do: take that out to optimize
        taskParams.env=&description;
        aid = Task_create(behaviour, &taskParams, NULL);
        if (aid!=NULL) return true;
        else return false;

    }

 /*********************************************************************************************
 * Class: Agent
 * Function: bool init_agent(int taskstackSize,int queueSize, int priority)
 * Return type: bool
 * Comments: Create the task and mailbox associated with the agent. The Agent's description
 *           struct is stored in task's environment variable.
 *           This init is with user custom's task stack size, queue size and priority.
 *           Be aware of heap size
 *           Priority set to -1, only set when the agent is registered
*********************************************************************************************/
    bool Agent::init_agent(int taskstackSize, int queueSize, int priority){

        Task_Params taskParams;
        Mailbox_Params mbxParams;

        description.AP=NULL;
        description.priority=priority;

        /*Creating mailbox
        * Msg size is 12*/
        Mailbox_Params_init(&mbxParams);
        description.mailbox_handle=Mailbox_create(12,queueSize,&mbxParams,NULL);

        /*Creating task*/
        Task_Params_init(&taskParams);
        taskParams.stack=new char[taskstackSize];
        taskParams.stackSize =taskstackSize;
        taskParams.priority = -1;
        taskParams.instance->name=description.agent_name;
        taskParams.env=&description;
        aid = Task_create(behaviour, &taskParams, NULL);

        if (aid!=NULL) return true;
        else return false;
    }
/*********************************************************************************************
* Class: Agent
* Function: bool init_agent(int taskstackSize,int queueSize, int priority)
* Return type: bool
* Comments: Create the task and mailbox associated with the agent. The Agent's description
*           struct is stored in task's environment variable.
*           This init is with user custom arguments.
*           Priority set to -1, only set when the agent is registered
*********************************************************************************************/
    bool Agent::init_agent(UArg arg0, UArg arg1){

        Task_Params taskParams;
        Mailbox_Params mbxParams;

        description.AP=NULL;
        description.priority=1;

        /*Creating mailbox: Msg size is 12 and default queue size is set to 3*/
        Mailbox_Params_init(&mbxParams);
        description.mailbox_handle= Mailbox_create(12,5,&mbxParams,NULL);

        /*Creating task*/
        Task_Params_init(&taskParams);
        taskParams.stack=task_stack;
        taskParams.stackSize =1024;
        taskParams.priority = -1;
        taskParams.instance->name=description.agent_name; //To do: take that out to optimize
        taskParams.arg0=arg0;
        taskParams.arg1=arg1;
        taskParams.env=&description;
        aid = Task_create(behaviour, &taskParams, NULL);

        if (aid!=NULL) return true;
        else return false;

    }

/*********************************************************************************************
* Class: Agent
* Function: bool init_agent(int taskstackSize,int queueSize, int priority)
* Return type: bool
* Comments: Create the task and mailbox associated with the agent. The Agent's description
*           struct is stored in task's environment variable.
*           This init is with user custom arguments, task stack size, queue size and priority.
*           Be aware of heap size. Priority set to -1, only set when the agent is registered
*********************************************************************************************/
    bool Agent::init_agent(int taskstackSize, int queueSize, int priority,UArg arg0, UArg arg1){

        Task_Params taskParams;
        Mailbox_Params mbxParams;

        description.AP=NULL;
        description.priority=priority;

        /*Creating mailbox
        * Msg size is 12*/
        Mailbox_Params_init(&mbxParams);
        description.mailbox_handle=Mailbox_create(12,queueSize,&mbxParams,NULL);

        /*Creating task*/
        Task_Params_init(&taskParams);
        taskParams.stack=new char[taskstackSize];
        taskParams.stackSize =taskstackSize;
        taskParams.priority =-1;
        taskParams.instance->name=description.agent_name;
        taskParams.arg0=arg0;
        taskParams.arg1=arg1;
        taskParams.env=&description;
        aid = Task_create(behaviour, &taskParams, NULL);

        if (aid!=NULL) return true;
        else return false;
    }

/*********************************************************************************************
* Class: Agent
* Function: void destroy_agent();
* Return type: Boolean
* Comment: Destroy agent if only is de-registered to AP. Returns true if destroyed
*          succesfully
*********************************************************************************************/
    bool Agent::destroy_agent(){
        if (!isRegistered()){
            Task_setEnv(aid,NULL);
            Mailbox_Handle m= description.mailbox_handle;
            Mailbox_delete(&m);
            Task_delete(&aid);
            return true;
        }
        else return false;
    }

/*********************************************************************************************
* Class: Agent
* Function:  get_task_handle(){
* Return type: Task Handle
* Comments: Returns agent's task handle
*********************************************************************************************/
    Task_Handle Agent::get_AID(){
        return aid;
    }

/*********************************************************************************************
* Class: Agent
* Function:  bool isRegistered()
* Return type: NULL
* Comments: Returns true if agent is registered to any platform
*********************************************************************************************/
    bool Agent::isRegistered(){
        if (description.AP!=NULL)return true;
        else return false;
    }
/*********************************************************************************************
*
*                         Class: Agent_AMS: Agent Management Services
*
********************************************************************************************/
/*********************************************************************************************
* Class: Agent_Management_Services
* Function: Agent_Management_Services Constructor
* Comment: Initialize list of agents task handle and its environment to NULL.
**********************************************************************************************/
    Agent_Management_Services::Agent_Management_Services(String name){
        int i=0;
        next_available=0;
        while (i<AGENT_LIST_SIZE){
            AP.Agent_Handle[i]=(Task_Handle) NULL;
            Task_setEnv(AP.Agent_Handle[i],NULL);
            i++;
        }
        AP.name=name;
    }
/*********************************************************************************************
* Class: Agent_Management_Services
* Function: int register_agent(Task_Handle aid)
* Comment: Register agent to the platform only if it is unique.
**********************************************************************************************/
    int Agent_Management_Services::register_agent(Task_Handle aid){
        if (aid==NULL) return HANDLE_NULL;

        if (!search(aid)){
            if(next_available<AGENT_LIST_SIZE){
                Agent_info *description;
                description=(Agent_info *)Task_getEnv(aid);
                description->AP=AP.name;
                AP.Agent_Handle[next_available]=aid;
                next_available++;
                Task_setPri(aid, description->priority);
                return NO_ERROR;
            }

            else return LIST_FULL;
        }
        else return DUPLICATED;
    }

/*********************************************************************************************
* Class: Agent_Management_Services
* Function: void init()
* Return: NULL
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
* Function: bool init(Task_FuncPtr action);
* Return: Boolean
* Comment: Create AMS task with default stack of 1024
*          In case that user needs to add a service or special action to the AMS
**********************************************************************************************/
    bool Agent_Management_Services::init(Task_FuncPtr action){

        Task_Handle temp;
        Mailbox_Params mbxParams;
        Task_Params taskParams;

        /*Initializing Agent_info*/
        description.AP=AP.name;
        description.agent_name=AP.name;
        description.priority=Task_numPriorities-1;

        /*Creating mailbox*/
        Mailbox_Params_init(&mbxParams);
        description.mailbox_handle= Mailbox_create(12,5,&mbxParams,NULL);

        /*Creating task*/
        Task_Params_init(&taskParams);
        taskParams.stack=task_stack;
        taskParams.stackSize = 1024;
        taskParams.priority = description.priority;//Assigning max priority
        taskParams.instance->name=description.AP;
        taskParams.env=&description;
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
* Function: bool init(Task_FuncPtr action,int taskstackSize);;
* Comment: Create AMS task with user custom stack size
*          Be aware of heap size
**********************************************************************************************/
   bool Agent_Management_Services::init(Task_FuncPtr action,int taskstackSize){

        Task_Handle temp;
        Mailbox_Params mbxParams;
        Task_Params taskParams;

        /*Initializing Agent_info*/
        description.AP=AP.name;
        description.agent_name=AP.name;
        description.priority=Task_numPriorities-1;

        /*Creating mailbox*/
        Mailbox_Params_init(&mbxParams);
        description.mailbox_handle= Mailbox_create(12,5,&mbxParams,NULL);

           /*Creating task*/
           Task_Params_init(&taskParams);
           taskParams.stack=new char[taskstackSize];
           taskParams.stackSize = taskstackSize;
           taskParams.priority =  description.priority;
           taskParams.instance->name=AP.name;
           taskParams.env=&description;
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
* Function:  int kill_agent(Task_Handle aid);
* Return: int
* Comment: Kill agent on the platform. It deregisters the agent first then it delete the
*           handles
**********************************************************************************************/
    int Agent_Management_Services::kill_agent(Task_Handle aid){
        int error;
        error=deregister_agent(aid);

        if(error==NO_ERROR){
            Agent_info *description;
            Mailbox_Handle m;
            description=(Agent_info *)Task_getEnv(aid);
            m=description->mailbox_handle;
            Task_delete(&aid);
            Mailbox_delete(&m);
        }

        return error;
    }

/*********************************************************************************************
* Class: Agent_Management_Services
* Function:  int deregister_agent(Task_Handle aid);
* Comment: Deregister agent on the platform. It searches inside of the list, when found,
*          the rest of the list is shifted to the right and the agent is removed.
*          Remove the receiver manually from all the msg list;
**********************************************************************************************/
    int Agent_Management_Services::deregister_agent(Task_Handle aid){
        int i=0;

        while(i<AGENT_LIST_SIZE){
              if(AP.Agent_Handle[i]==aid){
                      Agent_info *description;
                      suspend(aid);
                      description=(Agent_info *)Task_getEnv(aid);
                      description->AP=NULL;
                      while(i<AGENT_LIST_SIZE-1){
                          AP.Agent_Handle[i]=AP.Agent_Handle[i+1];
                      i++;
                  }
                  AP.Agent_Handle[AGENT_LIST_SIZE-1]=NULL;
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
* Function: bool modify_agent (Task_Handle aid,String new_AP)
* Return:
* Comment: Modifies Agent Platform name of the agent (Used if needed to migrate)
*          Search AID within list and return true if modified correctly
**********************************************************************************************/
    bool Agent_Management_Services::modify_agent(Task_Handle aid,String new_AP){

        if(search(aid)){
            Agent_info *description;
            suspend(aid);
            description=(Agent_info *)Task_getEnv(aid);
            description->AP=new_AP;
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
    bool Agent_Management_Services::search(Task_Handle aid){
        int i=0;

          while(i<next_available){
              if (AP.Agent_Handle[i]==aid) break;
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
              if (!strcmp(Task_Handle_name(AP.Agent_Handle[i]),name)) break;
              i++;
          }

          if (i==next_available) return false;
          else return true;
   }

/*********************************************************************************************
* Class: Agent_Management_Services
* Function:void suspend(Task_Handle aid)
* Return:  NULL
* Comment: Suspend Agent. Set it to inactive by setting priority to -1
**********************************************************************************************/
    void Agent_Management_Services::suspend(Task_Handle aid){
        if(search(aid)) Task_setPri(aid, -1);
   }
/*********************************************************************************************
* Class: Agent_Management_Services
* Function:void restore(Agent agent)
* Return:  NULL
* Comment: Restore Agent.
**********************************************************************************************/
    void Agent_Management_Services::resume(Task_Handle aid){
        if(search(aid)) {
            Agent_info *description;
            description=(Agent_info *)Task_getEnv(aid);
            Task_setPri(aid,description->priority);
        }
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
* Class: Agent_Management_Services
* Function:void get_mode
* Return:  NULL
* Comment: get running mode of agent
**********************************************************************************************/
    int Agent_Management_Services:: get_mode(Task_Handle aid){
        if(search(aid)){
            Task_Mode mode;
            mode=Task_getMode(aid);

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
        else return NULL;
    }
/*********************************************************************************************
* Class: Agent_Management_Services
* Function: AP_Description* get_AP_description();
* Return: AP_Description
* Comment: Returns description of the platform Returns copy instead of pointer since pointer
*          can override the information.
**********************************************************************************************/
    AP_Description Agent_Management_Services::get_AP_description(){
        return AP;
    }

/*********************************************************************************************
* Class: Agent_Management_Services
* Function: AP_Description* get_Agent_description();
* Return: Agent_info
* Comment: Returns description of an agent. Returns copy instead of pointer since pointer
*          can override the information.
**********************************************************************************************/
    Agent_info Agent_Management_Services::get_Agent_description(Task_Handle aid){
        Agent_info agent;
        Agent_info *description;
        description=(Agent_info *)Task_getEnv(aid);

        agent.AP=description->AP;
        agent.agent_name=description->agent_name;
        agent.mailbox_handle=description->mailbox_handle;
        agent.priority=description->priority;
        return agent;
    }

/*********************************************************************************************
* Class: Agent_Management_Services
* Function: get_AMS_AID();
* Return: Task Handle of the AMS
* Comment: returns AMS task handle in case that exists
**********************************************************************************************/
    Task_Handle Agent_Management_Services::get_AMS_AID(){
        return AP.AMS_aid;
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
* Function: get_AMS_AID();
* Return: Task Handle of the AMS
* Comment: returns if was successful
**********************************************************************************************/
   bool Agent_Management_Services::set_agent_pri(Task_Handle aid,int pri){
        if (search(aid)){
            Agent_info *description=(Agent_info *)Task_getEnv(aid);
            Task_setPri(aid,pri);
            description->priority=pri;
            return true;
        }
        else return false;
    }
/*********************************************************************************************
* Class: Agent_Management_Services
* Function: broadcast(MsgObj &msg)
* Return: NULL
* Comment: broadcast the msg to all subscribers
**********************************************************************************************/
   void Agent_Management_Services::broadcast(MsgObj *msg){
       Agent_info * description;
       msg->handle=Task_self(); //Set sender
       int i=0;

       while(i<next_available){
           if(msg->handle!=AP.Agent_Handle[i]){
               description=(Agent_info*)Task_getEnv(AP.Agent_Handle[i]);
               Mailbox_post(description->mailbox_handle, (xdc_Ptr)msg, BIOS_NO_WAIT);
           }
           i++;
       }
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
      Agent_info *description=(Agent_info*)Task_getEnv(Task_self());
      self_handle=Task_self();
      self_mailbox=description->mailbox_handle;
      clear_all_receiver();
      next_available=0;

   }
/*********************************************************************************************
* Class: Agent_Msg
* Function: private bool isRegistered(Task_Handle aid);
* Return type: Boolean
* Comment: if returns false, sender or receiver is not registered in the same platform
**********************************************************************************************/
  bool Agent_Msg::isRegistered(Task_Handle aid){
      Agent_info *description_receiver=(Agent_info *)Task_getEnv(aid);
      Agent_info *description_sender=(Agent_info *)Task_getEnv(self_handle);

      if(description_receiver->AP==description_sender->AP) return true;
      else return false;
}
/*********************************************************************************************
* Class: Agent_Msg
* Function: add_receiver(Task_Handle aid)
* Return type: Boolean. True if receiver is added successfully.
* Comment: Add receiver to list of receivers by using the agent's aid
**********************************************************************************************/
   int Agent_Msg::add_receiver(Task_Handle aid_receiver){

        if(isRegistered(aid_receiver)){
           if (aid_receiver==NULL) return HANDLE_NULL;

           if(next_available<MAX_RECEIVERS){
               receivers[next_available]=aid_receiver;
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
* Return type: NULL
* Comment: Clear all receiver in the list
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

        return Mailbox_pend(self_mailbox, (xdc_Ptr) &msg, timeout);
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: send()
* Return type: Boolean. TRUE if successful, FALSE if timeout
* Comment: Send msg to specific mailbox.
*          Set the MsgObj handle to sender's handle.
**********************************************************************************************/
    int Agent_Msg::send(Task_Handle aid_receiver){
        msg.handle=self_handle;
        Agent_info * description;

        if(isRegistered(aid_receiver)){
            description=(Agent_info*)Task_getEnv(aid_receiver);

            if(Mailbox_post(description->mailbox_handle, (xdc_Ptr)&msg, BIOS_NO_WAIT)) return NO_ERROR;
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
        msg.handle=self_handle;
        Agent_info * description;

        while (i<next_available){
            if(isRegistered(receivers[i])){
                description=(Agent_info*)Task_getEnv(receivers[i]);
                if(!Mailbox_post(description->mailbox_handle, (xdc_Ptr)&msg, BIOS_NO_WAIT))
                    no_error=false;
            }
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
        msg.type=msg_type;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: set_msg_body(String body)
* Return type: NULL
* Comment: Set message body according to FIPA ACL
**********************************************************************************************/
    void Agent_Msg::set_msg_body(String msg_body){
        msg.body=msg_body;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: get_msg();
* Return type: MsgObj
* Comment: Get message
**********************************************************************************************/
    MsgObj *Agent_Msg::get_msg(){
        MsgObj *ptr =&msg;
        return ptr;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: get_msg_type()
* Return type: int
* Comment: Get message type
**********************************************************************************************/
    int Agent_Msg::get_msg_type(){
        return msg.type;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: get_msg_body()
* Return type: String
* Comment: Get message body
**********************************************************************************************/
    String Agent_Msg::get_msg_body(){
        return msg.body;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: get_sender()
* Return type: String
* Comment: Get sender name
**********************************************************************************************/
    Task_Handle Agent_Msg::get_sender(){
        return msg.handle;
    }

/*********************************************************************************************
*********************************************************************************************/
    //Debug
    void Agent_Msg::print(){
        int i=0;
        System_printf("------ \n");
                    System_flush();
        while(i<next_available){
            System_printf("subscriber %x \n", receivers[i]);
            System_flush();
            i++;
        }
    }
    void Agent_Management_Services::print(){
        int i=0;

        System_printf("------ \n");
                    System_flush();
        while(i<next_available){
            System_printf("subscriber %x \n",AP.Agent_Handle[i]);
            System_flush();
            i++;
        }

    }

    void Agent::print(){
        System_printf("reg: %d AP: %s name: %s mail:%x prio: %i \n",isRegistered(),description.AP,description.agent_name,description.mailbox_handle,description.priority);
        System_flush();
    }
};

