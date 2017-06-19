#include "maes.h"
namespace MAES{
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
    bool USER_DEF_COND::broadcast_cond(){
        return true;
    }

}
