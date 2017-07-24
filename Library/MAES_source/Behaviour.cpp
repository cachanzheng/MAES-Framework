#include "maes.h"

namespace MAES{
/*********************************************************************************************
* Class: Generic_Behaviour
* Comment: Constructors and its derived specific class for user implementation
**********************************************************************************************/
    Generic_Behaviour::Generic_Behaviour(){}
    void Generic_Behaviour::setup(){}
    void Generic_Behaviour::failure_identification(){}
    void Generic_Behaviour::failure_recovery(){}
    bool Generic_Behaviour::done(){
        return true;
    }
    bool Generic_Behaviour::failure_detection(){
        return false;
    }
    void Generic_Behaviour::execute(){
        setup();
        do{
            action();
           // failure_detection();
            if(failure_detection()){
                failure_identification();
                failure_recovery();
            }
        }while (!done());
    }

    OneShotBehaviour::OneShotBehaviour(){}
    bool OneShotBehaviour::done(){
        return true;
    }

    CyclicBehaviour::CyclicBehaviour(){}
    bool CyclicBehaviour::done(){
        return false;
    }
}
