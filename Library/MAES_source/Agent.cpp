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
    Agent::Agent(String name, int pri, char *AgentStack, int sizeStack){
        if (agent.priority>=(int)(Task_numPriorities-1)) agent.priority=Task_numPriorities-2;
            agent.aid=NULL;
            agent.mailbox_handle=NULL;
            agent.agent_name=name;
            agent.priority = pri;
            resources.stack=AgentStack;
            resources.stackSize=sizeStack;
            agent.AP=NULL;
            agent.org=NULL;
            agent.affiliation=NONE;
            agent.role=NONE;
    }

    Agent::Agent(){

    }
}
