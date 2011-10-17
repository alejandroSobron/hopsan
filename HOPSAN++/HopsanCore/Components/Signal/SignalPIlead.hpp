#ifndef SIGNALPILEAD_HPP_INCLUDED
#define SIGNALPILEAD_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include <math.h>
#include "matrix.h"

//!
//! @file SignalPIlead.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Tue 16 Aug 2011 23:29:34
//! @brief PI-controler with lead filter in feadback path
//! @ingroup SignalComponents
//!
//This component is generated by COMPGEN for HOPSAN-NG simulation 
//from 
/*{, C:, Documents and Settings, petkr14, My Documents, \
CompgenNG}/ControlNG.nb*/

using namespace hopsan;

class SignalPIlead : public ComponentSignal
{
private:
     double mwa;
     double mda;
     double mumin;
     double mumax;
     double myref;
     double my;
     double mkx;
     Port *mpPyref;
     Port *mpPy;
     Port *mpPkx;
     Port *mpPu;
     double delayParts1[9];
     double delayParts2[9];
     Matrix jacobianMatrix;
     Vec systemEquations;
     Matrix delayedPart;
     int i;
     int iter;
     int mNoiter;
     int jsyseqnweight[4];
     int order[1];
     int mNstep;
     //inputVariables
     double yref;
     double y;
     double kx;
     //outputVariables
     double u;
     //Delay declarations
     //inputVariables pointers
     double *mpND_yref;
     double *mpND_y;
     double *mpND_kx;
     //outputVariables pointers
     double *mpND_u;
     Delay mDelayedPart10;
     Delay mDelayedPart11;

public:
     static Component *Creator()
     {
        return new SignalPIlead();
     }

     SignalPIlead(const std::string name = "PIlead"
                             ,const double wa = 1.
                             ,const double da = 1.
                             ,const double umin = -1.
                             ,const double umax = 1.
                             ,const double yref = 0.
                             ,const double y = 0.
                             ,const double kx = 1.
                             )
        : ComponentSignal(name)
     {
        mNstep=9;
        jacobianMatrix.create(1,1);
        systemEquations.create(1);
        delayedPart.create(2,6);
        mNoiter=2;
        jsyseqnweight[0]=1;
        jsyseqnweight[1]=0.67;
        jsyseqnweight[2]=0.5;
        jsyseqnweight[3]=0.5;

        mwa = wa;
        mda = da;
        mumin = umin;
        mumax = umax;
        myref = yref;
        my = y;
        mkx = kx;

        //Add ports to the component

        //Add inputVariables ports to the component
        mpPyref=addReadPort("Pyref","NodeSignal", Port::NOTREQUIRED);
        mpPy=addReadPort("Py","NodeSignal", Port::NOTREQUIRED);
        mpPkx=addReadPort("Pkx","NodeSignal", Port::NOTREQUIRED);

        //Add outputVariables ports to the component
        mpPu=addWritePort("Pu","NodeSignal", Port::NOTREQUIRED);

        //Register changable parameters to the HOPSAN++ core
        registerParameter("wa", "Break frequency", "rad/s", mwa);
        registerParameter("da", "relative damping", "", mda);
        registerParameter("umin", "Minium output signal", "", mumin);
        registerParameter("umax", "Maximum output signal", "", mumax);
        registerParameter("yref", "Reference value", "", myref);
        registerParameter("y", "Actual value", "", my);
        registerParameter("kx", "Break frequency", "rad/s", mkx);
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Read inputVariables pointers from nodes
        mpND_yref=getSafeNodeDataPtr(mpPyref, NodeSignal::VALUE,myref);
        mpND_y=getSafeNodeDataPtr(mpPy, NodeSignal::VALUE,my);
        mpND_kx=getSafeNodeDataPtr(mpPkx, NodeSignal::VALUE,mkx);
        //Read outputVariable pointers from nodes
        mpND_u=getSafeNodeDataPtr(mpPu, NodeSignal::VALUE);

        //Read variables from nodes

        //Read inputVariables from nodes
        yref = (*mpND_yref);
        y = (*mpND_y);
        kx = (*mpND_kx);

        //Read outputVariables from nodes
        u = mpPu->getStartValue(NodeSignal::VALUE);



        //Initialize delays
        delayParts1[1] = (-2*kx*u - 4*mda*mwa*y + mTimestep*Power(mwa,2)*y + \
2*mwa*yref - mTimestep*Power(mwa,2)*yref)/(2.*kx);
        mDelayedPart11.initialize(mNstep,delayParts1[1]);

        delayedPart[1][1] = delayParts1[1];
     }
    void simulateOneTimestep()
     {
        Vec stateVar(1);
        Vec stateVark(1);
        Vec deltaStateVar(1);

        //Read variables from nodes

        //Read inputVariables from nodes
        yref = (*mpND_yref);
        y = (*mpND_y);
        kx = (*mpND_kx);

        //LocalExpressions

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = u;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //PIlead
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =u - limit(-(mwa*(4*mda*y + mTimestep*mwa*(y - \
yref) - 2*yref))/(2.*kx) - delayedPart[1][1],mumin,mumax);

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;

          //Solving equation using LU-faktorisation
          ludcmp(jacobianMatrix, order);
          solvlu(jacobianMatrix,systemEquations,deltaStateVar,order);

        for(i=0;i<1;i++)
          {
          stateVar[i] = stateVark[i] - 
          jsyseqnweight[iter - 1] * deltaStateVar[i];
          }
        for(i=0;i<1;i++)
          {
          stateVark[i] = stateVar[i];
          }
        }
        u=stateVark[0];

        //Calculate the delayed parts
        delayParts1[1] = (-2*kx*u - 4*mda*mwa*y + mTimestep*Power(mwa,2)*y + \
2*mwa*yref - mTimestep*Power(mwa,2)*yref)/(2.*kx);

        delayedPart[1][1] = delayParts1[1];

        //Write new values to nodes
        //outputVariables
        (*mpND_u)=u;

        //Update the delayed variabels
        mDelayedPart11.update(delayParts1[1]);

     }
};
#endif // SIGNALPILEAD_HPP_INCLUDED
