#ifndef HYDRAULICVALVE63OC_HPP_INCLUDED
#define HYDRAULICVALVE63OC_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file HydraulicValve63OC.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Tue 21 May 2013 09:46:23
//! @brief A load sensing hydraulic directional valve
//! @ingroup HydraulicComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, HOPSAN++, CompgenModels}/HydraulicComponents.nb*/

using namespace hopsan;

class HydraulicValve63OC : public ComponentQ
{
private:
     double rho;
     double Cq;
     double Sd;
     double Frap;
     double Frat;
     double Frbp;
     double Frbt;
     double Froc;
     double Xap0;
     double Xat0;
     double Xbp0;
     double Xbt0;
     double Xoc0;
     double Xvmax;
     double plam;
     Port *mpPp;
     Port *mpPt;
     Port *mpPa;
     Port *mpPb;
     Port *mpPocp;
     Port *mpPoct;
     double delayParts1[9];
     double delayParts2[9];
     double delayParts3[9];
     double delayParts4[9];
     double delayParts5[9];
     double delayParts6[9];
     double delayParts7[9];
     double delayParts8[9];
     double delayParts9[9];
     double delayParts10[9];
     double delayParts11[9];
     Matrix jacobianMatrix;
     Vec systemEquations;
     Matrix delayedPart;
     int i;
     int iter;
     int mNoiter;
     double jsyseqnweight[4];
     int order[11];
     int mNstep;
     //Port Pp variable
     double pp;
     double qp;
     double Tp;
     double dEp;
     double cp;
     double Zcp;
     //Port Pt variable
     double pt;
     double qt;
     double Tt;
     double dEt;
     double ct;
     double Zct;
     //Port Pa variable
     double pa;
     double qa;
     double Ta;
     double dEa;
     double ca;
     double Zca;
     //Port Pb variable
     double pb;
     double qb;
     double Tb;
     double dEb;
     double cb;
     double Zcb;
     //Port Pocp variable
     double pocp;
     double qocp;
     double Tocp;
     double dEocp;
     double cocp;
     double Zcocp;
     //Port Poct variable
     double poct;
     double qoct;
     double Toct;
     double dEoct;
     double coct;
     double Zcoct;
//==This code has been autogenerated using Compgen==
     //inputVariables
     double xv;
     //outputVariables
     //LocalExpressions variables
     double Ks;
     double Kspa;
     double Ksta;
     double Kspb;
     double Kstb;
     double Ksoc;
     //Expressions variables
     //Port Pp pointer
     double *mpND_pp;
     double *mpND_qp;
     double *mpND_Tp;
     double *mpND_dEp;
     double *mpND_cp;
     double *mpND_Zcp;
     //Port Pt pointer
     double *mpND_pt;
     double *mpND_qt;
     double *mpND_Tt;
     double *mpND_dEt;
     double *mpND_ct;
     double *mpND_Zct;
     //Port Pa pointer
     double *mpND_pa;
     double *mpND_qa;
     double *mpND_Ta;
     double *mpND_dEa;
     double *mpND_ca;
     double *mpND_Zca;
     //Port Pb pointer
     double *mpND_pb;
     double *mpND_qb;
     double *mpND_Tb;
     double *mpND_dEb;
     double *mpND_cb;
     double *mpND_Zcb;
     //Port Pocp pointer
     double *mpND_pocp;
     double *mpND_qocp;
     double *mpND_Tocp;
     double *mpND_dEocp;
     double *mpND_cocp;
     double *mpND_Zcocp;
     //Port Poct pointer
     double *mpND_poct;
     double *mpND_qoct;
     double *mpND_Toct;
     double *mpND_dEoct;
     double *mpND_coct;
     double *mpND_Zcoct;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     double *mpxv;
     //outputVariables pointers
     Delay mDelayedPart10;
     Delay mDelayedPart20;
     Delay mDelayedPart30;
     Delay mDelayedPart40;
     Delay mDelayedPart50;
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new HydraulicValve63OC();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;
        jacobianMatrix.create(11,11);
        systemEquations.create(11);
        delayedPart.create(12,6);
        mNoiter=2;
        jsyseqnweight[0]=1;
        jsyseqnweight[1]=0.67;
        jsyseqnweight[2]=0.5;
        jsyseqnweight[3]=0.5;


