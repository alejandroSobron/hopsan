/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   SignalGain.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-05
//!
//! @brief Contains a Signal Gain Component
//!
//$Id$

#ifndef SIGNALDUMMY_HPP_INCLUDED
#define SIGNALDUMMY_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalDummy : public ComponentSignal
    {

    private:
        double *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalDummy();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpIn);
            addOutputVariable("out", "", "", &mpOut);
        }

        void initialize() {}

        void simulateOneTimestep()
        {
            (*mpOut) = 1;
            for(int i=0; i<(*mpIn); ++i)
            {
                (*mpOut) = (*mpOut) * i;
            }
        }
    };
}

#endif // SIGNALDUMMY_HPP_INCLUDED
