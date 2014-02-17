/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   version_gui.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains version definitions for the HopsanGUI, HMF files and component appearance files
//!
//$Id$
#ifndef VERSION_GUI_H
#define VERSION_GUI_H

// Stringify Macro
#ifndef TO_STR_2
 #define TO_STR_2(s) #s
 #define TO_STR(s) TO_STR_2(s)
#endif

// If we dont have the revision number then define UNKNOWN
// On real relase  builds, UNKNOWN will be replaced by actual revnum by external script
#ifndef HOPSANGUISVNREVISION
 #define HOPSANGUISVNREVISION UNKNOWN
#endif

#define HOPSANGUIVERSION "0.6.x_r" TO_STR(HOPSANGUISVNREVISION)
#define HMF_VERSIONNUM "0.4"
#define HMF_REQUIREDVERSIONNUM "0.3"
#define CAF_VERSIONNUM "0.3"

// Include compiler info macros
#include "compiler_info.h"

#endif // VERSION_GUI_H