        //Add ports to the component
        mpPp=addPowerPort("Pp","NodeHydraulic");
        mpPt=addPowerPort("Pt","NodeHydraulic");
        mpPa=addPowerPort("Pa","NodeHydraulic");
        mpPb=addPowerPort("Pb","NodeHydraulic");
        mpPocp=addPowerPort("Pocp","NodeHydraulic");
        mpPoct=addPowerPort("Poct","NodeHydraulic");
        //Add inputVariables to the component
            addInputVariable("xv","Spool position","m",0.,&mpxv);

        //Add outputVariables to the component

//==This code has been autogenerated using Compgen==
        //Add constants/parameters
            addConstant("rho", "oil density", "kg/m3", 860.,rho);
            addConstant("Cq", "Flow coefficient.", "", 0.67,Cq);
            addConstant("Sd", "spool diameter", "m", 0.001,Sd);
            addConstant("Frap", "Spool cricle fraction(P-A)", "", 1.,Frap);
            addConstant("Frat", "Spool cricle fraction(A-T)", "", 1.,Frat);
            addConstant("Frbp", "Spool cricle fraction(P-B)", "", 1.,Frbp);
            addConstant("Frbt", "Spool cricle fraction(B-T)", "", 1.,Frbt);
            addConstant("Froc", "Spool cricle fraction(OC)", "", 1.,Froc);
            addConstant("Xap0", "Overlap", "m", 0.,Xap0);
            addConstant("Xat0", "Overlap", "m", 0.,Xat0);
            addConstant("Xbp0", "Overlap", "m", 0.,Xbp0);
            addConstant("Xbt0", "Overlap", "m", 0.,Xbt0);
            addConstant("Xoc0", "Underlap oc port", "m", 0.,Xoc0);
            addConstant("Xvmax", "Max valve opening", "m", 0.01,Xvmax);
            addConstant("plam", "Turbulence onset pressure", "Pa", \
10000.,plam);
        mpSolver = new EquationSystemSolver(this,11);
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port Pp
        mpND_pp=getSafeNodeDataPtr(mpPp, NodeHydraulic::Pressure);
        mpND_qp=getSafeNodeDataPtr(mpPp, NodeHydraulic::Flow);
        mpND_Tp=getSafeNodeDataPtr(mpPp, NodeHydraulic::Temperature);
        mpND_dEp=getSafeNodeDataPtr(mpPp, NodeHydraulic::HeatFlow);
        mpND_cp=getSafeNodeDataPtr(mpPp, NodeHydraulic::WaveVariable);
        mpND_Zcp=getSafeNodeDataPtr(mpPp, NodeHydraulic::CharImpedance);
        //Port Pt
        mpND_pt=getSafeNodeDataPtr(mpPt, NodeHydraulic::Pressure);
        mpND_qt=getSafeNodeDataPtr(mpPt, NodeHydraulic::Flow);
        mpND_Tt=getSafeNodeDataPtr(mpPt, NodeHydraulic::Temperature);
        mpND_dEt=getSafeNodeDataPtr(mpPt, NodeHydraulic::HeatFlow);
        mpND_ct=getSafeNodeDataPtr(mpPt, NodeHydraulic::WaveVariable);
        mpND_Zct=getSafeNodeDataPtr(mpPt, NodeHydraulic::CharImpedance);
        //Port Pa
        mpND_pa=getSafeNodeDataPtr(mpPa, NodeHydraulic::Pressure);
        mpND_qa=getSafeNodeDataPtr(mpPa, NodeHydraulic::Flow);
        mpND_Ta=getSafeNodeDataPtr(mpPa, NodeHydraulic::Temperature);
        mpND_dEa=getSafeNodeDataPtr(mpPa, NodeHydraulic::HeatFlow);
        mpND_ca=getSafeNodeDataPtr(mpPa, NodeHydraulic::WaveVariable);
        mpND_Zca=getSafeNodeDataPtr(mpPa, NodeHydraulic::CharImpedance);
        //Port Pb
        mpND_pb=getSafeNodeDataPtr(mpPb, NodeHydraulic::Pressure);
        mpND_qb=getSafeNodeDataPtr(mpPb, NodeHydraulic::Flow);
        mpND_Tb=getSafeNodeDataPtr(mpPb, NodeHydraulic::Temperature);
        mpND_dEb=getSafeNodeDataPtr(mpPb, NodeHydraulic::HeatFlow);
        mpND_cb=getSafeNodeDataPtr(mpPb, NodeHydraulic::WaveVariable);
        mpND_Zcb=getSafeNodeDataPtr(mpPb, NodeHydraulic::CharImpedance);
        //Port Pocp
        mpND_pocp=getSafeNodeDataPtr(mpPocp, NodeHydraulic::Pressure);
        mpND_qocp=getSafeNodeDataPtr(mpPocp, NodeHydraulic::Flow);
        mpND_Tocp=getSafeNodeDataPtr(mpPocp, NodeHydraulic::Temperature);
        mpND_dEocp=getSafeNodeDataPtr(mpPocp, NodeHydraulic::HeatFlow);
        mpND_cocp=getSafeNodeDataPtr(mpPocp, NodeHydraulic::WaveVariable);
        mpND_Zcocp=getSafeNodeDataPtr(mpPocp, NodeHydraulic::CharImpedance);
        //Port Poct
        mpND_poct=getSafeNodeDataPtr(mpPoct, NodeHydraulic::Pressure);
        mpND_qoct=getSafeNodeDataPtr(mpPoct, NodeHydraulic::Flow);
        mpND_Toct=getSafeNodeDataPtr(mpPoct, NodeHydraulic::Temperature);
        mpND_dEoct=getSafeNodeDataPtr(mpPoct, NodeHydraulic::HeatFlow);
        mpND_coct=getSafeNodeDataPtr(mpPoct, NodeHydraulic::WaveVariable);
        mpND_Zcoct=getSafeNodeDataPtr(mpPoct, NodeHydraulic::CharImpedance);

