//!
//! @file   HydraulicPressureReducingValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-01-26
//!
//! @brief Contains a hydraulic pressure relief valve with first order dynamics

//$Id: HydraulicPressureReducingValve.hpp 2503 2011-01-24 07:48:09Z robbr48 $

#ifndef HYDRAULICPRESSUREREDUCINGVALVE_HPP_INCLUDED
#define HYDRAULICPRESSUREREDUCINGVALVE_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief A Hydraulic Pressure Releife Valve
    //! @ingroup HydraulicComponents
    //!
    class HydraulicPressureReducingValve : public ComponentQ
    {
    private:
        double x0, pref, tao, Kcs, Kcf, Cs, Cf, qnom, pnom, ph, x0max;
        double mPrevX0;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2;

        TurbulentFlowFunction mTurb;
        ValveHysteresis mHyst;
        FirstOrderFilter mFilterLP;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new HydraulicPressureReducingValve("PressureReducingValve");
        }

        HydraulicPressureReducingValve(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicPressureReducingValve";
            pref = 2000000;
            tao = 0.01;
            Kcs = 0.00000001;
            Kcf = 0.00000001;
            qnom = 0.001;
            ph = 500000;
            pnom = 7e6f;
            x0max = qnom / sqrt(pnom);
            Cs = sqrt(pnom) / Kcs;
            Cf = 1.0 / (Kcf*sqrt(pnom));

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            registerParameter("pref", "Reference Opening Pressure", "[Pa]", pref);
            registerParameter("tao", "Time Constant of Spool", "[s]", tao);
            registerParameter("kcs", "Steady State Characteristic due to Spring", "[(m^3/s)/Pa]", Kcs);
            registerParameter("kcf", "Steady State Characteristic due to Flow Forces", "[(m^3/s)/Pa]", Kcf);
            registerParameter("qnom", "Flow with Fully Open Valve and pressure drop Pnom", "[m^3/s]", qnom);
            registerParameter("ph", "Hysteresis Width", "[Pa]", ph);
        }


        void initialize()
        {
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);

            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::PRESSURE);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::FLOW);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);

            x0 = 0.00001;

            mPrevX0 = 0;

            double wCutoff = 1 / tao;
            double num [2] = {0.0, 1.0};
            double den [2] = {1.0/wCutoff, 1.0};
            mFilterLP.initialize(mTimestep, num, den, 0.0, 0.0, 0.0, x0max);
        }


        void simulateOneTimestep()
        {
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2;
            double b1, gamma, b2, xs, xh, xsh, wCutoff;
            bool cav = false;

            //Get variable values from nodes
            p1 = (*mpND_p1);
            q1 = (*mpND_q1);
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);
            p2 = (*mpND_p2);
            q2 = (*mpND_q2);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);

            //PRV Equations

            //Help variable b1
            b1 = Cs + (p1-p2)*Cf;
            if ( (p1-p2)*Cf < 0.0 )
            {
                b1 = Cs;
            }

            //Help variable gamma
            gamma;
            if (p1>p2)
            {
                if ( (sqrt(p1-p2)*2.0 + (Zc1+Zc2)*x0) != 0.0 )
                {
                    gamma = sqrt(p1-p2)*2.0 / (sqrt(p1-p2)*2.0 + (Zc1+Zc2)*x0);
                }
                else
                {
                    gamma = 1.0;
                }
            }
            else
            {
                if ( (sqrt(p2-p1)*2.0 + (Zc1+Zc2)*x0) != 0.0 )
                {
                    gamma = sqrt(p2-p1)*2.0 / (sqrt(p2-p1)*2.0 + (Zc1+Zc2)*x0);
                }
                else
                {
                    gamma = 1.0;
                }
            }

            //Help variable b2
            b2;
            if (p1 > p2)
            {
                b2 = gamma*(Zc1+Zc2)*sqrt(p1-p2);
            }
            else
            {
                b2 = gamma*(Zc1+Zc2)*sqrt(p2-p1);
            }
            if (b2 < 0.0)
            {
                b2 = 0.0;
            }

            // Calculation of spool position
            xs = (gamma*(-c2) + b2*x0/2.0 + pref) / (b1+b2);

            //Hysteresis
            xh = ph / (b1+b2);                                  //Hysteresis width [m]
            xsh = mHyst.getValue(xs, xh, mPrevX0);

            //Filter
            wCutoff = (1.0 + b2/b1) * 1.0/tao;                //Cutoff frequency
            double num [2] = {0.0, 1.0};
            double den [2] = {1.0/wCutoff, 1.0};
            mFilterLP.setNumDen(num,den);
            mFilterLP.update(xsh);
            x0 = mFilterLP.value();

            //Turbulent flow equation
            mTurb.setFlowCoefficient(x0);
            q2 = mTurb.getFlow(c1,c2,Zc1,Zc2);
            q1 = -q2;
            p2 = c2+Zc2*q2;
            p1 = c1+Zc1*q1;

            // Cavitation
            if (p1 < 0.0)
            {
                c1 = 0.0;
                Zc1 = 0.0;
                cav = true;
            }
            if (p2 < 0.0)
            {
                c2 = 0.0;
                Zc2 = 0.0;
                cav = true;
            }
            if (cav)
            {
                xs = ((-c2) + b2*x0/2.0 + pref) / (b1+b2);
                xsh = mHyst.getValue(xs, xh, mPrevX0);
                x0 = mFilterLP.value();        //! @todo How is this supposed to work? It is not possible to use the same filter twice in the same timesteps with different input values...

                mTurb.setFlowCoefficient(x0);
                q2 = mTurb.getFlow(c1,c2,Zc1,Zc2);
                q1 = -q2;
                p2 = c2+Zc2*q2;
                p1 = c1+Zc1*q1;

                if (p1 < 0.0) { p1 = 0.0; }
                if (p2 < 0.0) { p2 = 0.0; }
            }

            mPrevX0 = x0;

            //Write new variables to nodes
            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
        }
    };
}

#endif // HYDRAULICPRESSUREREDUCINGVALVE_HPP_INCLUDED
