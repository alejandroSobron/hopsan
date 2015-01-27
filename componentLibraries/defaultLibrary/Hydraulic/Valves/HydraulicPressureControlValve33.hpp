#ifndef HYDRAULICPRESSURECONTROLVALVE33_HPP_INCLUDED
#define HYDRAULICPRESSURECONTROLVALVE33_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file HydraulicPressureControlValve33.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Fri 25 Apr 2014 10:17:35
//! @brief A pressure controlled hydraulic 3-3 directional valve
//! @ingroup HydraulicComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, ComponentLibraries, defaultLibrary, Hydraulic, \
Valves}/HydraulicPressureControlValve33.nb*/

using namespace hopsan;

class HydraulicPressureControlValve33 : public ComponentQ
{
private:
     double rho;
     double Cq;
     double Sd;
     double Frap;
     double Frat;
     double Xap0;
     double Xat0;
     double plam;
     double Bv;
     double Xvmin;
     double Xvmax;
     double phi;
     double ks;
     Port *mpP1;
     Port *mpP2;
     Port *mpP3;
     Port *mpP4;
     double delayParts1[9];
     double delayParts2[9];
     double delayParts3[9];
     double delayParts4[9];
     double delayParts5[9];
     double delayParts6[9];
     double delayParts7[9];
     Matrix jacobianMatrix;
     Vec systemEquations;
     Matrix delayedPart;
     int i;
     int iter;
     int mNoiter;
     double jsyseqnweight[4];
     int order[7];
     int mNstep;
     //Port P1 variable
     double p1;
     double q1;
     double T1;
     double dE1;
     double c1;
     double Zc1;
     //Port P2 variable
     double p2;
     double q2;
     double T2;
     double dE2;
     double c2;
     double Zc2;
     //Port P3 variable
     double p3;
     double q3;
     double T3;
     double dE3;
     double c3;
     double Zc3;
     //Port P4 variable
     double p4;
     double q4;
     double T4;
     double dE4;
     double c4;
     double Zc4;
//==This code has been autogenerated using Compgen==
     //inputVariables
     double pref;
     //outputVariables
     double xv;
     //LocalExpressions variables
     double Ks;
     double Av;
     //Expressions variables
     //Port P1 pointer
     double *mpND_p1;
     double *mpND_q1;
     double *mpND_T1;
     double *mpND_dE1;
     double *mpND_c1;
     double *mpND_Zc1;
     //Port P2 pointer
     double *mpND_p2;
     double *mpND_q2;
     double *mpND_T2;
     double *mpND_dE2;
     double *mpND_c2;
     double *mpND_Zc2;
     //Port P3 pointer
     double *mpND_p3;
     double *mpND_q3;
     double *mpND_T3;
     double *mpND_dE3;
     double *mpND_c3;
     double *mpND_Zc3;
     //Port P4 pointer
     double *mpND_p4;
     double *mpND_q4;
     double *mpND_T4;
     double *mpND_dE4;
     double *mpND_c4;
     double *mpND_Zc4;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     double *mppref;
     //inputParameters pointers
     double *mprho;
     double *mpCq;
     double *mpSd;
     double *mpFrap;
     double *mpFrat;
     double *mpXap0;
     double *mpXat0;
     double *mpplam;
     double *mpBv;
     double *mpXvmin;
     double *mpXvmax;
     double *mpphi;
     double *mpks;
     //outputVariables pointers
     double *mpxv;
     Delay mDelayedPart10;
     Delay mDelayedPart11;
     Delay mDelayedPart20;
     Delay mDelayedPart30;
     Delay mDelayedPart40;
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new HydraulicPressureControlValve33();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;
        jacobianMatrix.create(7,7);
        systemEquations.create(7);
        delayedPart.create(8,6);
        mNoiter=2;
        jsyseqnweight[0]=1;
        jsyseqnweight[1]=0.67;
        jsyseqnweight[2]=0.5;
        jsyseqnweight[3]=0.5;


        //Add ports to the component
        mpP1=addPowerPort("P1","NodeHydraulic");
        mpP2=addPowerPort("P2","NodeHydraulic");
        mpP3=addPowerPort("P3","NodeHydraulic");
        mpP4=addPowerPort("P4","NodeHydraulic");
        //Add inputVariables to the component
            addInputVariable("pref","Reference pressure","Pa",1.e6,&mppref);