        //Read variables from nodes
        //Port Pp
        pp = (*mpND_pp);
        qp = (*mpND_qp);
        Tp = (*mpND_Tp);
        dEp = (*mpND_dEp);
        cp = (*mpND_cp);
        Zcp = (*mpND_Zcp);
        //Port Pt
        pt = (*mpND_pt);
        qt = (*mpND_qt);
        Tt = (*mpND_Tt);
        dEt = (*mpND_dEt);
        ct = (*mpND_ct);
        Zct = (*mpND_Zct);
        //Port Pa
        pa = (*mpND_pa);
        qa = (*mpND_qa);
        Ta = (*mpND_Ta);
        dEa = (*mpND_dEa);
        ca = (*mpND_ca);
        Zca = (*mpND_Zca);
        //Port Pb
        pb = (*mpND_pb);
        qb = (*mpND_qb);
        Tb = (*mpND_Tb);
        dEb = (*mpND_dEb);
        cb = (*mpND_cb);
        Zcb = (*mpND_Zcb);
        //Port Pocp
        pocp = (*mpND_pocp);
        qocp = (*mpND_qocp);
        Tocp = (*mpND_Tocp);
        dEocp = (*mpND_dEocp);
        cocp = (*mpND_cocp);
        Zcocp = (*mpND_Zcocp);
        //Port Poct
        poct = (*mpND_poct);
        qoct = (*mpND_qoct);
        Toct = (*mpND_Toct);
        dEoct = (*mpND_dEoct);
        coct = (*mpND_coct);
        Zcoct = (*mpND_Zcoct);

        //Read inputVariables from nodes
        xv = (*mpxv);

        //Read outputVariables from nodes

//==This code has been autogenerated using Compgen==

        //LocalExpressions
        Ks = (1.4142135623730951*Cq)/Sqrt(rho);
        Kspa = 3.14159*Frap*Ks*Sd*limit(-Xap0 + xv,0.,-Xap0 + Xvmax);
        Ksta = 3.14159*Frat*Ks*Sd*limit(-Xap0 - xv,0.,-Xat0 + Xvmax);
        Kspb = 3.14159*Frbp*Ks*Sd*limit(-Xap0 - xv,0.,-Xbp0 + Xvmax);
        Kstb = 3.14159*Frbt*Ks*Sd*limit(-Xap0 + xv,0.,-Xbt0 + Xvmax);
        Ksoc = 3.14159*Froc*Ks*Sd*limit(Xoc0 - Abs(xv),0.,Xoc0);

