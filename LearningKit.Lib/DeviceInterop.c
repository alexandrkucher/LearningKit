#include "LearningKitInterface.h"
#include <windows.h>
#include <stdio.h>
#include "strsafe.h"
#include <cfgmgr32.h>

#include "Public.h"

#define MAX_DEVPATH_LENGTH 256

typedef enum _INPUT_FUNCTION {
	LIGHT_ONE_BAR = 1,
	CLEAR_ONE_BAR,
	LIGHT_ALL_BARS,
	CLEAR_ALL_BARS,
	GET_BAR_GRAPH_LIGHT_STATE,
	GET_SWITCH_STATE,
	GET_SWITCH_STATE_AS_INTERRUPT_MESSAGE,
	GET_7_SEGEMENT_STATE,
	SET_7_SEGEMENT_STATE,
	RESET_DEVICE,
	REENUMERATE_DEVICE,
} INPUT_FUNCTION;


_Success_(return)
BOOL
GetDevicePath(
	_In_  LPGUID InterfaceGuid,
	_Out_writes_z_(BufLen) PWCHAR DevicePath,
	_In_ size_t BufLen
)
{
	CONFIGRET cr = CR_SUCCESS;
	PWSTR deviceInterfaceList = NULL;
	ULONG deviceInterfaceListLength = 0;
	PWSTR nextInterface;
	HRESULT hr = E_FAIL;
	BOOL bRet = TRUE;

	cr = CM_Get_Device_Interface_List_Size(
		&deviceInterfaceListLength,
		InterfaceGuid,
		NULL,
		CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
	if (cr != CR_SUCCESS) {
		printf("Error 0x%x retrieving device interface list size.\n", cr);
		goto clean0;
	}

	if (deviceInterfaceListLength <= 1) {
		bRet = FALSE;
		printf("Error: No active device interfaces found.\n"
			" Is the sample driver loaded?");
		goto clean0;
	}

	deviceInterfaceList = (PWSTR)malloc(deviceInterfaceListLength * sizeof(WCHAR));
	if (deviceInterfaceList == NULL) {
		bRet = FALSE;
		printf("Error allocating memory for device interface list.\n");
		goto clean0;
	}
	ZeroMemory(deviceInterfaceList, deviceInterfaceListLength * sizeof(WCHAR));

	cr = CM_Get_Device_Interface_List(
		InterfaceGuid,
		NULL,
		deviceInterfaceList,
		deviceInterfaceListLength,
		CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
	if (cr != CR_SUCCESS) {
		printf("Error 0x%x retrieving device interface list.\n", cr);
		goto clean0;
	}

	nextInterface = deviceInterfaceList + wcslen(deviceInterfaceList) + 1;
	if (*nextInterface != UNICODE_NULL) {
		printf("Warning: More than one device interface instance found. \n"
			"Selecting first matching device.\n\n");
	}

	hr = StringCchCopy(DevicePath, BufLen, deviceInterfaceList);
	if (FAILED(hr)) {
		bRet = FALSE;
		printf("Error: StringCchCopy failed with HRESULT 0x%x", hr);
		goto clean0;
	}

clean0:
	if (deviceInterfaceList != NULL) {
		free(deviceInterfaceList);
	}
	if (CR_SUCCESS != cr) {
		bRet = FALSE;
	}

	return bRet;
}

_Check_return_
_Ret_notnull_
_Success_(return != INVALID_HANDLE_VALUE)
HANDLE
OpenDevice(
	_In_ BOOL Synchronous
)

/*++
Routine Description:

Called by main() to open an instance of our device after obtaining its name

Arguments:

Synchronous - TRUE, if Device is to be opened for synchronous access.
FALSE, otherwise.

Return Value:

Device handle on success else INVALID_HANDLE_VALUE

--*/

{
	HANDLE hDev;
	WCHAR completeDeviceName[MAX_DEVPATH_LENGTH];

	if (!GetDevicePath(
		(LPGUID)&GUID_DEVINTERFACE_LearningKitKMDF,
		completeDeviceName,
		sizeof(completeDeviceName) / sizeof(completeDeviceName[0])))
	{
		return  INVALID_HANDLE_VALUE;
	}

	printf("DeviceName = (%S)\n", completeDeviceName);

	if (Synchronous) {
		hDev = CreateFile(completeDeviceName,
			GENERIC_WRITE | GENERIC_READ,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			NULL, // default security
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	}
	else {

		hDev = CreateFile(completeDeviceName,
			GENERIC_WRITE | GENERIC_READ,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			NULL, // default security
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			NULL);
	}

	if (hDev == INVALID_HANDLE_VALUE) {
		printf("Failed to open the device, error - %d", GetLastError());
	}
	else {
		printf("Opened the device successfully.\n");
	}

	return hDev;
}

BOOL
PlayWithDevice()
{
	HANDLE          deviceHandle;
	DWORD           code;
	ULONG           index;
	INPUT_FUNCTION  function;
	BAR_GRAPH_STATE barGraphState;
	ULONG           bar;
	SWITCH_STATE    switchState;
	UCHAR           sevenSegment = 0;
	UCHAR           i;
	BOOL            result = FALSE;

	deviceHandle = OpenDevice(FALSE);

	if (deviceHandle == INVALID_HANDLE_VALUE) {

		printf("Unable to find any OSR FX2 devices!\n");

		return FALSE;

	}

	//
	// Infinitely print out the list of choices, ask for input, process
	// the request
	//
	while (TRUE) {

		INPUT_FUNCTION function = LIGHT_ONE_BAR;
		switch (function) {

		case GET_SWITCH_STATE:

			switchState.SwitchesAsUChar = 0;

			if (!DeviceIoControl(deviceHandle,
				IOCTL_OSRUSBFX2_READ_SWITCHES,
				NULL,             // Ptr to InBuffer
				0,            // Length of InBuffer
				&switchState,          // Ptr to OutBuffer
				sizeof(SWITCH_STATE),  // Length of OutBuffer
				&index,                // BytesReturned
				0)) {                  // Ptr to Overlapped structure

				code = GetLastError();

				printf("DeviceIoControl failed with error 0x%x\n", code);

				goto Error;
			}

			printf("Switches: \n");
			printf("    Switch8 is %s\n", switchState.Switch8 ? "ON" : "OFF");
			printf("    Switch7 is %s\n", switchState.Switch7 ? "ON" : "OFF");
			printf("    Switch6 is %s\n", switchState.Switch6 ? "ON" : "OFF");
			printf("    Switch5 is %s\n", switchState.Switch5 ? "ON" : "OFF");
			printf("    Switch4 is %s\n", switchState.Switch4 ? "ON" : "OFF");
			printf("    Switch3 is %s\n", switchState.Switch3 ? "ON" : "OFF");
			printf("    Switch2 is %s\n", switchState.Switch2 ? "ON" : "OFF");
			printf("    Switch1 is %s\n", switchState.Switch1 ? "ON" : "OFF");

			break;

		case GET_SWITCH_STATE_AS_INTERRUPT_MESSAGE:

			switchState.SwitchesAsUChar = 0;

			if (!DeviceIoControl(deviceHandle,
				IOCTL_OSRUSBFX2_GET_INTERRUPT_MESSAGE,
				NULL,             // Ptr to InBuffer
				0,            // Length of InBuffer
				&switchState,           // Ptr to OutBuffer
				sizeof(switchState),    // Length of OutBuffer
				&index,                // BytesReturned
				0)) {                  // Ptr to Overlapped structure

				code = GetLastError();

				printf("DeviceIoControl failed with error 0x%x\n", code);

				goto Error;
			}

			printf("Switches: %d\n", index);
			printf("    Switch8 is %s\n", switchState.Switch8 ? "ON" : "OFF");
			printf("    Switch7 is %s\n", switchState.Switch7 ? "ON" : "OFF");
			printf("    Switch6 is %s\n", switchState.Switch6 ? "ON" : "OFF");
			printf("    Switch5 is %s\n", switchState.Switch5 ? "ON" : "OFF");
			printf("    Switch4 is %s\n", switchState.Switch4 ? "ON" : "OFF");
			printf("    Switch3 is %s\n", switchState.Switch3 ? "ON" : "OFF");
			printf("    Switch2 is %s\n", switchState.Switch2 ? "ON" : "OFF");
			printf("    Switch1 is %s\n", switchState.Switch1 ? "ON" : "OFF");

			break;

		case RESET_DEVICE:

			printf("Reset the device\n");

			if (!DeviceIoControl(deviceHandle,
				IOCTL_OSRUSBFX2_RESET_DEVICE,
				NULL,             // Ptr to InBuffer
				0,            // Length of InBuffer
				NULL,                 // Ptr to OutBuffer
				0,         // Length of OutBuffer
				&index,   // BytesReturned
				NULL)) {        // Ptr to Overlapped structure

				code = GetLastError();

				printf("DeviceIoControl failed with error 0x%x\n", code);

				goto Error;
			}

			break;

		case REENUMERATE_DEVICE:

			printf("Re-enumerate the device\n");

			if (!DeviceIoControl(deviceHandle,
				IOCTL_OSRUSBFX2_REENUMERATE_DEVICE,
				NULL,             // Ptr to InBuffer
				0,            // Length of InBuffer
				NULL,                 // Ptr to OutBuffer
				0,         // Length of OutBuffer
				&index,   // BytesReturned
				NULL)) {        // Ptr to Overlapped structure

				code = GetLastError();

				printf("DeviceIoControl failed with error 0x%x\n", code);

				goto Error;
			}

			//
			// Close the handle to the device and exit out so that
			// the driver can unload when the device is surprise-removed
			// and reenumerated.
			//
		default:

			result = TRUE;
			goto Error;

		}

	}   // end of while loop

Error:

	CloseHandle(deviceHandle);
	return result;

}