        //Add inputParammeters to the component
            addInputVariable("rho", "oil density", "kg/m3", 860.,&mprho);
            addInputVariable("Cq", "Flow coefficient.", "", 0.67,&mpCq);
            addInputVariable("Sd", "spool diameter", "m", 0.01,&mpSd);
            addInputVariable("Frap", "Spool cricle fraction(P-A)", "", \
1.,&mpFrap);
            addInputVariable("Frat", "Spool cricle fraction(A-T)", "", \
1.,&mpFrat);
            addInputVariable("Xap0", "Underlap", "m", 0.,&mpXap0);
            addInputVariable("Xat0", "Underlap", "m", 0.,&mpXat0);
            addInputVariable("plam", "Turbulence onset pressure", "Pa", \
10000.,&mpplam);
            addInputVariable("Bv", "Damping", "N/(m s)", 100.,&mpBv);
            addInputVariable("Xvmin", "Max spool displacement", "m", \
-0.01,&mpXvmin);
            addInputVariable("Xvmax", "Max spool displacement", "m", \
0.01,&mpXvmax);
            addInputVariable("phi", "Stream angle", "rad", 0.03,&mpphi);
            addInputVariable("ks", "Spring constant", "N/m", 100.,&mpks);
        //Add outputVariables to the component
            addOutputVariable("xv","Spool position","m",0.,&mpxv);

//==This code has been autogenerated using Compgen==
        //Add constantParameters
        mpSolver = new EquationSystemSolver(this,7);
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port P1
        mpND_p1=getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
        mpND_q1=getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
        mpND_T1=getSafeNodeDataPtr(mpP1, NodeHydraulic::Temperature);
        mpND_dE1=getSafeNodeDataPtr(mpP1, NodeHydraulic::HeatFlow);
        mpND_c1=getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
        mpND_Zc1=getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);
        //Port P2
        mpND_p2=getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
        mpND_q2=getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
        mpND_T2=getSafeNodeDataPtr(mpP2, NodeHydraulic::Temperature);
        mpND_dE2=getSafeNodeDataPtr(mpP2, NodeHydraulic::HeatFlow);
        mpND_c2=getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
        mpND_Zc2=getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);
        //Port P3
        mpND_p3=getSafeNodeDataPtr(mpP3, NodeHydraulic::Pressure);
        mpND_q3=getSafeNodeDataPtr(mpP3, NodeHydraulic::Flow);
        mpND_T3=getSafeNodeDataPtr(mpP3, NodeHydraulic::Temperature);
        mpND_dE3=getSafeNodeDataPtr(mpP3, NodeHydraulic::HeatFlow);
        mpND_c3=getSafeNodeDataPtr(mpP3, NodeHydraulic::WaveVariable);
        mpND_Zc3=getSafeNodeDataPtr(mpP3, NodeHydraulic::CharImpedance);
        //Port P4
        mpND_p4=getSafeNodeDataPtr(mpP4, NodeHydraulic::Pressure);
        mpND_q4=getSafeNodeDataPtr(mpP4, NodeHydraulic::Flow);
        mpND_T4=getSafeNodeDataPtr(mpP4, NodeHydraulic::Temperature);
        mpND_dE4=getSafeNodeDataPtr(mpP4, NodeHydraulic::HeatFlow);
        mpND_c4=getSafeNodeDataPtr(mpP4, NodeHydraulic::WaveVariable);
        mpND_Zc4=getSafeNodeDataPtr(mpP4, NodeHydraulic::CharImpedance);

        //Read variables from nodes
        //Port P1
        p1 = (*mpND_p1);
        q1 = (*mpND_q1);
        T1 = (*mpND_T1);
        dE1 = (*mpND_dE1);
        c1 = (*mpND_c1);
        Zc1 = (*mpND_Zc1);
        //Port P2
        p2 = (*mpND_p2);
        q2 = (*mpND_q2);
        T2 = (*mpND_T2);
        dE2 = (*mpND_dE2);
        c2 = (*mpND_c2);
        Zc2 = (*mpND_Zc2);
        //Port P3
        p3 = (*mpND_p3);
        q3 = (*mpND_q3);
        T3 = (*mpND_T3);
        dE3 = (*mpND_dE3);
        c3 = (*mpND_c3);
        Zc3 = (*mpND_Zc3);
        //Port P4
        p4 = (*mpND_p4);
        q4 = (*mpND_q4);
        T4 = (*mpND_T4);
        dE4 = (*mpND_dE4);
        c4 = (*mpND_c4);
        Zc4 = (*mpND_Zc4);

        //Read inputVariables from nodes
        pref = (*mppref);

        //Read inputParameters from nodes
        rho = (*mprho);
        Cq = (*mpCq);
        Sd = (*mpSd);
        Frap = (*mpFrap);
        Frat = (*mpFrat);
        Xap0 = (*mpXap0);
        Xat0 = (*mpXat0);
        plam = (*mpplam);
        Bv = (*mpBv);
        Xvmin = (*mpXvmin);
        Xvmax = (*mpXvmax);
        phi = (*mpphi);
        ks = (*mpks);

        //Read outputVariables from nodes
        xv = (*mpxv);