        //Initialize delays

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
        delayedPart[5][1] = delayParts5[1];
        delayedPart[6][1] = delayParts6[1];
        delayedPart[7][1] = delayParts7[1];
        delayedPart[8][1] = delayParts8[1];
        delayedPart[9][1] = delayParts9[1];
        delayedPart[10][1] = delayParts10[1];
        delayedPart[11][1] = delayParts11[1];
     }
    void simulateOneTimestep()
     {
        Vec stateVar(11);
        Vec stateVark(11);
        Vec deltaStateVar(11);

        //Read variables from nodes
        //Port Pp
        Tp = (*mpND_Tp);
        cp = (*mpND_cp);
        Zcp = (*mpND_Zcp);
        //Port Pt
        Tt = (*mpND_Tt);
        ct = (*mpND_ct);
        Zct = (*mpND_Zct);
        //Port Pa
        Ta = (*mpND_Ta);
        ca = (*mpND_ca);
        Zca = (*mpND_Zca);
        //Port Pb
        Tb = (*mpND_Tb);
        cb = (*mpND_cb);
        Zcb = (*mpND_Zcb);
        //Port Pocp
        Tocp = (*mpND_Tocp);
        cocp = (*mpND_cocp);
        Zcocp = (*mpND_Zcocp);
        //Port Poct
        Toct = (*mpND_Toct);
        coct = (*mpND_coct);
        Zcoct = (*mpND_Zcoct);

        //Read inputVariables from nodes
        xv = (*mpxv);

        //LocalExpressions
        Ks = (1.4142135623730951*Cq)/Sqrt(rho);
        Kspa = 3.14159*Frap*Ks*Sd*limit(-Xap0 + xv,0.,-Xap0 + Xvmax);
        Ksta = 3.14159*Frat*Ks*Sd*limit(-Xap0 - xv,0.,-Xat0 + Xvmax);
        Kspb = 3.14159*Frbp*Ks*Sd*limit(-Xap0 - xv,0.,-Xbp0 + Xvmax);
        Kstb = 3.14159*Frbt*Ks*Sd*limit(-Xap0 + xv,0.,-Xbt0 + Xvmax);
        Ksoc = 3.14159*Froc*Ks*Sd*limit(Xoc0 - Abs(xv),0.,Xoc0);

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = qp;
        stateVark[1] = qt;
        stateVark[2] = qa;
        stateVark[3] = qb;
        stateVark[4] = qoct;
        stateVark[5] = pp;
        stateVark[6] = pt;
        stateVark[7] = pa;
        stateVark[8] = pb;
        stateVark[9] = pocp;
        stateVark[10] = poct;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //Valve63OC
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =qp + Kspa*signedSquareL(-pa + pp,plam) + \
Kspb*signedSquareL(-pb + pp,plam);
          systemEquations[1] =qt + Ksta*signedSquareL(-pa + pt,plam) + \
Kstb*signedSquareL(-pb + pt,plam);
          systemEquations[2] =qa - Kspa*signedSquareL(-pa + pp,plam) - \
Ksta*signedSquareL(-pa + pt,plam);
          systemEquations[3] =qb - Kspb*signedSquareL(-pb + pp,plam) - \
Kstb*signedSquareL(-pb + pt,plam);
          systemEquations[4] =qoct - Ksoc*signedSquareL(pocp - poct,plam);
          systemEquations[5] =pp - (cp + qp*Zcp)*onPositive(pp);
          systemEquations[6] =pt - (ct + qt*Zct)*onPositive(pt);
          systemEquations[7] =pa - (ca + qa*Zca)*onPositive(pa);
          systemEquations[8] =pb - (cb + qb*Zcb)*onPositive(pb);
          systemEquations[9] =pocp - (cocp + qocp*Zcocp)*onPositive(pocp);
          systemEquations[10] =poct - (coct + qoct*Zcoct)*onPositive(poct);

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;
          jacobianMatrix[0][1] = 0;
          jacobianMatrix[0][2] = 0;
          jacobianMatrix[0][3] = 0;
          jacobianMatrix[0][4] = 0;
          jacobianMatrix[0][5] = Kspa*dxSignedSquareL(-pa + pp,plam) + \
Kspb*dxSignedSquareL(-pb + pp,plam);
          jacobianMatrix[0][6] = 0;
          jacobianMatrix[0][7] = -(Kspa*dxSignedSquareL(-pa + pp,plam));
          jacobianMatrix[0][8] = -(Kspb*dxSignedSquareL(-pb + pp,plam));
          jacobianMatrix[0][9] = 0;
          jacobianMatrix[0][10] = 0;
          jacobianMatrix[1][0] = 0;
          jacobianMatrix[1][1] = 1;
          jacobianMatrix[1][2] = 0;
          jacobianMatrix[1][3] = 0;
          jacobianMatrix[1][4] = 0;
          jacobianMatrix[1][5] = 0;
          jacobianMatrix[1][6] = Ksta*dxSignedSquareL(-pa + pt,plam) + \
Kstb*dxSignedSquareL(-pb + pt,plam);
          jacobianMatrix[1][7] = -(Ksta*dxSignedSquareL(-pa + pt,plam));
          jacobianMatrix[1][8] = -(Kstb*dxSignedSquareL(-pb + pt,plam));
          jacobianMatrix[1][9] = 0;
          jacobianMatrix[1][10] = 0;
          jacobianMatrix[2][0] = 0;
          jacobianMatrix[2][1] = 0;
          jacobianMatrix[2][2] = 1;
          jacobianMatrix[2][3] = 0;
          jacobianMatrix[2][4] = 0;
          jacobianMatrix[2][5] = -(Kspa*dxSignedSquareL(-pa + pp,plam));
          jacobianMatrix[2][6] = -(Ksta*dxSignedSquareL(-pa + pt,plam));
          jacobianMatrix[2][7] = Kspa*dxSignedSquareL(-pa + pp,plam) + \
Ksta*dxSignedSquareL(-pa + pt,plam);
          jacobianMatrix[2][8] = 0;
          jacobianMatrix[2][9] = 0;
          jacobianMatrix[2][10] = 0;
          jacobianMatrix[3][0] = 0;
          jacobianMatrix[3][1] = 0;
          jacobianMatrix[3][2] = 0;
          jacobianMatrix[3][3] = 1;
          jacobianMatrix[3][4] = 0;
          jacobianMatrix[3][5] = -(Kspb*dxSignedSquareL(-pb + pp,plam));
          jacobianMatrix[3][6] = -(Kstb*dxSignedSquareL(-pb + pt,plam));
          jacobianMatrix[3][7] = 0;
          jacobianMatrix[3][8] = Kspb*dxSignedSquareL(-pb + pp,plam) + \
Kstb*dxSignedSquareL(-pb + pt,plam);
          jacobianMatrix[3][9] = 0;
          jacobianMatrix[3][10] = 0;
          jacobianMatrix[4][0] = 0;
          jacobianMatrix[4][1] = 0;
          jacobianMatrix[4][2] = 0;
          jacobianMatrix[4][3] = 0;
          jacobianMatrix[4][4] = 1;
          jacobianMatrix[4][5] = 0;
          jacobianMatrix[4][6] = 0;
          jacobianMatrix[4][7] = 0;
          jacobianMatrix[4][8] = 0;
          jacobianMatrix[4][9] = -(Ksoc*dxSignedSquareL(pocp - poct,plam));
          jacobianMatrix[4][10] = Ksoc*dxSignedSquareL(pocp - poct,plam);
          jacobianMatrix[5][0] = -(Zcp*onPositive(pp));
          jacobianMatrix[5][1] = 0;
          jacobianMatrix[5][2] = 0;
          jacobianMatrix[5][3] = 0;
          jacobianMatrix[5][4] = 0;
          jacobianMatrix[5][5] = 1;
          jacobianMatrix[5][6] = 0;
          jacobianMatrix[5][7] = 0;
          jacobianMatrix[5][8] = 0;
          jacobianMatrix[5][9] = 0;
          jacobianMatrix[5][10] = 0;
          jacobianMatrix[6][0] = 0;
          jacobianMatrix[6][1] = -(Zct*onPositive(pt));
          jacobianMatrix[6][2] = 0;
          jacobianMatrix[6][3] = 0;
          jacobianMatrix[6][4] = 0;
          jacobianMatrix[6][5] = 0;
          jacobianMatrix[6][6] = 1;
          jacobianMatrix[6][7] = 0;
          jacobianMatrix[6][8] = 0;
          jacobianMatrix[6][9] = 0;
          jacobianMatrix[6][10] = 0;
          jacobianMatrix[7][0] = 0;
          jacobianMatrix[7][1] = 0;
          jacobianMatrix[7][2] = -(Zca*onPositive(pa));
          jacobianMatrix[7][3] = 0;
          jacobianMatrix[7][4] = 0;
          jacobianMatrix[7][5] = 0;
          jacobianMatrix[7][6] = 0;
          jacobianMatrix[7][7] = 1;
          jacobianMatrix[7][8] = 0;
          jacobianMatrix[7][9] = 0;
          jacobianMatrix[7][10] = 0;
          jacobianMatrix[8][0] = 0;
          jacobianMatrix[8][1] = 0;
          jacobianMatrix[8][2] = 0;
          jacobianMatrix[8][3] = -(Zcb*onPositive(pb));
          jacobianMatrix[8][4] = 0;
          jacobianMatrix[8][5] = 0;
          jacobianMatrix[8][6] = 0;
          jacobianMatrix[8][7] = 0;
          jacobianMatrix[8][8] = 1;
          jacobianMatrix[8][9] = 0;
          jacobianMatrix[8][10] = 0;
          jacobianMatrix[9][0] = 0;
          jacobianMatrix[9][1] = 0;
          jacobianMatrix[9][2] = 0;
          jacobianMatrix[9][3] = 0;
          jacobianMatrix[9][4] = 0;
          jacobianMatrix[9][5] = 0;
          jacobianMatrix[9][6] = 0;
          jacobianMatrix[9][7] = 0;
          jacobianMatrix[9][8] = 0;
          jacobianMatrix[9][9] = 1;
          jacobianMatrix[9][10] = 0;
          jacobianMatrix[10][0] = 0;
          jacobianMatrix[10][1] = 0;
          jacobianMatrix[10][2] = 0;
          jacobianMatrix[10][3] = 0;
          jacobianMatrix[10][4] = -(Zcoct*onPositive(poct));
          jacobianMatrix[10][5] = 0;
          jacobianMatrix[10][6] = 0;
          jacobianMatrix[10][7] = 0;
          jacobianMatrix[10][8] = 0;
          jacobianMatrix[10][9] = 0;
          jacobianMatrix[10][10] = 1;
//==This code has been autogenerated using Compgen==

          //Solving equation using LU-faktorisation
          ludcmp(jacobianMatrix, order);
          solvlu(jacobianMatrix,systemEquations,deltaStateVar,order);

        for(i=0;i<11;i++)
          {
          stateVar[i] = stateVark[i] - 
          jsyseqnweight[iter - 1] * deltaStateVar[i];
          }
        for(i=0;i<11;i++)
          {
          stateVark[i] = stateVar[i];
          }
          qp=stateVark[0];
          qt=stateVark[1];
          qa=stateVark[2];
          qb=stateVark[3];
          qoct=stateVark[4];
          pp=stateVark[5];
          pt=stateVark[6];
          pa=stateVark[7];
          pb=stateVark[8];
          pocp=stateVark[9];
          poct=stateVark[10];
          //Expressions
          qocp = -qoct;
        }

        //Calculate the delayed parts

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
        delayedPart[5][1] = delayParts5[1];
        delayedPart[6][1] = delayParts6[1];
        delayedPart[7][1] = delayParts7[1];
        delayedPart[8][1] = delayParts8[1];
        delayedPart[9][1] = delayParts9[1];
        delayedPart[10][1] = delayParts10[1];
        delayedPart[11][1] = delayParts11[1];

        //Write new values to nodes
        //Port Pp
        (*mpND_pp)=pp;
        (*mpND_qp)=qp;
        (*mpND_dEp)=dEp;
        //Port Pt
        (*mpND_pt)=pt;
        (*mpND_qt)=qt;
        (*mpND_dEt)=dEt;
        //Port Pa
        (*mpND_pa)=pa;
        (*mpND_qa)=qa;
        (*mpND_dEa)=dEa;
        //Port Pb
        (*mpND_pb)=pb;
        (*mpND_qb)=qb;
        (*mpND_dEb)=dEb;
        //Port Pocp
        (*mpND_pocp)=pocp;
        (*mpND_qocp)=qocp;
        (*mpND_dEocp)=dEocp;
        //Port Poct
        (*mpND_poct)=poct;
        (*mpND_qoct)=qoct;
        (*mpND_dEoct)=dEoct;
        //outputVariables

        //Update the delayed variabels

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // HYDRAULICVALVE63OC_HPP_INCLUDED
