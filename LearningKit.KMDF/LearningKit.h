#pragma once


#include <ntddk.h>
#include <wdf.h>
#include <wdfusb.h>

typedef struct _DEVICE_CONTEXT {

	WDFUSBDEVICE                    UsbDevice;
	WDFUSBINTERFACE                 UsbInterface;
	WDFUSBPIPE                      InterruptPipe;
	WDFWAITLOCK                     ResetDeviceWaitLock;
	UCHAR                           CurrentSwitchState;
	WDFQUEUE                        InterruptMsgQueue;
	ULONG                           UsbDeviceTraits;

	//
	// The following fields are used during event logging to 
	// report the events relative to this specific instance 
	// of the device.
	//

	WDFMEMORY                       DeviceNameMemory;
	PCWSTR                          DeviceName;

	WDFMEMORY                       LocationMemory;
	PCWSTR                          Location;

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

//
// This macro will generate an inline function called DeviceGetContext
// which will be used to get a pointer to the device context memory
// in a type safe manner.
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, GetDeviceContext)

EXTERN_C_START
EVT_WDF_DEVICE_D0_ENTRY LearningKitKMDFEvtDeviceD0Entry;
EVT_WDF_DEVICE_D0_EXIT LearningKitKMDFEvtDeviceD0Exit;
EVT_WDF_DRIVER_DEVICE_ADD LearningKitKMDFEvtDeviceAdd;
EVT_WDF_DEVICE_PREPARE_HARDWARE LearningKitKMDFEvtDevicePrepareHardware;
EXTERN_C_END