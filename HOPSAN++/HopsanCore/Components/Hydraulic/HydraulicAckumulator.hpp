#ifndef HYDRAULICACKUMULATOR_HPP_INCLUDED
#define HYDRAULICACKUMULATOR_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include <math.h>

//!
//! @file HydraulicAckumulator.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Mon 12 Sep 2011 22:31:30
//! @brief Hydraulic lossless ackumulator
//! @ingroup HydraulicComponents
//!
//This component is generated by COMPGEN for HOPSAN-NG simulation 
//from 
/*{, C:, Documents and Settings, petkr14, My Documents, \
CompgenNG}/HydraulicComponentsNG.nb*/

using namespace hopsan;

class HydraulicAckumulator : public ComponentQ
{
private:
     double mp0;
     double mV0;
     double mKca;
     double mkappa;
     Port *mpP1;
     Port *mpPVa;
     Port *mpPpa;
     double delayParts1[9];
     double delayParts2[9];
     double delayParts3[9];
     double delayParts4[9];
     Matrix jacobianMatrix;
     Vec systemEquations;
     Matrix delayedPart;
     int i;
     int iter;
     int mNoiter;
     int jsyseqnweight[4];
     int order[4];
     int mNstep;
     //Port P1 variable
     double p1;
     double q1;
     double c1;
     double Zc1;
     //inputVariables
     //outputVariables
     double Va;
     double pa;
     //Port P1 pointer
     double *mpND_p1;
     double *mpND_q1;
     double *mpND_c1;
     double *mpND_Zc1;
     //Delay declarations
     //inputVariables pointers
     //outputVariables pointers
     double *mpND_Va;
     double *mpND_pa;
     Delay mDelayedPart10;
     Delay mDelayedPart20;
     Delay mDelayedPart21;
     Delay mDelayedPart30;

public:
     static Component *Creator()
     {
        return new HydraulicAckumulator();
     }

     HydraulicAckumulator(const std::string name = "Ackumulator"
                             ,const double p0 = 1.e7
                             ,const double V0 = 0.001
                             ,const double Kca = 1.e-8
                             ,const double kappa = 1.2
                             )
        : ComponentQ(name)
     {
        mNstep=9;
        jacobianMatrix.create(4,4);
        systemEquations.create(4);
        delayedPart.create(5,6);
        mNoiter=2;
        jsyseqnweight[0]=1;
        jsyseqnweight[1]=0.67;
        jsyseqnweight[2]=0.5;
        jsyseqnweight[3]=0.5;

        mp0 = p0;
        mV0 = V0;
        mKca = Kca;
        mkappa = kappa;

        //Add ports to the component
        mpP1=addPowerPort("P1","NodeHydraulic");

        //Add inputVariables ports to the component

        //Add outputVariables ports to the component
        mpPVa=addWritePort("PVa","NodeSignal", Port::NOTREQUIRED);
        mpPpa=addWritePort("Ppa","NodeSignal", Port::NOTREQUIRED);

        //Register changable parameters to the HOPSAN++ core
        registerParameter("p0", "Preload pressure", "N/m^2", mp0);
        registerParameter("V0", "Ack. Volume", "m^3", mV0);
        registerParameter("Kca", "Flow coefficent", "m^3/Pa", mKca);
        registerParameter("kappa", "polytropic exp. of gas", "", mkappa);
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port P1
        mpND_p1=getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
        mpND_q1=getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
        mpND_c1=getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
        mpND_Zc1=getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);
        //Read inputVariables pointers from nodes
        //Read outputVariable pointers from nodes
        mpND_Va=getSafeNodeDataPtr(mpPVa, NodeSignal::VALUE);
        mpND_pa=getSafeNodeDataPtr(mpPpa, NodeSignal::VALUE);

        //Read variables from nodes
        //Port P1
        p1 = (*mpND_p1);
        q1 = (*mpND_q1);
        c1 = (*mpND_c1);
        Zc1 = (*mpND_Zc1);

        //Read inputVariables from nodes

        //Read outputVariables from nodes
        Va = mpPVa->getStartValue(NodeSignal::VALUE);
        pa = mpPpa->getStartValue(NodeSignal::VALUE);



