/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

#ifndef _PUBLIC_H
#define _PUBLIC_H

#include <initguid.h>

//
// Define an Interface Guid so that app can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_LearningKitKMDF,
    0x8dd29177,0x898e,0x47cd,0xb4,0x5c,0x87,0xd7,0xe7,0xee,0x52,0x68);
// {8dd29177-898e-47cd-b45c-87d7e7ee5268}

#endif