/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/
#pragma once

#ifndef _PUBLIC_H
#define _PUBLIC_H

#include <initguid.h>

//
// Define an Interface Guid so that app can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_LearningKitKMDF,
    0x8dd29177,0x898e,0x47cd,0xb4,0x5c,0x87,0xd7,0xe7,0xee,0x52,0x68);
// {8dd29177-898e-47cd-b45c-87d7e7ee5268}


typedef struct _SWITCH_STATE {

	union {
		struct {
			//
			// Individual switches starting from the 
			//  left of the set of switches
			//
			UCHAR Switch1 : 1;
			UCHAR Switch2 : 1;
			UCHAR Switch3 : 1;
			UCHAR Switch4 : 1;
			UCHAR Switch5 : 1;
			UCHAR Switch6 : 1;
			UCHAR Switch7 : 1;
			UCHAR Switch8 : 1;
		};

		//
		// The state of all the switches as a single
		// UCHAR
		//
		UCHAR SwitchesAsUChar;

	};


}SWITCH_STATE, *PSWITCH_STATE;

typedef struct _BAR_GRAPH_STATE {

	union {

		struct {
			//
			// Individual bars starting from the 
			//  top of the stack of bars 
			//
			// NOTE: There are actually 10 bars, 
			//  but the very top two do not light
			//  and are not counted here
			//
			UCHAR Bar1 : 1;
			UCHAR Bar2 : 1;
			UCHAR Bar3 : 1;
			UCHAR Bar4 : 1;
			UCHAR Bar5 : 1;
			UCHAR Bar6 : 1;
			UCHAR Bar7 : 1;
			UCHAR Bar8 : 1;
		};

		//
		// The state of all the bar graph as a single
		// UCHAR
		//
		UCHAR BarsAsUChar;

	};

}BAR_GRAPH_STATE, *PBAR_GRAPH_STATE;

#define IOCTL_INDEX             0x800
#define FILE_DEVICE_OSRUSBFX2   65500U

#define IOCTL_OSRUSBFX2_GET_CONFIG_DESCRIPTOR CTL_CODE(FILE_DEVICE_OSRUSBFX2,     \
                                                     IOCTL_INDEX,     \
                                                     METHOD_BUFFERED,         \
                                                     FILE_READ_ACCESS)

#define IOCTL_OSRUSBFX2_RESET_DEVICE  CTL_CODE(FILE_DEVICE_OSRUSBFX2,     \
                                                     IOCTL_INDEX + 1, \
                                                     METHOD_BUFFERED,         \
                                                     FILE_WRITE_ACCESS)

#define IOCTL_OSRUSBFX2_REENUMERATE_DEVICE  CTL_CODE(FILE_DEVICE_OSRUSBFX2, \
                                                    IOCTL_INDEX  + 3,  \
                                                    METHOD_BUFFERED, \
                                                    FILE_WRITE_ACCESS)

#define IOCTL_OSRUSBFX2_GET_BAR_GRAPH_DISPLAY CTL_CODE(FILE_DEVICE_OSRUSBFX2,\
                                                    IOCTL_INDEX  + 4, \
                                                    METHOD_BUFFERED, \
                                                    FILE_READ_ACCESS)


#define IOCTL_OSRUSBFX2_SET_BAR_GRAPH_DISPLAY CTL_CODE(FILE_DEVICE_OSRUSBFX2,\
                                                    IOCTL_INDEX + 5, \
                                                    METHOD_BUFFERED, \
                                                    FILE_WRITE_ACCESS)


#define IOCTL_OSRUSBFX2_READ_SWITCHES   CTL_CODE(FILE_DEVICE_OSRUSBFX2, \
                                                    IOCTL_INDEX + 6, \
                                                    METHOD_BUFFERED, \
                                                    FILE_READ_ACCESS)


#define IOCTL_OSRUSBFX2_GET_7_SEGMENT_DISPLAY CTL_CODE(FILE_DEVICE_OSRUSBFX2, \
                                                    IOCTL_INDEX + 7, \
                                                    METHOD_BUFFERED, \
                                                    FILE_READ_ACCESS)


#define IOCTL_OSRUSBFX2_SET_7_SEGMENT_DISPLAY CTL_CODE(FILE_DEVICE_OSRUSBFX2, \
                                                    IOCTL_INDEX + 8, \
                                                    METHOD_BUFFERED, \
                                                    FILE_WRITE_ACCESS)

#define IOCTL_OSRUSBFX2_GET_INTERRUPT_MESSAGE CTL_CODE(FILE_DEVICE_OSRUSBFX2,\
                                                    IOCTL_INDEX + 9, \
                                                    METHOD_OUT_DIRECT, \
                                                    FILE_READ_ACCESS)
#endif