        //Initialize delays
        delayParts2[1] = (-2*Va - mKca*mTimestep*p1*limit(onPositive(p1 - pa) \
+ onPositive(Va),0.,1.) + mKca*mTimestep*pa*limit(onPositive(p1 - pa) + \
onPositive(Va),0.,1.))/2.;
        mDelayedPart21.initialize(mNstep,delayParts2[1]);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
     }
    void simulateOneTimestep()
     {
        Vec stateVar(4);
        Vec stateVark(4);
        Vec deltaStateVar(4);

        //Read variables from nodes
        //Port P1
        c1 = (*mpND_c1);
        Zc1 = (*mpND_Zc1);

        //Read inputVariables from nodes

        //LocalExpressions

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = q1;
        stateVark[1] = Va;
        stateVark[2] = pa;
        stateVark[3] = p1;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //Ackumulator
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =q1 + mKca*(p1 - pa)*limit(onPositive(p1 - pa) + \
onPositive(Va),0.,1.);
          systemEquations[1] =Va - limit(-(mKca*mTimestep*(-p1 + \
pa)*limit(onPositive(p1 - pa) + onPositive(Va),0.,1.))/2. - \
delayedPart[2][1],0.,mV0);
          systemEquations[2] =pa - (mp0*Power(mV0,mkappa))/Power(mV0 - \
Va,mkappa);
          systemEquations[3] =p1 - (c1 + q1*Zc1)*onPositive(p1);

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;
          jacobianMatrix[0][1] = 0;
          jacobianMatrix[0][2] = -(mKca*limit(onPositive(p1 - pa) + \
onPositive(Va),0.,1.));
          jacobianMatrix[0][3] = mKca*limit(onPositive(p1 - pa) + \
onPositive(Va),0.,1.);
          jacobianMatrix[1][0] = 0;
          jacobianMatrix[1][1] = 1;
          jacobianMatrix[1][2] = \
(mKca*mTimestep*dxLimit(-(mKca*mTimestep*(-p1 + pa)*limit(onPositive(p1 - pa) \
+ onPositive(Va),0.,1.))/2. - delayedPart[2][1],0.,mV0)*limit(onPositive(p1 - \
pa) + onPositive(Va),0.,1.))/2.;
          jacobianMatrix[1][3] = \
-(mKca*mTimestep*dxLimit(-(mKca*mTimestep*(-p1 + pa)*limit(onPositive(p1 - \
pa) + onPositive(Va),0.,1.))/2. - \
delayedPart[2][1],0.,mV0)*limit(onPositive(p1 - pa) + \
onPositive(Va),0.,1.))/2.;
          jacobianMatrix[2][0] = 0;
          jacobianMatrix[2][1] = -(mkappa*mp0*Power(mV0,mkappa)*Power(mV0 - \
Va,-1 - mkappa));
          jacobianMatrix[2][2] = 1;
          jacobianMatrix[2][3] = 0;
          jacobianMatrix[3][0] = -(Zc1*onPositive(p1));
          jacobianMatrix[3][1] = 0;
          jacobianMatrix[3][2] = 0;
          jacobianMatrix[3][3] = 1;

          //Solving equation using LU-faktorisation
          ludcmp(jacobianMatrix, order);
          solvlu(jacobianMatrix,systemEquations,deltaStateVar,order);

        for(i=0;i<4;i++)
          {
          stateVar[i] = stateVark[i] - 
          jsyseqnweight[iter - 1] * deltaStateVar[i];
          }
        for(i=0;i<4;i++)
          {
          stateVark[i] = stateVar[i];
          }
        }
        q1=stateVark[0];
        Va=stateVark[1];
        pa=stateVark[2];
        p1=stateVark[3];

        //Calculate the delayed parts
        delayParts2[1] = (-2*Va - mKca*mTimestep*p1*limit(onPositive(p1 - pa) \
+ onPositive(Va),0.,1.) + mKca*mTimestep*pa*limit(onPositive(p1 - pa) + \
onPositive(Va),0.,1.))/2.;

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];

        //Write new values to nodes
        //Port P1
        (*mpND_p1)=p1;
        (*mpND_q1)=q1;
        //outputVariables
        (*mpND_Va)=Va;
        (*mpND_pa)=pa;

        //Update the delayed variabels
        mDelayedPart21.update(delayParts2[1]);

     }
};
#endif // HYDRAULICACKUMULATOR_HPP_INCLUDED
