//maes.h
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <string.h>
#include <xdc/runtime/Error.h>

namespace MAES
{
/*********************************************************************************************
 *                                       DEFINITIONS                                         *
*********************************************************************************************/
#define AGENT_LIST_SIZE 64
#define MAX_RECEIVERS   AGENT_LIST_SIZE-1
#define BEHAVIOUR_LIST_SIZE 8
#define ORGANIZATIONS_SIZE 16

/*********************************************************************************************
*   Define Agent Mode
**********************************************************************************************/
#define ACTIVE          0x00
#define SUSPENDED       0x01
#define WAITING         0x02
#define TERMINATED      0x03

/*********************************************************************************************
*   Define Error handling
**********************************************************************************************/
#define NO_ERROR        0x00
#define FOUND           0x01
#define HANDLE_NULL     0x02
#define LIST_FULL       0x03
#define DUPLICATED      0x04
#define NOT_FOUND       0x05
#define TIMEOUT         0x06
#define INVALID         0x07
#define NOT_REGISTERED  0x08

/*********************************************************************************************
*   Define msg type according to FIPA ACL Message Representation in Bit-Efficient Encoding
*   Specification
**********************************************************************************************/
#define ACCEPT_PROPOSAL  0x10
#define AGREE            0x11
#define CANCEL           0x12
#define CFP              0x13
#define CONFIRM          0x14
#define DISCONFIRM       0x15
#define FAILURE          0x16
#define INFORM           0x17
#define INFORM_IF        0x18
#define INFORM_REF       0x19
#define NOT_UNDERSTOOD   0x1A
#define PROPAGATE        0x1B
#define PROPOSE          0x1C
#define QUERY_IF         0x1D
#define QUERY_REF        0x1E
#define REFUSE           0x1F
#define REJECT_PROPOSAL  0x20
#define REQUEST          0x21
#define REQUEST_WHEN     0x22
#define REQUEST_WHENEVER 0x23
#define SUBSCRIBE        0x24
#define NO_RESPONSE      0x25
/*********************************************************************************************
*   Define Request Action
**********************************************************************************************/
#define REGISTER        0x30
#define DEREGISTER      0x31
#define KILL            0x32
#define RESUME          0x33
#define SUSPEND         0x34
#define MODIFY          0x35
#define BROADCAST       0x36
#define CREATE          0x37
#define RESTART         0x38

/*********************************************************************************************
*   Define Organization Affiliation/Role
**********************************************************************************************/
#define OWNER           0x40
#define ADMIN           0x41
#define MEMBER          0X42
#define OUTCAST         0x43
#define MODERATOR       0x44
#define PARTICIPANT     0x45
#define VISITOR         0x46
#define NONE            0x47
/*********************************************************************************************
*   Define Organization Type
**********************************************************************************************/
#define HIERARCHY       0x50
#define TEAM            0x51

/*********************************************************************************************
 *                                         TYPEDEF                                           *
**********************************************************************************************/
typedef Task_Handle Agent_AID;
typedef char        Agent_Stack;
typedef int         ERROR_CODE;
typedef int         MSG_TYPE;
/*********************************************************************************************
* Class: Agent_Organization
* Comment:
* Variables
**********************************************************************************************/
    typedef struct{
        int org_type;
        int members_num;
        int banned_num;
        Agent_AID members[AGENT_LIST_SIZE];
        Agent_AID banned[AGENT_LIST_SIZE];
        Agent_AID owner;
        Agent_AID admin;
        Agent_AID moderator;
    }org_info;

/*********************************************************************************************
* Class: MsgObj
* Variables: Agent sender_agent: Agent sending the message
*            Agent target_agent: Destination agent
*            type: Type of message
*            String content_string: Content of the message in string format
*            int content_int: Content of the message in int format
**********************************************************************************************/
    typedef struct{
          Agent_AID sender_agent;
          Agent_AID target_agent;
          int type;
          String content_string;
          int content_int;
    }MsgObj;
/*********************************************************************************************
* Class: Agent_info
* Variables: Agent_AID aid: agent's AID
*            Mailbox_Handle mailbox_handle: communication method
*            String agent_name: agent's name
*            int priority: Priority of the agent
*            Agent_AID AP: AMS AID who manage the agent.
*            org_inf *org: Organization which belongs the agent
*            int affiliation: affiliation with the org
*            int role: role withing organization
***********************************************************************************************/
    typedef struct{
        Agent_AID aid;
        Mailbox_Handle mailbox_handle;
        String agent_name;
        int priority;
        Agent_AID AP;
        org_info *org;
        int affiliation;
        int role;
    }Agent_info;

/*********************************************************************************************
* Class: Agent_resources
* Variables: Agent_Stack *stack: pointer to stack
*            int size: Agent Size
***********************************************************************************************/
    typedef struct{
        char *stack;
        int stackSize;
    }Agent_resources;

/*********************************************************************************************
 *                                         CLASSES                                           *
**********************************************************************************************
**********************************************************************************************
* Class: USER_DEF_COND
* Comment: Class to be overriden for user's own conditions for AMS.
**********************************************************************************************/
    class USER_DEF_COND{
    public:
        virtual bool register_cond();
        virtual bool kill_cond();
        virtual bool deregister_cond();
        virtual bool suspend_cond();
        virtual bool resume_cond();
        virtual bool modify_cond();
        virtual bool broadcast_cond();
        virtual bool restart();
        virtual bool create();
    };

/*********************************************************************************************
* Class: Agent
* Variables: Agent_info description;
*            char *task_stack: pointer to the stack
*            int stackSize: task stack size
*
* Comment: Agent construction class.
*          Add lines in cfg file to use Mailbox module:
*          var Mailbox = xdc.useModule('ti.sysbios.knl.Mailbox')
**********************************************************************************************/
    class Agent{
    public:
        friend class Agent_Platform;
        friend class Agent_Msg;
        friend class Agent_Organization;
        /*Constructor*/
        Agent(String name, int pri, char *AgentStack,int sizeStack);

