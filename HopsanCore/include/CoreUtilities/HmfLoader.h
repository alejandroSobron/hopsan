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
//! @file   HmfLoader.cc
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-03-20
//!
//! @brief Contains the HopsanCore HMF file load function
//!
//$Id$

#ifndef HMFLOADER_H_INCLUDED
#define HMFLOADER_H_INCLUDED


#include <vector>
#include <ctype.h>
#include "HopsanTypes.h"
#include "win32dll.h"

namespace hopsan {

//Forward declaration
class ComponentSystem;
class HopsanEssentials;
class HopsanCoreMessageHandler;

int DLLIMPORTEXPORT getEpochVersion(const HString& version);
int DLLIMPORTEXPORT getMajorVersion(const HString& version);
int DLLIMPORTEXPORT getMinorVersion(const HString& version);
bool DLLIMPORTEXPORT isVersionAGreaterThanB(const HString& versionA, const HString& versionB);
int DLLIMPORTEXPORT compareHopsanVersions(const HString& versionA, const HString& versionB);

ComponentSystem* loadHopsanModelFile(const HString &rFilePath, HopsanEssentials* pHopsanEssentials, double &rStartTime, double &rStopTime);
ComponentSystem* loadHopsanModel(const std::vector<unsigned char> xmlVector, HopsanEssentials* pHopsanEssentials);
ComponentSystem* loadHopsanModel(const char* xmlStr, HopsanEssentials* pHopsanEssentials, double &rStartTime, double &rStopTime);
ComponentSystem* loadHopsanModel(char* xmlStr, HopsanEssentials* pHopsanEssentials, double &rStartTime, double &rStopTime);
void loadHopsanParameterFile(const HString &rFilePath, HopsanEssentials* pHopsanEssentials, ComponentSystem *pSystem);

}

#endif

