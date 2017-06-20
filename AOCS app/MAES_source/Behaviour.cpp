#include "maes.h"

namespace MAES{
/*********************************************************************************************
* Class: Generic_Behaviour
* Comment: Constructors and its derived specific class for user implementation
**********************************************************************************************/
    Generic_Behaviour::Generic_Behaviour(){}
    void Generic_Behaviour::setup(){}
    void Generic_Behaviour::execute(){
        setup();
        do{
            action();
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
