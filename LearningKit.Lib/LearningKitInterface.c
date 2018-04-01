#include "LearningKitInterface.h"


extern HANDLE OpenDevice(_In_ BOOL Synchronous);

_Bool DisplaySymbol(KIT_SYMBOL symbol)
{
	HANDLE deviceHandle;
	DWORD code;
	ULONG returnedBytes;

	deviceHandle = OpenDevice(FALSE);
	if (deviceHandle == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	UCHAR sevenSegmentState = symbol;

	_Bool result = DeviceIoControl(deviceHandle,
		IOCTL_OSRUSBFX2_SET_7_SEGMENT_DISPLAY,
		&sevenSegmentState,   // Ptr to InBuffer
		sizeof(UCHAR),  // Length of InBuffer
		NULL,           // Ptr to OutBuffer
		0,         // Length of OutBuffer
		&returnedBytes,         // BytesReturned
		0);

	CloseHandle(deviceHandle);

	return result;
}

KIT_SYMBOL GetDisplayedSymbol()
{
	HANDLE deviceHandle;
	DWORD code;
	ULONG returnedBytes;

	deviceHandle = OpenDevice(FALSE);
	if (deviceHandle == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	UCHAR sevenSegment = 0;

	_Bool result = DeviceIoControl(deviceHandle,
		IOCTL_OSRUSBFX2_GET_7_SEGMENT_DISPLAY,
		NULL,             // Ptr to InBuffer
		0,            // Length of InBuffer
		&sevenSegment,                 // Ptr to OutBuffer
		sizeof(UCHAR),         // Length of OutBuffer
		&returnedBytes,                     // BytesReturned
		0);                      // Ptr to Overlapped structure

	CloseHandle(deviceHandle);

	return result ? (KIT_SYMBOL)sevenSegment : Undefined;
}

_Bool LightBars(KIT_BAR bar)
{
	HANDLE deviceHandle;
	ULONG returnedBytes;

	deviceHandle = OpenDevice(FALSE);
	if (deviceHandle == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	UCHAR barGraphState = bar;

	_Bool result = DeviceIoControl(deviceHandle,
		IOCTL_OSRUSBFX2_SET_BAR_GRAPH_DISPLAY,
		&barGraphState,         // Ptr to InBuffer
		sizeof(BAR_GRAPH_STATE),   // Length of InBuffer
		NULL,         // Ptr to OutBuffer
		0,            // Length of OutBuffer
		&returnedBytes,       // BytesReturned
		0);

	CloseHandle(deviceHandle);

	return result;
}

KIT_BAR GetLightBars()
{
	HANDLE deviceHandle;
	ULONG returnedBytes;

	deviceHandle = OpenDevice(FALSE);
	if (deviceHandle == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	UCHAR bars = 0;

	_Bool result = DeviceIoControl(deviceHandle,
		IOCTL_OSRUSBFX2_GET_BAR_GRAPH_DISPLAY,
		NULL,             // Ptr to InBuffer
		0,            // Length of InBuffer
		&bars,          // Ptr to OutBuffer
		sizeof(BAR_GRAPH_STATE), // Length of OutBuffer
		&returnedBytes,                  // BytesReturned
		0);                   // Ptr to Overlapped structure

	CloseHandle(deviceHandle);

	return result ? (KIT_BAR)bars : Undefined;
}

KIT_SWITCHES GetSwitches()
{
	HANDLE deviceHandle;
	ULONG returnedBytes;

	deviceHandle = OpenDevice(FALSE);
	if (deviceHandle == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	UCHAR switchState = 0;

	_Bool result = DeviceIoControl(deviceHandle,
		IOCTL_OSRUSBFX2_READ_SWITCHES,
		NULL,             // Ptr to InBuffer
		0,            // Length of InBuffer
		&switchState,           // Ptr to OutBuffer
		sizeof(switchState),    // Length of OutBuffer
		&returnedBytes,                // BytesReturned
		0);

	CloseHandle(deviceHandle);

	return (KIT_SWITCHES)switchState;
}

KIT_SWITCHES GetSwitchesInterrupt()
{
	HANDLE deviceHandle;
	ULONG returnedBytes;

	deviceHandle = OpenDevice(FALSE);
	if (deviceHandle == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	UCHAR switchState = 0;

	_Bool result = DeviceIoControl(deviceHandle,
		IOCTL_OSRUSBFX2_GET_INTERRUPT_MESSAGE,
		NULL,             // Ptr to InBuffer
		0,            // Length of InBuffer
		&switchState,           // Ptr to OutBuffer
		sizeof(switchState),    // Length of OutBuffer
		&returnedBytes,                // BytesReturned
		0);

	CloseHandle(deviceHandle);

	return (KIT_SWITCHES)switchState;
}
