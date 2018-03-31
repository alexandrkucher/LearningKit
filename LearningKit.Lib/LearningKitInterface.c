#include "LearningKitInterface.h"


extern HANDLE OpenDevice(_In_ BOOL Synchronous);

_Bool DisplaySymbol(UCHAR symbol)
{
	HANDLE deviceHandle;
	DWORD code;
	ULONG returnedBytes;

	deviceHandle = OpenDevice(FALSE);
	if (deviceHandle == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	UCHAR map[10] = { 0xD7, 0x6, 0xB3, 0xA7,  0x66, 0xE5, 0xF5, 0x07, 0xF7, 0xE7 };
	UCHAR sevenSegmentState = map[symbol];

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
