#ifndef ELECTRICUSOURCE_HPP_INCLUDED
#define ELECTRICUSOURCE_HPP_INCLUDED

#include <iostream>
//#include <Qt/qdebug.h>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include <math.h>
#include "matrix.h"

//!
//! @file ElectricUsource.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Sun 12 Jun 2011 13:16:14
//! @brief An inertia load with spring and damper
//! @ingroup ElectricComponents
//!
//This component is generated by COMPGEN for HOPSAN-NG simulation 
//from 
/*{, C:, Documents and Settings, petkr14, My Documents, \
CompgenNG}/ElectricNG2.nb*/

using namespace hopsan;

class ElectricUsource : public ComponentC
{
private:
     Port *mpPel1;
     Port *mpPuin;
     int mNstep;
     //Port Pel1 variable
     double uel1;
     double iel1;
     double cel1;
     double Zcel1;
     //Port Puin variable
     double uin;
     //Port Pel1 pointer
     double *mpND_uel1;
     double *mpND_iel1;
     double *mpND_cel1;
     double *mpND_Zcel1;
     //Port Puin pointer
     double *mpND_uin;
     //Delay declarations

public:
     static Component *Creator()
     {
        std::cout << "running ElectricUsource creator" << std::endl;
        return new ElectricUsource("Usource");
     }

     ElectricUsource(const std::string name = "Usource"
                             )
        : ComponentC(name)
     {
        mNstep=9;

        //Add ports to the component
        mpPel1=addPowerPort("Pel1","NodeElectric");
        mpPuin=addReadPort("Puin","NodeSignal");

        //Register changable parameters to the HOPSAN++ core
     }

    void initialize()
     {
        //Read variable pointers from nodes
        //Port Pel1
        mpND_uel1=getSafeNodeDataPtr(mpPel1, NodeElectric::VOLTAGE);
        mpND_iel1=getSafeNodeDataPtr(mpPel1, NodeElectric::CURRENT);
        mpND_cel1=getSafeNodeDataPtr(mpPel1, NodeElectric::WAVEVARIABLE);
        mpND_Zcel1=getSafeNodeDataPtr(mpPel1, NodeElectric::CHARIMP);
        //Port Puin
        mpND_uin=getSafeNodeDataPtr(mpPuin, NodeSignal::VALUE);
        //Read variables from nodes
        //Port Pel1
        uel1 = (*mpND_uel1);
        iel1 = (*mpND_iel1);
        cel1 = (*mpND_cel1);
        Zcel1 = (*mpND_Zcel1);
        //Port Puin
        uin = (*mpND_uin);



        //Initialize delays
     }
    void simulateOneTimestep()
     {
        //Read variables from nodes
        //Port Pel1
        uel1 = (*mpND_uel1);
        iel1 = (*mpND_iel1);
        cel1 = (*mpND_cel1);
        Zcel1 = (*mpND_Zcel1);
        //Port Puin
        uin = (*mpND_uin);

        //LocalExpressions

        //Expressions
        double cel1 = uin;
        double Zcel1 = 0.;

        //Write new values to nodes
        //Port Pel1
        (*mpND_cel1)=cel1;
        (*mpND_Zcel1)=Zcel1;
        //Port Puin

        //Update the delayed variabels

     }
};
#endif // ELECTRICUSOURCE_HPP_INCLUDED