    private:
        Agent();
        Agent_resources resources;
        Agent_info agent;
    };

/*********************************************************************************************
 *                           Class and functions in Unnamed namespace for AP                 *
**********************************************************************************************/
namespace{
        void AMS_task(UArg arg0,UArg arg1);  //AMS_Task
}
/*********************************************************************************************
* Class: AP_Description
* Variables: Agent_AID AMS_AID: Task handle of AMS
*            String AP_name: Platform name
*            int subscribers: members in the platform
***********************************************************************************************/
    typedef struct{
        Agent_AID AMS_AID;
        String AP_name;
        int subscribers;
    }AP_Description;
/*********************************************************************************************
* Class:   Agent_Platform
* Comment: API for Agent Management Services
* Variables: USER_DEF_COND cond: contains the default conditions for AMS private services.
*            USER_DEF_cond ptr_cond: pointer to the USER_DEF_COND where contain the user
*                                    defined functions.
*            char task_stack[2048]: default size if additional services of AMS is required
*            Agent_AID Agent_Handle[AGENT_LIST_SIZE]: members subscribed
*            AP_description platform: Information about the platform.
**********************************************************************************************/
    class Agent_Platform{
    public:
        /*Constructor*/
        Agent_Platform(String name);
        Agent_Platform(String name,USER_DEF_COND*user_cond);

        /*Methods*/
        bool boot();

        /*Only called from Main*/
        void agent_init(Agent &a, Task_FuncPtr behaviour, Agent_AID &aid);
        void agent_init(Agent &a, Task_FuncPtr behaviour, UArg arg0, UArg arg1,Agent_AID &aid);

        /*Public Services available for all agents*/
        bool agent_search(Agent_AID aid);
        void agent_wait(Uint32 ticks);
        void agent_yield();
        Agent_AID get_running_agent();
        int get_state(Agent_AID aid);
        Agent_info get_Agent_description(Agent_AID aid);
        AP_Description get_AP_description();// to do

        /*Services only can be done by AMS task*/
        ERROR_CODE register_agent(Agent_AID aid);
        ERROR_CODE deregister_agent(Agent_AID aid);
        ERROR_CODE kill_agent(Agent_AID aid);
        ERROR_CODE suspend_agent(Agent_AID aid);
        ERROR_CODE resume_agent(Agent_AID aid);
        ERROR_CODE modify_agent_pri(Agent_AID aid,int pri);
        void broadcast(MsgObj *msg);
        void restart(Agent_AID aid);

