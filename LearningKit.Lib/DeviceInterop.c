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

		printf("\nUSBFX TEST -- Functions:\n\n");
		printf("\t1.  Light Bar\n");
		printf("\t2.  Clear Bar\n");
		printf("\t3.  Light entire Bar graph\n");
		printf("\t4.  Clear entire Bar graph\n");
		printf("\t5.  Get bar graph state\n");
		printf("\t6.  Get Switch state\n");
		printf("\t7.  Get Switch Interrupt Message\n");
		printf("\t8.  Get 7 segment state\n");
		printf("\t9.  Set 7 segment state\n");
		printf("\t10. Reset the device\n");
		printf("\t11. Reenumerate the device\n");
		printf("\n\t0. Exit\n");
		printf("\n\tSelection: ");

		if (scanf_s("%d", &function) <= 0) {

			printf("Error reading input!\n");
			goto Error;

		}

		switch (function) {

		case LIGHT_ONE_BAR:

			printf("Which Bar (input number 1 thru 8)?\n");
			if (scanf_s("%d", &bar) <= 0) {

				printf("Error reading input!\n");
				goto Error;

			}

			if (bar == 0 || bar > 8) {
				printf("Invalid bar number!\n");
				goto Error;
			}

			bar--; // normalize to 0 to 7

			barGraphState.BarsAsUChar = 1 << (UCHAR)bar;

			if (!DeviceIoControl(deviceHandle,
				IOCTL_OSRUSBFX2_SET_BAR_GRAPH_DISPLAY,
				&barGraphState,           // Ptr to InBuffer
				sizeof(BAR_GRAPH_STATE),  // Length of InBuffer
				NULL,         // Ptr to OutBuffer
				0,            // Length of OutBuffer
				&index,       // BytesReturned
				0)) {         // Ptr to Overlapped structure

				code = GetLastError();

				printf("DeviceIoControl failed with error 0x%x\n", code);

				goto Error;
			}

			break;

		case CLEAR_ONE_BAR:


			printf("Which Bar (input number 1 thru 8)?\n");
			if (scanf_s("%d", &bar) <= 0) {

				printf("Error reading input!\n");
				goto Error;

			}

			if (bar == 0 || bar > 8) {
				printf("Invalid bar number!\n");
				goto Error;
			}

			bar--;

			//
			// Read the current state
			//
			if (!DeviceIoControl(deviceHandle,
				IOCTL_OSRUSBFX2_GET_BAR_GRAPH_DISPLAY,
				NULL,             // Ptr to InBuffer
				0,            // Length of InBuffer
				&barGraphState,           // Ptr to OutBuffer
				sizeof(BAR_GRAPH_STATE),  // Length of OutBuffer
				&index,        // BytesReturned
				0)) {         // Ptr to Overlapped structure

				code = GetLastError();

				printf("DeviceIoControl failed with error 0x%x\n", code);

				goto Error;
			}

			if (barGraphState.BarsAsUChar & (1 << bar)) {

				printf("Bar is set...Clearing it\n");
				barGraphState.BarsAsUChar &= ~(1 << bar);

				if (!DeviceIoControl(deviceHandle,
					IOCTL_OSRUSBFX2_SET_BAR_GRAPH_DISPLAY,
					&barGraphState,         // Ptr to InBuffer
					sizeof(BAR_GRAPH_STATE), // Length of InBuffer
					NULL,             // Ptr to OutBuffer
					0,            // Length of OutBuffer
					&index,        // BytesReturned
					0)) {          // Ptr to Overlapped structure

					code = GetLastError();

					printf("DeviceIoControl failed with error 0x%x\n", code);

					goto Error;

				}

			}
			else {

				printf("Bar not set.\n");

			}

			break;

		case LIGHT_ALL_BARS:			

			break;

		case CLEAR_ALL_BARS:

			barGraphState.BarsAsUChar = 0;

			if (!DeviceIoControl(deviceHandle,
				IOCTL_OSRUSBFX2_SET_BAR_GRAPH_DISPLAY,
				&barGraphState,                 // Ptr to InBuffer
				sizeof(BAR_GRAPH_STATE),         // Length of InBuffer
				NULL,             // Ptr to OutBuffer
				0,            // Length of OutBuffer
				&index,         // BytesReturned
				0)) {        // Ptr to Overlapped structure

				code = GetLastError();

				printf("DeviceIoControl failed with error 0x%x\n", code);

				goto Error;
			}

			break;


		case GET_BAR_GRAPH_LIGHT_STATE:

			barGraphState.BarsAsUChar = 0;

			if (!DeviceIoControl(deviceHandle,
				IOCTL_OSRUSBFX2_GET_BAR_GRAPH_DISPLAY,
				NULL,             // Ptr to InBuffer
				0,            // Length of InBuffer
				&barGraphState,          // Ptr to OutBuffer
				sizeof(BAR_GRAPH_STATE), // Length of OutBuffer
				&index,                  // BytesReturned
				0)) {                   // Ptr to Overlapped structure

				code = GetLastError();

				printf("DeviceIoControl failed with error 0x%x\n", code);

				goto Error;
			}

			printf("Bar Graph: \n");
			printf("    Bar8 is %s\n", barGraphState.Bar8 ? "ON" : "OFF");
			printf("    Bar7 is %s\n", barGraphState.Bar7 ? "ON" : "OFF");
			printf("    Bar6 is %s\n", barGraphState.Bar6 ? "ON" : "OFF");
			printf("    Bar5 is %s\n", barGraphState.Bar5 ? "ON" : "OFF");
			printf("    Bar4 is %s\n", barGraphState.Bar4 ? "ON" : "OFF");
			printf("    Bar3 is %s\n", barGraphState.Bar3 ? "ON" : "OFF");
			printf("    Bar2 is %s\n", barGraphState.Bar2 ? "ON" : "OFF");
			printf("    Bar1 is %s\n", barGraphState.Bar1 ? "ON" : "OFF");

			break;

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

		case GET_7_SEGEMENT_STATE:

			printf("7 Segment mask:  0x%x\n", sevenSegment);
			break;

		case SET_7_SEGEMENT_STATE:		

			printf("7 Segment mask:  0x%x\n", sevenSegment);
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

//exit:
//
//if (pinBuf) {
//	free(pinBuf);
//}
//
//if (poutBuf) {
//	free(poutBuf);
//}

