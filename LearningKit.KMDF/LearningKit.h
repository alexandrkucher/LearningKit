
#include <initguid.h>
#include <ntddk.h>
#include "usbdi.h"
#include "usbdlib.h"
#include "public.h"
#include "driverspecs.h"
#include <wdf.h>
#include <wdfusb.h>

extern const __declspec(selectany) LONGLONG DEFAULT_CONTROL_TRANSFER_TIMEOUT = 5 * -1 * WDF_TIMEOUT_TO_SEC;

//
// Define the vendor commands supported by our device
//
#define USBFX2LK_READ_7SEGMENT_DISPLAY      0xD4
#define USBFX2LK_READ_SWITCHES              0xD6
#define USBFX2LK_READ_BARGRAPH_DISPLAY      0xD7
#define USBFX2LK_SET_BARGRAPH_DISPLAY       0xD8
#define USBFX2LK_IS_HIGH_SPEED              0xD9
#define USBFX2LK_REENUMERATE                0xDA
#define USBFX2LK_SET_7SEGMENT_DISPLAY       0xDB

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

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD LearningKitKMDFEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP LearningKitKMDFEvtDriverContextCleanup;

EVT_WDF_DEVICE_D0_ENTRY LearningKitKMDFEvtDeviceD0Entry;
EVT_WDF_DEVICE_D0_EXIT LearningKitKMDFEvtDeviceD0Exit;
EVT_WDF_DRIVER_DEVICE_ADD LearningKitKMDFEvtDeviceAdd;
EVT_WDF_DEVICE_PREPARE_HARDWARE LearningKitKMDFEvtDevicePrepareHardware;

EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL LearningKitKMDFEvtIoDeviceControl;
EVT_WDF_IO_QUEUE_IO_STOP LearningKitKMDFEvtIoStop;

NTSTATUS
LearningKitKMDFQueueInitialize(
	_In_ WDFDEVICE Device
);

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
LearningKitSetPowerPolicy(
	_In_ WDFDEVICE Device
);

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
SelectInterfaces(
	_In_ WDFDEVICE Device
);

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
LearningKitConfigContReaderForInterruptEndPoint(
	_In_ PDEVICE_CONTEXT DeviceContext
);

EVT_WDF_USB_READER_COMPLETION_ROUTINE LearningKitEvtUsbInterruptPipeReadComplete;
EVT_WDF_USB_READERS_FAILED OsrFxEvtUsbInterruptReadersFailed;

VOID
LearningKitIoctlGetInterruptMessage(
	_In_ WDFDEVICE Device,
	_In_ NTSTATUS ReaderStatus
);

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
ResetDevice(
	_In_ WDFDEVICE Device
);

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
ReenumerateDevice(
	_In_ PDEVICE_CONTEXT DevContext
);

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
GetBarGraphState(
	_In_ PDEVICE_CONTEXT DevContext,
	_Out_ PBAR_GRAPH_STATE BarGraphState
);

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
SetBarGraphState(
	_In_ PDEVICE_CONTEXT DevContext,
	_In_ PBAR_GRAPH_STATE BarGraphState
);

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
GetSevenSegmentState(
	_In_ PDEVICE_CONTEXT DevContext,
	_Out_ PUCHAR SevenSegment
);

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
SetSevenSegmentState(
	_In_ PDEVICE_CONTEXT DevContext,
	_In_ PUCHAR SevenSegment
);

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
GetSwitchState(
	_In_ PDEVICE_CONTEXT DevContext,
	_In_ PSWITCH_STATE SwitchState
);

VOID
StopAllPipes(
	IN PDEVICE_CONTEXT DeviceContext
);

NTSTATUS
StartAllPipes(
	IN PDEVICE_CONTEXT DeviceContext
);

EXTERN_C_END

FORCEINLINE
GUID
DeviceToActivityId(
	_In_ WDFDEVICE Device
)
{
	GUID activity = { 0 };
	RtlCopyMemory(&activity, &Device, sizeof(WDFDEVICE));
	return activity;
}