//$Id$

#ifndef HYDRAULICLAMINARORIFICE_HPP_INCLUDED
#define HYDRAULICLAMINARORIFICE_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief A hydraulic laminar orifice component
    //! @ingroup HydraulicComponents
    //!
    class HydraulicLaminarOrifice : public ComponentQ
    {
    private:
        double Kc;
        bool cav;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_Kc;

        Port *mpP1, *mpP2, *mpIn;

    public:
        static Component *Creator()
        {
            return new HydraulicLaminarOrifice("LaminarOrifice");
        }

        HydraulicLaminarOrifice(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicLaminarOrifice";
            Kc = 1.0e-11;

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpIn = addReadPort("Kc", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("Kc", "Pressure-Flow Coefficient", "[m^5/Ns]", Kc);
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

            mpND_Kc = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, Kc);
        }


        void simulateOneTimestep()
        {
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2;

            //Get variable values from nodes
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);
            Kc = (*mpND_Kc);

            //Orifice equations
            q2 = Kc*(c1-c2)/(1.0+Kc*(Zc1+Zc2));
            q1 = -q2;
            p1 = c1 + q1*Zc1;
            p2 = c2 + q2*Zc2;

            //Cavitation check
            cav = false;
            if(p1 < 0.0)
            {
                c1 = 0.0;
                Zc1 = 0.0;
                cav = true;
            }
            if(p2 < 0.0)
            {
                c2 = 0.0;
                Zc2 = 0.0;
                cav = true;
            }
            if(cav)
            {
                q2 = Kc*(c1-c2)/(1.0+Kc*(Zc1+Zc2));
                q1 = -q2;
                p1 = c1 + q1*Zc1;
                p2 = c2 + q2*Zc2;
                if(p1 < 0.0) { p1 = 0.0; }
                if(p2 < 0.0) { p2 = 0.0; }
            }

            //Write new variables to nodes
            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
        }
    };
}

#endif // HYDRAULICLAMINARORIFICE_HPP_INCLUDED
