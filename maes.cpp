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
*                     Unnamed namespace: For using within namespace
*
*********************************************************************************************/

    namespace{
/*********************************************************************************************
*
*                         Class: AMS_Services
*
*********************************************************************************************
*********************************************************************************************
* Class: AMS_Services
* Function: Agent_Services Constructor
* Comment: Initialize list of agents task handle and its environment to NULL.
**********************************************************************************************/
        AMS_Services::AMS_Services(){
            int i=0;
            AP.next_available=0;
            while (i<AGENT_LIST_SIZE){
                AP.Agent_Handle[i]=(Agent) NULL;
                Task_setEnv(AP.Agent_Handle[i],NULL);
                i++;
            }
        }
/*********************************************************************************************
* Class: AMS_Services
* Function: int register_agent(Agent aid)
* Comment: Register agent to the platform only if it is unique by agent's task
**********************************************************************************************/
        int AMS_Services::register_agent(Agent aid){
            if (aid==NULL) return HANDLE_NULL;

            if (!search(aid)){
                if(AP.next_available<AGENT_LIST_SIZE){
                    Agent_info *description;
                    description=(Agent_info *)Task_getEnv(aid);
                    description->AP=AP.AMS_description.AP;
                    AP.Agent_Handle[AP.next_available]=aid;
                    AP.next_available++;
                    Task_setPri(aid, description->priority);
                    return NO_ERROR;
                }
                else return LIST_FULL;
            }
            else return DUPLICATED;
        }
/*********************************************************************************************
* Class: AMS_Services
* Function:  int deregister_agent(Agent aid);
* Comment: Deregister agent on the platform by handle. It searches inside of the list, when found,
*          the rest of the list is shifted to the right and the agent is removed.
**********************************************************************************************/
        int AMS_Services::deregister_agent(Agent aid){
            int i=0;
            while(i<AGENT_LIST_SIZE){
                  if(AP.Agent_Handle[i]==aid){
                          Agent_info *description;
                          suspend_agent(aid);
                          description=(Agent_info *)Task_getEnv(aid);
                          description->AP=NULL;

                          while(i<AGENT_LIST_SIZE-1){
                              AP.Agent_Handle[i]=AP.Agent_Handle[i+1];
                          i++;
                      }
                      AP.Agent_Handle[AGENT_LIST_SIZE-1]=NULL;
                      AP.next_available--;
                      break;
                  }
                  i++;
              }

              if (i==AGENT_LIST_SIZE) return NOT_FOUND;
              else return NO_ERROR;
       }
/*********************************************************************************************
* Class: AMS_Services
* Function:  int kill_agent(Agent aid);
* Return: int
* Comment: Kill agent on the platform. It deregisters the agent first then it delete the
*           handles by agent's handle
**********************************************************************************************/
        int AMS_Services::kill_agent(Agent aid){
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
* Class: AMS_Services
* Function:void suspend_agent(Agent aid)
* Return:  NULL
* Comment: suspend_agent Agent. Set it to inactive by setting priority to -1
**********************************************************************************************/
        bool AMS_Services::suspend_agent(Agent aid){
            if(search(aid)) {
                Task_setPri(aid, -1);
                return true;
            }
            else return false;
       }
/*********************************************************************************************
* Class: AMS_Services
* Function:void restore(Agent agent)
* Return:  NULL
* Comment: Restore Agent.
**********************************************************************************************/
       bool AMS_Services::resume_agent(Agent aid){
            if(search(aid)) {
                Agent_info *description;
                description=(Agent_info *)Task_getEnv(aid);
                Task_setPri(aid,description->priority);
                return true;
            }
            else return false;
       }
/*********************************************************************************************
* Class: AMS_Services
* Function: bool modify_agent (Agent aid,String new_AP)
* Return:
* Comment: Modifies Agent Platform name of the agent (Used if needed to migrate)
*          Search AID within list and return true if modified correctly
*          Suspend agent when modified
**********************************************************************************************/
       bool AMS_Services::modify_agent(Agent aid,Agent new_AP){

           if(search(aid)){
               Agent_info *description;
               suspend_agent(aid);
               description=(Agent_info *)Task_getEnv(aid);
               description->AP=new_AP;
               return true;
           }

           else return false;
       }
/*********************************************************************************************
* Class: AMS_Services
* Function: get_AMS_AID();
* Return: Task Handle of the AMS
* Comment: returns if was successful
**********************************************************************************************/
       bool AMS_Services::set_agent_pri(Agent aid,int pri){
            if (search(aid)){
                Agent_info *description=(Agent_info *)Task_getEnv(aid);
                Task_setPri(aid,pri);
                description->priority=pri;
                return true;
            }
            else return false;
        }
/*********************************************************************************************
* Class: AMS_Services
* Function: broadcast(MsgObj &msg)
* Return: int
* Comment: broadcast the msg to all subscribers as REQUEST type
**********************************************************************************************/
      void AMS_Services::broadcast(MsgObj *msg){
          Agent_info * description;
          int i=0;
          while(i<AP.next_available){
              if(msg->sender_agent!=AP.Agent_Handle[i]){
                  description=(Agent_info*)Task_getEnv(AP.Agent_Handle[i]);
                  Mailbox_post(description->mailbox_handle, (xdc_Ptr)msg, BIOS_NO_WAIT);
              }
              i++;
          }
      }
/*********************************************************************************************
* Class: AMS_Services
* Function: get_AP()
* Comment: Get information of Agent Platform.
**********************************************************************************************/
      AP_Description*AMS_Services::get_AP(){
          return &AP;
      }
/*********************************************************************************************
* Class: AMS_Services
* Function:  bool search();
* Return: Bool
* Comment: Search AID within list and return true if found.
*          next_available is used instead of AGENT_LIST_SIZE since it will optimize
*          search
**********************************************************************************************/
      bool AMS_Services::search(Agent aid){
          int i=0;

            while(i<AP.next_available){
                if (AP.Agent_Handle[i]==aid) break;
                i++;
            }

            if (i==AP.next_available) return false;
            else return true;
     }
/*********************************************************************************************
* Class: AMS_Services
* Function:void get_mode
* Return:  NULL
* Comment: get running mode of agent
**********************************************************************************************/
     int AMS_Services:: get_mode(Agent aid){
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
* Class: -
* Function: AMS_task
* Return: NULL
* Comment: AMS task description. No visible outside of namespace. For internal use only
**********************************************************************************************/
        void AMS_task(UArg arg0, UArg arg1){
            AMS_Services *services=(AMS_Services*)arg0;
            USER_DEF_COND *cond= (USER_DEF_COND*)arg1;
            Agent_Msg msg;
            int error_msg=0;

            while(1){
                msg.receive(BIOS_WAIT_FOREVER);
                if (msg.get_msg_type()==REQUEST){
                  switch(msg.get_msg_int()){
                        case REGISTER:
                            if(cond->register_cond()){
                                error_msg=services->register_agent(msg.get_target_agent());
                                if(error_msg==NO_ERROR){ //To do: extra condition
                                    msg.set_msg_type(CONFIRM);
                                }
                                else msg.set_msg_type(REFUSE);
                            }
                            else msg.set_msg_type(REFUSE);
                            /*Respond*/
                            msg.send(msg.get_sender());
                            break;

                        case DEREGISTER:

                            if(cond->deregister_cond()){
                                error_msg=services->deregister_agent(msg.get_target_agent());
                                if(error_msg==NO_ERROR){ //To do: extra condition
                                    msg.set_msg_type(CONFIRM);
                                }
                                else msg.set_msg_type(REFUSE);
                            }
                            else msg.set_msg_type(REFUSE);
                            /*Respond*/
                            msg.send(msg.get_sender());
                            break;
                        case KILL:
                            if(cond->kill_cond()){
                                error_msg=services->kill_agent(msg.get_target_agent());
                                if(error_msg==NO_ERROR){ //To do: extra condition
                                   msg.set_msg_type(CONFIRM);
                                }
                                else msg.set_msg_type(REFUSE);
                            }
                            else msg.set_msg_type(REFUSE);
                            /*Respond*/
                            msg.send(msg.get_sender());
                            break;

                        case MODIFY:
                            if(cond->modify_cond()){
                                error_msg=services->modify_agent(msg.get_target_agent(), (Agent)msg.get_msg_string());
                                if(error_msg==1){ //To do: extra condition
                                    msg.set_msg_type(CONFIRM);
                                }
                                else msg.set_msg_type(REFUSE);
                            }
                            else msg.set_msg_type(REFUSE);
                            /*Respond*/
                            msg.send(msg.get_sender());
                            break;

                        case RESUME:
                            if (cond->resume_cond()){
                                error_msg=services->resume_agent(msg.get_target_agent());
                                if(error_msg==1){ //To do: extra condition
                                   msg.set_msg_type(CONFIRM);
                                }
                                else msg.set_msg_type(REFUSE);
                            }
                            else msg.set_msg_type(REFUSE);
                            /*Respond*/
                            msg.send(msg.get_sender());
                            break;

                        case SUSPEND:
                            if (cond->suspend_cond()){
                                error_msg=services->suspend_agent(msg.get_target_agent());
                                if(error_msg==1){ //To do: extra condition
                                   msg.set_msg_type(CONFIRM);
                                }
                                else msg.set_msg_type(REFUSE);
                            }
                            else msg.set_msg_type(REFUSE);
                            /*Respond*/
                            msg.send(msg.get_sender());
                            break;

                        case MODIFY_PRI:
                            if(cond->setpri_cond()){
                                error_msg=services->set_agent_pri(msg.get_target_agent(),(int)msg.get_msg_string());
                                if(error_msg==1){ //To do: extra condition
                                  msg.set_msg_type(CONFIRM);
                                }
                                else msg.set_msg_type(REFUSE);
                            }
                            else msg.set_msg_type(REFUSE);
                            /*Respond*/
                            msg.send(msg.get_sender());
                            break;

                        case BROADCAST:
                            if(cond->broadcast_cond()){
                                services->broadcast(msg.get_msg());
                                msg.set_msg_type(CONFIRM);
                            }
                            else msg.set_msg_type(REFUSE);
                            /*Respond*/
                            msg.send(msg.get_sender());
                            break;
                    }
                }//End if

                else{
                    msg.set_msg_type(NOT_UNDERSTOOD);
                    msg.send(msg.get_sender());
                }
            }//finish while
        }
    }

/*********************************************************************************************
*
*                                  Class: USER_DEF_COND
*
**********************************************************************************************
**********************************************************************************************
* Class: USER_DEF_COND
* Defined the default conditions
**********************************************************************************************/
    bool USER_DEF_COND::register_cond(){
        return true;
    }
    bool USER_DEF_COND::kill_cond(){
        return true;
    }
    bool USER_DEF_COND::deregister_cond(){
        return true;
    }
    bool USER_DEF_COND::suspend_cond(){
        return true;
    }
    bool USER_DEF_COND::resume_cond(){
        return true;
    }
    bool USER_DEF_COND::modify_cond(){
        return true;
    }
    bool USER_DEF_COND::setpri_cond(){
        return true;
    }
    bool USER_DEF_COND::broadcast_cond(){
        return true;
    }
/*********************************************************************************************
*
*                                  Class: Agent
*
**********************************************************************************************
**********************************************************************************************
* Class: Agent
* Function: Agent constructor
**********************************************************************************************/
    Agent_Struct::Agent_Struct(String name){
        description.agent_name=name;
    }
/*********************************************************************************************
 * Class: Agent
 * Function: bool create()
 * Return type: Boolean
 * Comments: Create the task and mailbox associated with the agent. The Agent's description
 *           struct is stored in task's environment variable.
 *           Priority set to -1, only set when the agent is registered
*********************************************************************************************/
    Agent Agent_Struct::create(Task_FuncPtr behaviour){

        Agent aid;
        Task_Params taskParams;
        Mailbox_Params mbxParams;

        description.AP=NULL;
        description.priority= 1;

        /*Creating mailbox: Msg size is 20 and default queue size is set to 3*/
        Mailbox_Params_init(&mbxParams);
        description.mailbox_handle= Mailbox_create(20,8,&mbxParams,NULL);

        /*Creating task*/
        Task_Params_init(&taskParams);
        taskParams.stack=task_stack;
        taskParams.stackSize =1024;
        taskParams.priority = -1;
        taskParams.instance->name=description.agent_name; //To do: take that out to optimize
        taskParams.env=&description;
        aid = Task_create(behaviour, &taskParams, NULL);
        return aid;

    }
/*********************************************************************************************
 * Class: Agent
 * Function: bool create(int priority)
 * Return type: Boolean
 * Comments: Create the task and mailbox associated with the agent. The Agent's description
 *           struct is stored in task's environment variable.
 *           Priority set to -1, only set when the agent is registered
*********************************************************************************************/
   Agent Agent_Struct::create(Task_FuncPtr behaviour,int priority){

        Agent aid;
        Task_Params taskParams;
        Mailbox_Params mbxParams;

        description.AP=NULL;
        description.priority= priority;

        /*Creating mailbox: Msg size is 20 and default queue size is set to 3*/
        Mailbox_Params_init(&mbxParams);
        description.mailbox_handle= Mailbox_create(20,8,&mbxParams,NULL);

        /*Creating task*/
        Task_Params_init(&taskParams);
        taskParams.stack=task_stack;
        taskParams.stackSize =1024;
        taskParams.priority = -1;
        taskParams.instance->name=description.agent_name; //To do: take that out to optimize
        taskParams.env=&description;
        aid = Task_create(behaviour, &taskParams, NULL);
        return aid;

    }
 /*********************************************************************************************
 * Class: Agent
 * Function: bool create(int taskstackSize,int queueSize, int priority)
 * Return type: bool
 * Comments: Create the task and mailbox associated with the agent. The Agent's description
 *           struct is stored in task's environment variable.
 *           This init is with user custom's task stack size, queue size and priority.
 *           Be aware of heap size
 *           Priority set to -1, only set when the agent is registered
*********************************************************************************************/
    Agent Agent_Struct::create(Task_FuncPtr behaviour,int taskstackSize, int queueSize, int priority){

        Agent aid;
        Task_Params taskParams;
        Mailbox_Params mbxParams;

        description.AP=NULL;
        description.priority=priority;

        /*Creating mailbox
        * Msg size is 20*/
        Mailbox_Params_init(&mbxParams);
        description.mailbox_handle=Mailbox_create(20,queueSize,&mbxParams,NULL);

        /*Creating task*/
        Task_Params_init(&taskParams);
        taskParams.stack=new char[taskstackSize];
        taskParams.stackSize =taskstackSize;
        taskParams.priority = -1;
        taskParams.instance->name=description.agent_name;
        taskParams.env=&description;
        aid = Task_create(behaviour, &taskParams, NULL);

        return aid;
    }
/*********************************************************************************************
* Class: Agent
* Function: bool create(int taskstackSize,int queueSize, int priority)
* Return type: bool
* Comments: Create the task and mailbox associated with the agent. The Agent's description
*           struct is stored in task's environment variable.
*           This init is with user custom arguments.
*           Priority set to -1, only set when the agent is registered
*********************************************************************************************/
    Agent Agent_Struct::create(Task_FuncPtr behaviour,UArg arg0, UArg arg1){

        Agent aid;
        Task_Params taskParams;
        Mailbox_Params mbxParams;

        description.AP=NULL;
        description.priority=1;

        /*Creating mailbox: Msg size is 20 and default queue size is set to 3*/
        Mailbox_Params_init(&mbxParams);
        description.mailbox_handle= Mailbox_create(20,8,&mbxParams,NULL);

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
        return aid;
    }

/*********************************************************************************************
* Class: Agent
* Function: bool create(int taskstackSize,int queueSize, int priority)
* Return type: bool
* Comments: Create the task and mailbox associated with the agent. The Agent's description
*           struct is stored in task's environment variable.
*           This init is with user custom arguments, task stack size, queue size and priority.
*           Be aware of heap size. Priority set to -1, only set when the agent is registered
*********************************************************************************************/
    Agent Agent_Struct::create(Task_FuncPtr behaviour,int taskstackSize, int queueSize, int priority, UArg arg0, UArg arg1){

        Agent aid;
        Task_Params taskParams;
        Mailbox_Params mbxParams;

        description.AP=NULL;
        description.priority=priority;

        /*Creating mailbox
        * Msg size is 20*/
        Mailbox_Params_init(&mbxParams);
        description.mailbox_handle=Mailbox_create(20,queueSize,&mbxParams,NULL);

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

        return aid;

    }
/*********************************************************************************************
*
*                                  Class: Agent_Platform
*
**********************************************************************************************
**********************************************************************************************
* Class: Agent_Platform
* Function: Agent_Platform Constructor
* Comment: Initialize list of agents task handle and its environment to NULL.
**********************************************************************************************/
    Agent_Platform::Agent_Platform(String name){
         AP_Description *ptr_AP=services.get_AP();
         ptr_AP->AMS_description.agent_name=name;
         ptr_cond=&cond;
    }
/*********************************************************************************************
* Class: Agent_Platform
* Function: Agent_Platform Constructor
* Comment: Initialize list of agents task handle and its environment to NULL.
*          This constructor is used if user has their own conditions
**********************************************************************************************/
    Agent_Platform::Agent_Platform(String name,USER_DEF_COND*user_cond){
         AP_Description *ptr_AP=services.get_AP();
         ptr_AP->AMS_description.agent_name=name;
         ptr_cond=user_cond;
    }
/*********************************************************************************************
* Class: Agent_Platform
* Function: bool init();
* Return: Boolean
* Comment: Create AMS task with default stack of 1024
***********************************************************************************************/
    bool Agent_Platform::init(){

        Agent temp;
        Mailbox_Params mbxParams;
        Task_Params taskParams;
        AP_Description *ptr_AP=services.get_AP();

        /*Assigning priority to AMS*/
        ptr_AP->AMS_description.priority=Task_numPriorities-1;

        /*Creating mailbox*/
        Mailbox_Params_init(&mbxParams);
        ptr_AP->AMS_description.mailbox_handle= Mailbox_create(20,8,&mbxParams,NULL);

        /*Creating task*/
        Task_Params_init(&taskParams);
        taskParams.stack=task_stack;
        taskParams.stackSize = 1024;
        taskParams.priority = Task_numPriorities-1;//Assigning max priority
        taskParams.instance->name=ptr_AP->AMS_description.agent_name;
        taskParams.env=&ptr_AP->AMS_description;
        taskParams.arg0=(UArg)&services;
        taskParams.arg1=(UArg)ptr_cond;
        ptr_AP->AMS_description.AP = Task_create(AMS_task, &taskParams, NULL);

        /*Initializing all the previously created task*/

        if (ptr_AP->AMS_description.AP!=NULL){
            temp=Task_Object_first();
            while (temp!=NULL && temp!=ptr_AP->AMS_description.AP){
                services.register_agent(temp);
                temp=Task_Object_next(temp);
            }
            return true;
        }
        else return false;
   }

/*********************************************************************************************
* Class: Agent_Platform
* Function: bool init(Task_FuncPtr action,int taskstackSize);;
* Comment: Create AMS task with user custom stack size
*          Be aware of heap size
**********************************************************************************************/
    bool Agent_Platform::init(int taskstackSize){

        Agent temp;
        Mailbox_Params mbxParams;
        Task_Params taskParams;
        AP_Description *ptr_AP=services.get_AP();

        /*Assigning priority to AMS*/
        ptr_AP->AMS_description.priority=Task_numPriorities-1;

        /*Creating mailbox*/
        Mailbox_Params_init(&mbxParams);
        ptr_AP->AMS_description.mailbox_handle= Mailbox_create(20,8,&mbxParams,NULL);

        /*Creating task*/
        Task_Params_init(&taskParams);
        taskParams.stack=new char[taskstackSize];
        taskParams.stackSize = taskstackSize;
        taskParams.priority = Task_numPriorities-1;//Assigning max priority
        taskParams.instance->name=ptr_AP->AMS_description.agent_name;
        taskParams.env=&ptr_AP->AMS_description;
        taskParams.arg0=(UArg)&services;
        taskParams.arg1=(UArg)ptr_cond;
        ptr_AP->AMS_description.AP= Task_create(AMS_task, &taskParams, NULL);

        /*Registering all created agents*/
        if (ptr_AP->AMS_description.AP!=NULL){
        /*Initializing all the previously created task*/
            temp=Task_Object_first();
            while (temp!=NULL && temp!=ptr_AP->AMS_description.AP){
                services.register_agent(temp);
                temp=Task_Object_next(temp);
            }

            return true;
        }
        else return false;
   }
/**********************************************************************************************
* Class: Agent_Platform
* Function:  bool search(Agent aid);
* Return: Bool
* Comment: search agent in Platform by agent aid
**********************************************************************************************/
    bool Agent_Platform::search(Agent aid){
        return services.search(aid);
    }
/*********************************************************************************************
* Class:Agent_Platform
* Function: void agent_wait (uint32 ticks)
* Return type: NULL
* Comments: When called within agent's function it will make agent sleeps defined ticks
*********************************************************************************************/
    void Agent_Platform::agent_wait(Uint32 ticks){
        Task_sleep(ticks);
    }
/*********************************************************************************************
* Class: Agent_Platform
* Function: void agent_yield()
* Return type: NULL
* Comments: It yields the processor to another readied agent of equal priority.
*           It lower priorities are on readied, the current task won't be preempted
*********************************************************************************************/
    void Agent_Platform::agent_yield(){
        Task_yield();
    }
/*********************************************************************************************
* Class:Agent_Platform
* Function: get_running_agent()
* Return type: NULL
* Comments: Returns aid of current running agent
*********************************************************************************************/
    Agent Agent_Platform::get_running_agent(){
        return Task_self();
    }
/*********************************************************************************************
* Class: Agent_Platform
* Function:void get_mode
* Return:  NULL
* Comment: get running mode of agent
**********************************************************************************************/
    int Agent_Platform:: get_mode(Agent aid){
        return services.get_mode(aid);
    }
/*********************************************************************************************
* Class: Agent_Platform
* Function: AP_Description* get_Agent_description(Agent aid);
* Return: Agent_info
* Comment: Returns description of an agent. Returns copy instead of pointer since pointer
*          can override the information.
**********************************************************************************************/
    const Agent_info *Agent_Platform::get_Agent_description(Agent aid){
        Agent_info *description;
        description=(Agent_info *)Task_getEnv(aid);
        return description;
    }
/*********************************************************************************************
* Class: Agent_Platform
* Function: AP_Description* get_AP_description();
* Return: AP_Description
* Comment: Returns description of the platform Returns copy instead of pointer since pointer
*          can override the information.
**********************************************************************************************/
    const AP_Description *Agent_Platform::get_AP_description(){
        return services.get_AP();
    }

/*********************************************************************************************
* Class: Agent_Platform
* Function: get_AMS_AID();
* Return: Task Handle of the AMS
* Comment: returns AMS task handle in case that exists
**********************************************************************************************/
    Agent Agent_Platform::get_AMS_Agent(){
        return services.get_AP()->AMS_description.AP;
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
* Function: private bool isRegistered(Agent aid);
* Return type: Boolean
* Comment: if returns false, sender or receiver is not registered in the same platform
**********************************************************************************************/
  bool Agent_Msg::isRegistered(Agent aid){
      Agent_info *description_receiver=(Agent_info *)Task_getEnv(aid);
      Agent_info *description_sender=(Agent_info *)Task_getEnv(self_handle);

      if(description_receiver->AP==description_sender->AP) return true;
      else return false;
}

/*********************************************************************************************
* Class: Agent_Msg
* Function: private get_mailbox(Agent aid)
* Return type: Mailbox_Handle
**********************************************************************************************/
    Mailbox_Handle Agent_Msg::get_mailbox(Agent aid){
        Agent_info * description;
        description = (Agent_info*) Task_getEnv(aid);
        return description->mailbox_handle;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: add_receiver(Agent aid)
* Return type: Boolean. True if receiver is added successfully.
* Comment: Add receiver to list of receivers by using the agent's aid
**********************************************************************************************/
   int Agent_Msg::add_receiver(Agent aid_receiver){

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
* Function: remove_receiver(Agent aid)
* Return type: Boolean. True if receiver is removed successfully. False if it is not encountered
* Comment: Remove receiver in list of receivers. It searches inside of the list, when found,
* the rest of the list is shifted to the right and the receiver is removed.
**********************************************************************************************/
   int Agent_Msg::remove_receiver(Agent aid){

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
* Function: refresh_list()
* Return type: NULL
* Comment: Refresh the list with all the registered agents. Remove agent if it is not registered
**********************************************************************************************/
    void Agent_Msg::refresh_list(){
        int i=0;
        while (i<next_available){
            if(!isRegistered(receivers[i]))remove_receiver(receivers[i]);
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
* Function: send(Agent aid)
* Return type: Boolean. TRUE if successful, FALSE if timeout
* Comment: Send msg to specific mailbox.
*          Set the MsgObj handle to sender's handle.
**********************************************************************************************/
    int Agent_Msg::send(Agent aid_receiver){
        msg.sender_agent=self_handle;
        msg.target_agent=aid_receiver;

        if(isRegistered(aid_receiver)){
            if(Mailbox_post(get_mailbox(aid_receiver), (xdc_Ptr)&msg, BIOS_NO_WAIT)) return NO_ERROR;
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
        msg.sender_agent=self_handle;

        while (receivers[i]!=NULL){
            if(isRegistered(receivers[i])){
                msg.target_agent=receivers[i];
                if(!Mailbox_post(get_mailbox(receivers[i]),(xdc_Ptr)&msg, BIOS_NO_WAIT));
                    no_error=false;
                i++;

            }
            else refresh_list();
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
* Function: set_msg_string(String content)
* Return type: NULL
* Comment: Set message body according to FIPA ACL
**********************************************************************************************/
    void Agent_Msg::set_msg_string(String msg_body){
        msg.content_string=msg_body;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: set_msg_int(int content)
* Return type: NULL
* Comment: Set message body according to FIPA ACL
**********************************************************************************************/
    void Agent_Msg::set_msg_int(int content){
        msg.content_int=content;
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
* Function: get_msg_string()
* Return type: String
* Comment: Get string content
**********************************************************************************************/
    String Agent_Msg::get_msg_string(){
        return msg.content_string;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: get_msg_int()
* Return type: int
* Comment: Get int content
**********************************************************************************************/
    int Agent_Msg::get_msg_int(){
        return msg.content_int;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: get_sender()
* Return type: Agent
* Comment: Get sender
**********************************************************************************************/
    Agent Agent_Msg::get_sender(){
        return msg.sender_agent;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: get_target_agent()
* Return type: Agent
* Comment: Get target aid
**********************************************************************************************/
    Agent Agent_Msg::get_target_agent(){
        return msg.target_agent;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: int request_AP(int request, Agent target_agent,int timeout)
* Return type: Int
* Comment: request the Agent Platform to perform a service and wait for response during
*          a time specified by the user
**********************************************************************************************/
    int Agent_Msg::request_AP(int request, Agent target_agent,int timeout){
        Agent AMS;
        Agent_info *temp;

        if (request != BROADCAST && request!=MODIFY && request!=MODIFY_PRI ){

            /*Setting msg*/
            msg.type=REQUEST;
            msg.content_int=request;
            msg.target_agent=target_agent;
            msg.sender_agent=Task_self();

            /*Getting AP address:
             * 1. Get the Agent info
             * 2. Get the AMS info*/
            temp= (Agent_info*) Task_getEnv(self_handle);
            AMS=temp->AP;

            /*Sending request*/
            Mailbox_post(get_mailbox(AMS), (xdc_Ptr)&msg, BIOS_NO_WAIT);

            /*Waiting for answer*/
            receive(timeout);

            return msg.type;
        }

        else return REFUSE;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: int request_AP(int request, Agent target_agent,int timeout,int content)
* Return type: Int
* Comment: request the agent to modify priority of target agent
**********************************************************************************************/
    int Agent_Msg::request_AP(int request, Agent target_agent,int timeout, Agent content){
        Agent AMS;
        Agent_info *temp;

        if(request==MODIFY){

            /*Setting msg*/
            msg.type=REQUEST;
            msg.content_int=request;
            msg.content_string=(String) content;
            msg.target_agent=target_agent;
            msg.sender_agent=Task_self();

            /*Getting AP address:
             * 1. Get the Agent info
             * 2. Get the AMS info*/
            temp= (Agent_info*) Task_getEnv(self_handle);
            AMS=temp->AP;

            /*Sending request*/
            Mailbox_post(get_mailbox(AMS), (xdc_Ptr)&msg, BIOS_NO_WAIT);

            /*Waiting for answer*/
            receive(timeout);

            return msg.type;
        }

        else return REFUSE;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: int request_AP(int request, Agent target_agent,int timeout,int content)
* Return type: Int
* Comment: request the agent to modify priority of target agent
**********************************************************************************************/
    int Agent_Msg::request_AP(int request, Agent target_agent,int timeout, int content){
        Agent AMS;
        Agent_info *temp;

        if(request==MODIFY_PRI){

            /*Setting msg*/
            msg.type=REQUEST;
            msg.content_int=request;
            msg.content_string=(String) content;
            msg.target_agent=target_agent;
            msg.sender_agent=Task_self();

            /*Getting AP address:
             * 1. Get the Agent info
             * 2. Get the AMS info*/
            temp= (Agent_info*) Task_getEnv(self_handle);
            AMS=temp->AP;

            /*Sending request*/
            Mailbox_post(get_mailbox(AMS), (xdc_Ptr)&msg, BIOS_NO_WAIT);

            /*Waiting for answer*/
            receive(timeout);

            return msg.type;
        }

        else return REFUSE;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: void Broadcast();
* Return type: Int
* Comment: request the agent to set
**********************************************************************************************/
    int Agent_Msg::broadcast(int timeout, String content){
        Agent AMS;
        Agent_info *temp;

        msg.type=REQUEST;
        msg.content_int=BROADCAST;
        msg.target_agent=NULL;
        msg.sender_agent=Task_self();
        msg.content_string=content;

        /*Getting AP address:
         * 1. Get the Agent info
         * 2. Get the AMS info*/
        temp= (Agent_info*) Task_getEnv(self_handle);
        AMS=temp->AP;

        /*Sending request*/
        Mailbox_post(get_mailbox(AMS), (xdc_Ptr)&msg, BIOS_NO_WAIT);

        /*Waiting for answer*/
        receive(timeout);
        return msg.type;
    }

};