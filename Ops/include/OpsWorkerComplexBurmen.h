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
//! @file   OpsWorkerComplexBurmen.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2016-01-08
//!
//! @brief Contains the optimization worker class for the Complex-RF algorithm by Bürmen
//!
//$Id$

#ifndef WORKERCOMPLEXBURMEN_H
#define WORKERCOMPLEXBURMEN_H

#include "OpsWorkerComplexRF.h"

namespace Ops {

class OPS_DLLAPI WorkerComplexBurmen : public WorkerComplexRF
{
public:
    WorkerComplexBurmen(Evaluator *pEvaluator, MessageHandler *pMessageHandler);
    AlgorithmT getAlgorithm();

    virtual void initialize();
    virtual void run();
};

}

#endif // WORKERCOMPLEXBURMEN_H
