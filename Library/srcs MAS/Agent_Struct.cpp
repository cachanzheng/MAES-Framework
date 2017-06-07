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

}
