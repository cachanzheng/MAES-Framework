#include "maes.h"
namespace MAES
{
/*********************************************************************************************
* Class: Agent_Organization
* Function: Organization constructor
* Return type: NULL
* Comments: Creates the organization with specific type
*********************************************************************************************/
    Agent_Organization::Agent_Organization(ORG_TYPE type){
        description.org_type=type;
        description.members_num=0;
        description.banned_num=0;

        for (int i=0; i<AGENT_LIST_SIZE;i++){
            description.members[i]=NULL;
            description.banned[i]=NULL;
        }

    }

/*********************************************************************************************
* Class: Agent_Organization
* Function: bool isMember(Agent_AID aid)
* Return type: int
* Comments:  Private member, search member within the list.
*********************************************************************************************/
    ERROR_CODE Agent_Organization::isMember(Agent_AID aid){
        for(int i=0;i<description.members_num;i++){
          if (description.members[i]==aid) return FOUND;
        }
        return NOT_FOUND;
    }
/*********************************************************************************************
* Class: Agent_Organization
* Function: bool isMember(Agent_AID aid)
* Return type: int
* Comments:  Private member, search member within the list.
*********************************************************************************************/
    ERROR_CODE Agent_Organization::isBanned(Agent_AID aid){
        for(int i=0;i<description.banned_num;i++){
            if (description.banned[i]==aid) return FOUND;

        }
        return NOT_FOUND;
    }

/*********************************************************************************************
* Class: Agent_Organization
* Function: bool create()
* Return type: Boolean
* Comments: Create the organization. Should be called from an Agent's behaviour. Owner is
*           stored in first of the member list.
*           When function is called within agent's behaviour, the ownership of the organization
*           is assigned. If it already has owner or it is not registered to any platform, returns false.
*********************************************************************************************/
    ERROR_CODE Agent_Organization::create(){
        if (Task_self()==NULL) return INVALID;
        else if(description.owner==NULL){
            Agent *agent=(Agent *)Task_getEnv(Task_self());

            if(agent->agent.AP!=NULL) {
                agent->agent.role=PARTICIPANT;
                agent->agent.affiliation=OWNER;
                agent->agent.org=&description;
                description.owner=Task_self();
                description.members[description.members_num]=description.owner;
                description.members_num++;
                return NO_ERROR;
            }
            else return NOT_REGISTERED;
        }
        else return INVALID;
    }

/*********************************************************************************************
* Class: Agent_Organization
* Function: add_agent(Agent_AID aid)
* Return type: int
* Comments: adds agent as member to the list. Only can be done by owner or admin.
*           Returns error if: List is full, agent is already in another org, it's not
*           registered in the platform, it's duplicated, it's description.banned
*           or this method is performed other than
*           owner or admin.
*********************************************************************************************/
    ERROR_CODE Agent_Organization::add_agent(Agent_AID aid){
        Agent *agent=(Agent *)Task_getEnv(aid);

        if (description.members_num==AGENT_LIST_SIZE) return LIST_FULL;
        else if (agent->agent.org!=NULL || isBanned(aid)==FOUND || agent->agent.AP==NULL) return INVALID;
        else if (isMember(aid)==FOUND) return DUPLICATED;
        else if (description.owner==Task_self() || description.admin==Task_self()){

            description.members[description.members_num]=aid;
            description.members_num++;
            agent->agent.affiliation=MEMBER;
            agent->agent.role=PARTICIPANT;
            agent->agent.org=&description;
            return NO_ERROR;
        }

        else return INVALID;
    }

/*********************************************************************************************
* Class: Agent_Organization
* Function: kick_agent(Agent_AID aid)
* Return type: int
* Comments: adds agent as member to the list. Only can be done by owner or admin. Cannot
*           autokick.
*********************************************************************************************/
    ERROR_CODE Agent_Organization::kick_agent(Agent_AID aid){
        if (aid == Task_self()) return INVALID;
        else if (description.owner==Task_self() || description.admin==Task_self()){
            int i=0;
            Agent *agent=(Agent *)Task_getEnv(aid);
            while(i<AGENT_LIST_SIZE){
                if(description.members[i]==aid){
                    while(i<AGENT_LIST_SIZE-1){
                        description.members[i]=description.members[i+1];
                        i++;
                    }
                    description.members[AGENT_LIST_SIZE-1]=NULL;
                    description.members_num--;
                    agent->agent.role=NONE;
                    agent->agent.affiliation=NON_MEMBER;
                    agent->agent.org=NULL;

                    System_printf("test123 %x\n", agent->agent.AP);
                    System_flush();
                    break;
                }
                i++;
            }
            if (i==AGENT_LIST_SIZE) return NOT_FOUND;
            else return NO_ERROR;
        }

        else return INVALID;
    }
/*********************************************************************************************
* Class: Agent_Organization
* Function: bool destroy()
* Return type: NULL
* Comments: Set all agent's org pointer to null and destroy organization. Only can be
*            called by owner. Returns false if this is not called by owner.
*********************************************************************************************/
    ERROR_CODE  Agent_Organization::destroy(){
        if (description.owner==Task_self()){
            Agent * agent;
            for(int i=0;i<description.members_num;i++){
                agent = (Agent*) Task_getEnv(description.members[i]);
                agent->agent.org=NULL;
                agent->agent.affiliation=NON_MEMBER;
                agent->agent.role=NONE;
                description.members[i]=NULL;

            }
            for(int i=0; i<description.banned_num;i++){
                 description.banned[i]=NULL;
            }
            return NO_ERROR;
        }

        else return INVALID;
    }

/*********************************************************************************************
* Class: Agent_Organization
* Function: bool change_owner(Agent_AID aid)
* Return type: Boolean
* Comments: Change organization owner. Only can be done by the owner and if the new owner is
*           registered as member in the organization.Returns false if it is not called by owner or the target agent is not member
*********************************************************************************************/
    ERROR_CODE Agent_Organization::change_owner(Agent_AID aid){
        if (description.owner==Task_self() && isMember(aid)==FOUND){
            Agent * agent= (Agent*) Task_getEnv(description.owner);
            agent->agent.affiliation=MEMBER;
            agent->agent.role=VISITOR;
            agent= (Agent*) Task_getEnv(aid);
            agent->agent.affiliation=OWNER;
            agent->agent.role=PARTICIPANT;
            description.owner=aid;
            return NO_ERROR;
        }
        else return INVALID;
    }

/*********************************************************************************************
* Class: Agent_Organization
* Function: set_admin(Agent_AID aid)
* Return type: Boolean
* Comments: Set admin of the organization. Only can be done by owner
*********************************************************************************************/
    ERROR_CODE Agent_Organization::set_admin(Agent_AID aid){
        if (description.owner==Task_self() && isMember(aid)==FOUND){
            Agent * agent= (Agent*) Task_getEnv(aid);
            agent->agent.affiliation=ADMIN;
            agent->agent.role=VISITOR;
            description.admin=aid;
            return NO_ERROR;
        }
        else return INVALID;
    }
/*********************************************************************************************
* Class: Agent_Organization
* Function: set_moderator(Agent_AID aid)
* Return type: Boolean
* Comments: Set moderator of the organization. Only can be done by owner
*********************************************************************************************/
    ERROR_CODE Agent_Organization::set_moderator(Agent_AID aid){
        if (description.owner==Task_self() && isMember(aid)==FOUND){
            Agent * agent= (Agent*) Task_getEnv(aid);
            agent->agent.affiliation=MEMBER;
            agent->agent.role=MODERATOR;
            description.moderator=aid;
            return NO_ERROR;
        }
        else return INVALID;
    }
/*********************************************************************************************
* Class: Agent_Organization
* Function: bool ban_agent(Agent_AID aid)
* Return type: Boolean
* Comments: Looks if the aid is already description.banned. If not put it to the list.
*           Only can be done by owner and moderator.
*********************************************************************************************/
    ERROR_CODE Agent_Organization::ban_agent(Agent_AID aid){
        if (description.banned_num==AGENT_LIST_SIZE) return LIST_FULL;
        else if (isBanned(aid)==FOUND)  return DUPLICATED;
        else if (description.owner==Task_self() || description.admin==Task_self()){
            description.banned[description.banned_num]=aid;
            description.banned_num++;
            if(isMember(aid)==FOUND) kick_agent(aid);
            return NO_ERROR;
        }

        else return INVALID;
    }
/*********************************************************************************************
* Class: Agent_Organization
* Function: ERROR_CODE remove_ban(Agent_AID aid)
* Return type: Boolean
* Comments: Looks if the aid is already description.banned. If not put it to the list.
*           only can be done by owner and moderator.
*********************************************************************************************/
    ERROR_CODE Agent_Organization::remove_ban(Agent_AID aid){
        if (description.owner==Task_self() || description.admin==Task_self()){
            int i=0;
            while(i<AGENT_LIST_SIZE){
                if(description.banned[i]==aid){
                    while(i<AGENT_LIST_SIZE-1){
                       description.banned[i]=description.banned[i+1];
                       i++;
                    }
                    description.banned[AGENT_LIST_SIZE-1]=NULL;
                    description.banned_num--;
                    break;
                }
                i++;
            }

            if (i==AGENT_LIST_SIZE) return NOT_FOUND;
            else return NO_ERROR;
        }
        else return INVALID;
    }
/*********************************************************************************************
* Class: Agent_Organization
* Function: void clear_ban_list()
* Return type: Boolean
* Comments: Clears all the description.banned list
*           only can be done by owner and moderator.
*********************************************************************************************/
    void Agent_Organization::clear_ban_list(){
        if (description.owner==Task_self() || description.admin==Task_self()){
            for(int i=0;i<AGENT_LIST_SIZE;i++){
               description.banned[i]=NULL;
            }
        }
    }
/*********************************************************************************************
* Class: Agent_Organization
* Function: void set_participant(Agent_AID aid)
* Return type: NULL
* Comments: Gives voice to an agent
*********************************************************************************************/
    ERROR_CODE Agent_Organization::set_participant(Agent_AID aid){
        if ((description.owner==Task_self() || description.moderator==Task_self()) && isMember(aid)==FOUND){
            Agent *agent=(Agent *)Task_getEnv(aid);
            agent->agent.role=PARTICIPANT;
            return NO_ERROR;
        }
        else return INVALID;
    }
/*********************************************************************************************
* Class: Agent_Organization
* Function: void set_visitor(Agent_AID aid)
* Return type: NULL
* Comments: Agent only can hear but not talk
*********************************************************************************************/
    ERROR_CODE Agent_Organization::set_visitor(Agent_AID aid){
        if ((description.owner==Task_self() || description.moderator==Task_self()) && isMember(aid)==FOUND){
            Agent *agent=(Agent *)Task_getEnv(aid);
            agent->agent.role=VISITOR;
            return NO_ERROR;
        }
        else return INVALID;
    }
/*********************************************************************************************
* Class: Agent_Organization
* Function: get_size
* Return type: int
* Comments: How many agents are inside the organization
*********************************************************************************************/
    int Agent_Organization::get_size(){
       return description.members_num;
    }


/*********************************************************************************************
* Class: Agent_Organization
* Function: get_info
* Return type: const org_info*
* Comments: get organization info
*********************************************************************************************/
    org_info Agent_Organization::get_info(){
       return description;
    }
/*********************************************************************************************
* Class: Agent_Organization
* Function: get_org_type()
* Return type: int
* Comments: Get Organization type
*********************************************************************************************/
    ORG_TYPE Agent_Organization::get_org_type(){
        return description.org_type;
    }
/*********************************************************************************************
* Class: Agent_Organization
* Function: invite(Agent_Msg msg, int password, Agent_AID target_agent, int timeout)
* Return type: int
* Comments: Invite specific agent and wait response for certain time.
*********************************************************************************************/
    MSG_TYPE Agent_Organization::invite(Agent_Msg msg, int password, Agent_AID target_agent, int timeout){
        Agent *caller=(Agent *)Task_getEnv(Task_self());

        if (caller->agent.affiliation==OWNER || caller->agent.affiliation==ADMIN){
            msg.set_msg_type(PROPOSE);
            msg.set_msg_string("Join Organization");
            msg.set_msg_int(password);
            msg.send(target_agent,timeout);

            msg.receive(timeout);

            if (msg.get_msg_type()==ACCEPT_PROPOSAL) add_agent (target_agent);
        }

        else msg.set_msg_type(REFUSE);
        return msg.get_msg_type();

    }
}