//==This code has been autogenerated using Compgen==

        //LocalExpressions
        Ks = (1.4142135623730951*Cq)/Sqrt(rho);
        Av = 0.785398*Power(Sd,2);
        p4 = c4;

        //Initialize delays
        delayParts1[1] = (-1.*Av*mTimestep*p4 + Av*mTimestep*pref - 2.*Bv*xv \
+ ks*mTimestep*xv + 6.28319*Cq*Frat*mTimestep*Sd*xv*Abs(-p2 + \
p3)*Cos(phi)*onPositive(limit(-Xat0 - xv,0.,-Xat0 - Xvmin)) + \
6.28319*Cq*Frap*mTimestep*Sd*xv*Abs(p1 - p3)*Cos(phi)*onPositive(limit(Xap0 + \
xv,0.,Xap0 + Xvmax)))/(2.*Bv + ks*mTimestep + \
6.28319*Cq*Frat*mTimestep*Sd*Abs(-p2 + p3)*Cos(phi)*onPositive(limit(-Xat0 - \
xv,0.,-Xat0 - Xvmin)) + 6.28319*Cq*Frap*mTimestep*Sd*Abs(p1 - \
p3)*Cos(phi)*onPositive(limit(Xap0 + xv,0.,Xap0 + Xvmax)));
        mDelayedPart11.initialize(mNstep,delayParts1[1]);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
        delayedPart[5][1] = delayParts5[1];
        delayedPart[6][1] = delayParts6[1];
        delayedPart[7][1] = delayParts7[1];
     }
    void simulateOneTimestep()
     {
        Vec stateVar(7);
        Vec stateVark(7);
        Vec deltaStateVar(7);

        //Read variables from nodes
        //Port P1
        T1 = (*mpND_T1);
        c1 = (*mpND_c1);
        Zc1 = (*mpND_Zc1);
        //Port P2
        T2 = (*mpND_T2);
        c2 = (*mpND_c2);
        Zc2 = (*mpND_Zc2);
        //Port P3
        T3 = (*mpND_T3);
        c3 = (*mpND_c3);
        Zc3 = (*mpND_Zc3);
        //Port P4
        T4 = (*mpND_T4);
        c4 = (*mpND_c4);
        Zc4 = (*mpND_Zc4);

        //Read inputVariables from nodes
        pref = (*mppref);

        //LocalExpressions
        Ks = (1.4142135623730951*Cq)/Sqrt(rho);
        Av = 0.785398*Power(Sd,2);
        p4 = c4;

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = xv;
        stateVark[1] = q1;
        stateVark[2] = q2;
        stateVark[3] = q3;
        stateVark[4] = p1;
        stateVark[5] = p2;
        stateVark[6] = p3;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //PressureControlValve33
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =xv - limit(-((Av*mTimestep*(-1.*p4 + \
pref))/(2.*Bv + ks*mTimestep + 6.28319*Cq*Frat*mTimestep*Sd*Abs(p2 - \
p3)*Cos(phi)*onPositive(limit(-Xat0 - xv,0.,-Xat0 - Xvmin)) + \
6.28319*Cq*Frap*mTimestep*Sd*Abs(p1 - p3)*Cos(phi)*onPositive(limit(Xap0 + \
xv,0.,Xap0 + Xvmax)))) - delayedPart[1][1],Xvmin,Xvmax);
          systemEquations[1] =q1 + 3.14159*Frap*Ks*Sd*limit(Xap0 + xv,0.,Xap0 \
+ Xvmax)*signedSquareL(p1 - p3,plam);
          systemEquations[2] =q2 + 3.14159*Frat*Ks*Sd*limit(Xap0 - xv,0.,Xat0 \
+ Xvmax)*signedSquareL(p2 - p3,plam);
          systemEquations[3] =q3 + Ks*Sd*(-3.14159*Frap*limit(Xap0 + \
xv,0.,Xap0 + Xvmax)*signedSquareL(p1 - p3,plam) - 3.14159*Frat*limit(Xap0 - \
xv,0.,Xat0 + Xvmax)*signedSquareL(p2 - p3,plam));
          systemEquations[4] =p1 - lowLimit(c1 + q1*Zc1*onPositive(p1),0);
          systemEquations[5] =p2 - lowLimit(c2 + q2*Zc2*onPositive(p2),0);
          systemEquations[6] =p3 - lowLimit(c3 + q3*Zc3*onPositive(p3),0);

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;
          jacobianMatrix[0][1] = 0;
          jacobianMatrix[0][2] = 0;
          jacobianMatrix[0][3] = 0;
          jacobianMatrix[0][4] = \
(-6.28319*Av*Cq*Frap*Power(mTimestep,2)*(-1.*p4 + pref)*Sd*Cos(phi)*dxAbs(p1 \
- p3)*dxLimit(-((Av*mTimestep*(-1.*p4 + pref))/(2.*Bv + ks*mTimestep + \
6.28319*Cq*Frat*mTimestep*Sd*Abs(p2 - p3)*Cos(phi)*onPositive(limit(-Xat0 - \
xv,0.,-Xat0 - Xvmin)) + 6.28319*Cq*Frap*mTimestep*Sd*Abs(p1 - \
p3)*Cos(phi)*onPositive(limit(Xap0 + xv,0.,Xap0 + Xvmax)))) - \
delayedPart[1][1],Xvmin,Xvmax)*onPositive(limit(Xap0 + xv,0.,Xap0 + \
Xvmax)))/Power(2.*Bv + ks*mTimestep + 6.28319*Cq*Frat*mTimestep*Sd*Abs(p2 - \
p3)*Cos(phi)*onPositive(limit(-Xat0 - xv,0.,-Xat0 - Xvmin)) + \
6.28319*Cq*Frap*mTimestep*Sd*Abs(p1 - p3)*Cos(phi)*onPositive(limit(Xap0 + \
xv,0.,Xap0 + Xvmax)),2);
          jacobianMatrix[0][5] = \
(-6.28319*Av*Cq*Frat*Power(mTimestep,2)*(-1.*p4 + pref)*Sd*Cos(phi)*dxAbs(p2 \
- p3)*dxLimit(-((Av*mTimestep*(-1.*p4 + pref))/(2.*Bv + ks*mTimestep + \
6.28319*Cq*Frat*mTimestep*Sd*Abs(p2 - p3)*Cos(phi)*onPositive(limit(-Xat0 - \
xv,0.,-Xat0 - Xvmin)) + 6.28319*Cq*Frap*mTimestep*Sd*Abs(p1 - \
p3)*Cos(phi)*onPositive(limit(Xap0 + xv,0.,Xap0 + Xvmax)))) - \
delayedPart[1][1],Xvmin,Xvmax)*onPositive(limit(-Xat0 - xv,0.,-Xat0 - \
Xvmin)))/Power(2.*Bv + ks*mTimestep + 6.28319*Cq*Frat*mTimestep*Sd*Abs(p2 - \
p3)*Cos(phi)*onPositive(limit(-Xat0 - xv,0.,-Xat0 - Xvmin)) + \
6.28319*Cq*Frap*mTimestep*Sd*Abs(p1 - p3)*Cos(phi)*onPositive(limit(Xap0 + \
xv,0.,Xap0 + Xvmax)),2);
          jacobianMatrix[0][6] = -((Av*mTimestep*(-1.*p4 + \
pref)*dxLimit(-((Av*mTimestep*(-1.*p4 + pref))/(2.*Bv + ks*mTimestep + \
6.28319*Cq*Frat*mTimestep*Sd*Abs(p2 - p3)*Cos(phi)*onPositive(limit(-Xat0 - \
xv,0.,-Xat0 - Xvmin)) + 6.28319*Cq*Frap*mTimestep*Sd*Abs(p1 - \
p3)*Cos(phi)*onPositive(limit(Xap0 + xv,0.,Xap0 + Xvmax)))) - \
delayedPart[1][1],Xvmin,Xvmax)*(-6.28319*Cq*Frat*mTimestep*Sd*Cos(phi)*dxAbs(\
p2 - p3)*onPositive(limit(-Xat0 - xv,0.,-Xat0 - Xvmin)) - \
6.28319*Cq*Frap*mTimestep*Sd*Cos(phi)*dxAbs(p1 - p3)*onPositive(limit(Xap0 + \
xv,0.,Xap0 + Xvmax))))/Power(2.*Bv + ks*mTimestep + \
6.28319*Cq*Frat*mTimestep*Sd*Abs(p2 - p3)*Cos(phi)*onPositive(limit(-Xat0 - \
xv,0.,-Xat0 - Xvmin)) + 6.28319*Cq*Frap*mTimestep*Sd*Abs(p1 - \
p3)*Cos(phi)*onPositive(limit(Xap0 + xv,0.,Xap0 + Xvmax)),2));
          jacobianMatrix[1][0] = 3.14159*Frap*Ks*Sd*dxLimit(Xap0 + xv,0.,Xap0 \
+ Xvmax)*signedSquareL(p1 - p3,plam);
          jacobianMatrix[1][1] = 1;
          jacobianMatrix[1][2] = 0;
          jacobianMatrix[1][3] = 0;
          jacobianMatrix[1][4] = 3.14159*Frap*Ks*Sd*dxSignedSquareL(p1 - \
p3,plam)*limit(Xap0 + xv,0.,Xap0 + Xvmax);
          jacobianMatrix[1][5] = 0;
          jacobianMatrix[1][6] = -3.14159*Frap*Ks*Sd*dxSignedSquareL(p1 - \
p3,plam)*limit(Xap0 + xv,0.,Xap0 + Xvmax);
          jacobianMatrix[2][0] = -3.14159*Frat*Ks*Sd*dxLimit(Xap0 - \
xv,0.,Xat0 + Xvmax)*signedSquareL(p2 - p3,plam);
          jacobianMatrix[2][1] = 0;
          jacobianMatrix[2][2] = 1;
          jacobianMatrix[2][3] = 0;
          jacobianMatrix[2][4] = 0;
          jacobianMatrix[2][5] = 3.14159*Frat*Ks*Sd*dxSignedSquareL(p2 - \
p3,plam)*limit(Xap0 - xv,0.,Xat0 + Xvmax);
          jacobianMatrix[2][6] = -3.14159*Frat*Ks*Sd*dxSignedSquareL(p2 - \
p3,plam)*limit(Xap0 - xv,0.,Xat0 + Xvmax);
          jacobianMatrix[3][0] = Ks*Sd*(-3.14159*Frap*dxLimit(Xap0 + \
xv,0.,Xap0 + Xvmax)*signedSquareL(p1 - p3,plam) + 3.14159*Frat*dxLimit(Xap0 - \
xv,0.,Xat0 + Xvmax)*signedSquareL(p2 - p3,plam));
          jacobianMatrix[3][1] = 0;
          jacobianMatrix[3][2] = 0;
          jacobianMatrix[3][3] = 1;
          jacobianMatrix[3][4] = -3.14159*Frap*Ks*Sd*dxSignedSquareL(p1 - \
p3,plam)*limit(Xap0 + xv,0.,Xap0 + Xvmax);
          jacobianMatrix[3][5] = -3.14159*Frat*Ks*Sd*dxSignedSquareL(p2 - \
p3,plam)*limit(Xap0 - xv,0.,Xat0 + Xvmax);
          jacobianMatrix[3][6] = Ks*Sd*(3.14159*Frat*dxSignedSquareL(p2 - \
p3,plam)*limit(Xap0 - xv,0.,Xat0 + Xvmax) + 3.14159*Frap*dxSignedSquareL(p1 - \
p3,plam)*limit(Xap0 + xv,0.,Xap0 + Xvmax));
          jacobianMatrix[4][0] = 0;
          jacobianMatrix[4][1] = -(Zc1*dxLowLimit(c1 + \
q1*Zc1*onPositive(p1),0)*onPositive(p1));
          jacobianMatrix[4][2] = 0;
          jacobianMatrix[4][3] = 0;
          jacobianMatrix[4][4] = 1;
          jacobianMatrix[4][5] = 0;
          jacobianMatrix[4][6] = 0;
          jacobianMatrix[5][0] = 0;
          jacobianMatrix[5][1] = 0;
          jacobianMatrix[5][2] = -(Zc2*dxLowLimit(c2 + \
q2*Zc2*onPositive(p2),0)*onPositive(p2));
          jacobianMatrix[5][3] = 0;
          jacobianMatrix[5][4] = 0;
          jacobianMatrix[5][5] = 1;
          jacobianMatrix[5][6] = 0;
          jacobianMatrix[6][0] = 0;
          jacobianMatrix[6][1] = 0;
          jacobianMatrix[6][2] = 0;
          jacobianMatrix[6][3] = -(Zc3*dxLowLimit(c3 + \
q3*Zc3*onPositive(p3),0)*onPositive(p3));
          jacobianMatrix[6][4] = 0;
          jacobianMatrix[6][5] = 0;
          jacobianMatrix[6][6] = 1;
//==This code has been autogenerated using Compgen==

          //Solving equation using LU-faktorisation
          mpSolver->solve(jacobianMatrix, systemEquations, stateVark, iter);
          xv=stateVark[0];
          q1=stateVark[1];
          q2=stateVark[2];
          q3=stateVark[3];
          p1=stateVark[4];
          p2=stateVark[5];
          p3=stateVark[6];
          //Expressions
          p4 = c4;
        }

        //Calculate the delayed parts
        delayParts1[1] = (-1.*Av*mTimestep*p4 + Av*mTimestep*pref - 2.*Bv*xv \
+ ks*mTimestep*xv + 6.28319*Cq*Frat*mTimestep*Sd*xv*Abs(-p2 + \
p3)*Cos(phi)*onPositive(limit(-Xat0 - xv,0.,-Xat0 - Xvmin)) + \
6.28319*Cq*Frap*mTimestep*Sd*xv*Abs(p1 - p3)*Cos(phi)*onPositive(limit(Xap0 + \
xv,0.,Xap0 + Xvmax)))/(2.*Bv + ks*mTimestep + \
6.28319*Cq*Frat*mTimestep*Sd*Abs(-p2 + p3)*Cos(phi)*onPositive(limit(-Xat0 - \
xv,0.,-Xat0 - Xvmin)) + 6.28319*Cq*Frap*mTimestep*Sd*Abs(p1 - \
p3)*Cos(phi)*onPositive(limit(Xap0 + xv,0.,Xap0 + Xvmax)));

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
        delayedPart[5][1] = delayParts5[1];
        delayedPart[6][1] = delayParts6[1];
        delayedPart[7][1] = delayParts7[1];

        //Write new values to nodes
        //Port P1
        (*mpND_p1)=p1;
        (*mpND_q1)=q1;
        (*mpND_dE1)=dE1;
        //Port P2
        (*mpND_p2)=p2;
        (*mpND_q2)=q2;
        (*mpND_dE2)=dE2;
        //Port P3
        (*mpND_p3)=p3;
        (*mpND_q3)=q3;
        (*mpND_dE3)=dE3;
        //Port P4
        (*mpND_p4)=p4;
        (*mpND_q4)=q4;
        (*mpND_dE4)=dE4;
        //outputVariables
        (*mpxv)=xv;

        //Update the delayed variabels
        mDelayedPart11.update(delayParts1[1]);

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // HYDRAULICPRESSURECONTROLVALVE33_HPP_INCLUDED