    private:
        /*Class variables*/
        Agent agentAMS;
        char task_stack[4096];
        Agent_AID Agent_Handle[AGENT_LIST_SIZE];
        int subscribers;
        USER_DEF_COND cond;
        USER_DEF_COND *ptr_cond;

    };

/*********************************************************************************************
* Class: Agent_Msg
* Comment:   Predefined class for msg object.
* Variables: MsgObj msg: To be used to receive and send
*            Agent receivers[MAX_Receivers]: list of receivers
*            int subscribers: index of the receiver list where denotes the spot available
*                                in the list
*            Agent caller: Contains info about the calling agent.
*            bool isRegistered: Private method to check if Agent is registered
*            Mailbox_Handle: Private method to get Mailbox from Agent_AID
***********************************************************************************************/
    class Agent_Msg {
    public:
        /*Constructor*/
        Agent_Msg();
        //friend class AMS_Services;
        /*Methods*/
        ERROR_CODE add_receiver(Agent_AID aid_receiver);
        ERROR_CODE remove_receiver(Agent_AID aid_receiver);
        void clear_all_receiver();
        void refresh_list();
        MSG_TYPE receive(Uint32 timeout);
        ERROR_CODE send(Agent_AID aid_receiver,int timeout);
        ERROR_CODE send();
        void set_msg_type(int type);
        void set_msg_string(String body);
        void set_msg_int(int content);
        MsgObj *get_msg();
        int get_msg_type();
        String get_msg_string();
        int get_msg_int();
        Agent_AID get_sender();
        Agent_AID get_target_agent();
        ERROR_CODE request_AP(int request, Agent_AID target_agent);
        ERROR_CODE modify_pri(Agent_AID target_agent, int pri);
        ERROR_CODE kill(Agent_AID &target_agent);
        ERROR_CODE broadcast(String content);
        ERROR_CODE restart();

    private:
        MsgObj msg;
        Agent_AID receivers[MAX_RECEIVERS];
        int subscribers;
        Agent_AID caller;
        bool isRegistered(Agent_AID aid);
        Mailbox_Handle get_mailbox(Agent_AID aid);
    };
/*********************************************************************************************
* Class: Agent_Organization
* Comment:   Predefined struct for agent organization
* Variables: org_info description: description of the organization
*            Agent_AID banned[AGENT_LIST_SIZE]: list of banned agents
*            int banned_num: number of banned agents
*            bool isRegistered: private method to check if agent is registered
**********************************************************************************************/
    class Agent_Organization{
    public:
        Agent_Organization(int organization_type);
        ERROR_CODE create();
        ERROR_CODE destroy();
        ERROR_CODE isMember(Agent_AID aid);
        ERROR_CODE isBanned(Agent_AID aid);
        ERROR_CODE change_owner(Agent_AID aid);
        ERROR_CODE set_admin(Agent_AID aid);
        ERROR_CODE set_moderator(Agent_AID aid);
        ERROR_CODE add_agent(Agent_AID aid);
        ERROR_CODE kick_agent(Agent_AID aid);
        ERROR_CODE ban_agent(Agent_AID aid);
        ERROR_CODE remove_ban(Agent_AID aid);
        void clear_ban_list();
        ERROR_CODE set_participant(Agent_AID aid);
        ERROR_CODE set_visitor(Agent_AID aid);
        int get_org_type();
        org_info get_info();
        int get_size();
        ERROR_CODE invite(Agent_Msg msg, int password, Agent_AID target_agent, int timeout);

    private:
        org_info description;
        bool isRegistered(Agent_AID aid);
    };
/*********************************************************************************************
 *                               BEHAVIOUR CLASSES                                           *
**********************************************************************************************/
    class Generic_Behaviour{
    public:
        Generic_Behaviour();
        Agent_Msg msg;
        virtual void action()=0;
        virtual bool done();
        virtual bool failure_detection();
        virtual void failure_identification();
        virtual void failure_recovery();
        virtual void setup();
        void execute();
    };

    class OneShotBehaviour:public Generic_Behaviour{
    public:
        OneShotBehaviour();
        virtual void action()=0;
        virtual bool done();
    };

    class CyclicBehaviour:public Generic_Behaviour{
    public:
        CyclicBehaviour();
        virtual void action()=0;
        virtual bool done();
    };

}
