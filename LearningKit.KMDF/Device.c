#include "LearningKit.h"
//#include "device.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, LearningKitKMDFEvtDevicePrepareHardware)
#pragma alloc_text (PAGE, LearningKitKMDFEvtDeviceD0Entry)
#pragma alloc_text (PAGE, LearningKitKMDFEvtDeviceD0Exit)
#endif


NTSTATUS
LearningKitKMDFEvtDeviceAdd(
	_In_    WDFDRIVER       Driver,
	_Inout_ PWDFDEVICE_INIT DeviceInit
)
	/*++
	Routine Description:

	EvtDeviceAdd is called by the framework in response to AddDevice
	call from the PnP manager. We create and initialize a device object to
	represent a new instance of the device.

	Arguments:

	Driver - Handle to a framework driver object created in DriverEntry

	DeviceInit - Pointer to a framework-allocated WDFDEVICE_INIT structure.

	Return Value:

	NTSTATUS

	--*/
{
	NTSTATUS status;

	WDF_PNPPOWER_EVENT_CALLBACKS pnpPowerCallbacks;
	WDF_DEVICE_PNP_CAPABILITIES pnpCaps;
	WDF_OBJECT_ATTRIBUTES   deviceAttributes;
	PDEVICE_CONTEXT deviceContext;
	WDFDEVICE device;
	WDF_OBJECT_ATTRIBUTES attributes;

	PAGED_CODE();

	WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
	pnpPowerCallbacks.EvtDevicePrepareHardware = LearningKitKMDFEvtDevicePrepareHardware;

	pnpPowerCallbacks.EvtDeviceD0Entry = LearningKitKMDFEvtDeviceD0Entry;
	pnpPowerCallbacks.EvtDeviceD0Exit = LearningKitKMDFEvtDeviceD0Exit;

	WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks);

	WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoBuffered);

	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);

	status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);

	if (NT_SUCCESS(status)) {

		
		WDF_DEVICE_PNP_CAPABILITIES_INIT(&pnpCaps);
		pnpCaps.SurpriseRemovalOK = WdfTrue;

		WdfDeviceSetPnpCapabilities(device, &pnpCaps);

		deviceContext = GetDeviceContext(device);
				
		//
		// Create a device interface so that applications can find and talk
		// to us.
		//
		status = WdfDeviceCreateDeviceInterface(
			device,
			&GUID_DEVINTERFACE_LearningKitKMDF,
			NULL // ReferenceString
		);

		if (NT_SUCCESS(status)) {
			//
			// Initialize the I/O Package and any Queues
			//
			status = LearningKitKMDFQueueInitialize(device);
		}

		//
		// Create the lock that we use to serialize calls to ResetDevice(). As an
		// alternative to using a WDFWAITLOCK to serialize the calls, a sequential
		// WDFQUEUE can be created and reset IOCTLs would be forwarded to it.
		//
		WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
		attributes.ParentObject = device;

		status = WdfWaitLockCreate(&attributes, &deviceContext->ResetDeviceWaitLock);
		if (!NT_SUCCESS(status)) {
			/*TraceEvents(TRACE_LEVEL_ERROR, DBG_PNP,
				"WdfWaitLockCreate failed  %!STATUS!\n", status);*/
			return status;
		}
	}

	return status;
}

NTSTATUS
LearningKitKMDFEvtDevicePrepareHardware(
	_In_ WDFDEVICE Device,
	_In_ WDFCMRESLIST ResourceList,
	_In_ WDFCMRESLIST ResourceListTranslated
)
/*++

Routine Description:

	In this callback, the driver does whatever is necessary to make the
	hardware ready to use.  In the case of a USB device, this involves
	reading and selecting descriptors.

Arguments:

	Device - handle to a device

Return Value:

	NT status value

--*/
{
	NTSTATUS status;
	PDEVICE_CONTEXT pDeviceContext;
	WDF_USB_DEVICE_CREATE_CONFIG createParams;
	WDF_USB_DEVICE_SELECT_CONFIG_PARAMS configParams;
	WDF_USB_DEVICE_INFORMATION          deviceInfo;
	ULONG                               waitWakeEnable;

	UNREFERENCED_PARAMETER(ResourceList);
	UNREFERENCED_PARAMETER(ResourceListTranslated);

	waitWakeEnable = FALSE;

	PAGED_CODE();

	//TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

	status = STATUS_SUCCESS;
	pDeviceContext = GetDeviceContext(Device);

	//
	// Create a USB device handle so that we can communicate with the
	// underlying USB stack. The WDFUSBDEVICE handle is used to query,
	// configure, and manage all aspects of the USB device.
	// These aspects include device properties, bus properties,
	// and I/O creation and synchronization. We only create the device the first time
	// PrepareHardware is called. If the device is restarted by pnp manager
	// for resource rebalance, we will use the same device handle but then select
	// the interfaces again because the USB stack could reconfigure the device on
	// restart.
	//
	if (pDeviceContext->UsbDevice == NULL) {

		//
		// Specifying a client contract version of 602 enables us to query for
		// and use the new capabilities of the USB driver stack for Windows 8.
		// It also implies that we conform to rules mentioned in MSDN
		// documentation for WdfUsbTargetDeviceCreateWithParameters.
		//
		WDF_USB_DEVICE_CREATE_CONFIG_INIT(&createParams,
			USBD_CLIENT_CONTRACT_VERSION_602
		);

		status = WdfUsbTargetDeviceCreateWithParameters(Device,
			&createParams,
			WDF_NO_OBJECT_ATTRIBUTES,
			&pDeviceContext->UsbDevice
		);

		if (!NT_SUCCESS(status)) {
			//TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfUsbTargetDeviceCreateWithParameters failed 0x%x", status);
			return status;
		}
	}


	//
	// Retrieve USBD version information, port driver capabilites and device
	// capabilites such as speed, power, etc.
	//
	WDF_USB_DEVICE_INFORMATION_INIT(&deviceInfo);

	status = WdfUsbTargetDeviceRetrieveInformation(
		pDeviceContext->UsbDevice,
		&deviceInfo);
	if (NT_SUCCESS(status)) {
		
		waitWakeEnable = deviceInfo.Traits &
			WDF_USB_DEVICE_TRAIT_REMOTE_WAKE_CAPABLE;

		/*TraceEvents(TRACE_LEVEL_INFORMATION, DBG_PNP,
			"IsDeviceRemoteWakeable: %s\n",
			waitWakeEnable ? "TRUE" : "FALSE");*/
		//
		// Save these for use later.
		//
		pDeviceContext->UsbDeviceTraits = deviceInfo.Traits;
	}
	else {
		pDeviceContext->UsbDeviceTraits = 0;
	}

	status = SelectInterfaces(Device);
	if (!NT_SUCCESS(status)) {
		/*TraceEvents(TRACE_LEVEL_ERROR, DBG_PNP,
			"SelectInterfaces failed 0x%x\n", status);*/
		return status;
	}

	//
	// Enable wait-wake and idle timeout if the device supports it
	//
	if (waitWakeEnable) {
		status = LearningKitSetPowerPolicy(Device);
		if (!NT_SUCCESS(status)) {
			/*TraceEvents(TRACE_LEVEL_ERROR, DBG_PNP,
				"OsrFxSetPowerPolicy failed  %!STATUS!\n", status);*/
			return status;
		}
	}

	status = LearningKitConfigContReaderForInterruptEndPoint(pDeviceContext);

	/*TraceEvents(TRACE_LEVEL_INFORMATION, DBG_PNP, "<-- EvtDevicePrepareHardware\n");*/

	return status;


	/*
	//
	// Select the first configuration of the device, using the first alternate
	// setting of each interface
	//
	WDF_USB_DEVICE_SELECT_CONFIG_PARAMS_INIT_MULTIPLE_INTERFACES(&configParams,
		0,
		NULL
	);
	status = WdfUsbTargetDeviceSelectConfig(pDeviceContext->UsbDevice,
		WDF_NO_OBJECT_ATTRIBUTES,
		&configParams
	);

	if (!NT_SUCCESS(status)) {
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfUsbTargetDeviceSelectConfig failed 0x%x", status);
		return status;
	}

	//TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

	return status;
	*/
}

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
LearningKitSetPowerPolicy(
	_In_ WDFDEVICE Device
)
{
	WDF_DEVICE_POWER_POLICY_IDLE_SETTINGS idleSettings;
	WDF_DEVICE_POWER_POLICY_WAKE_SETTINGS wakeSettings;
	NTSTATUS    status = STATUS_SUCCESS;

	PAGED_CODE();

	//
	// Init the idle policy structure.
	//
	WDF_DEVICE_POWER_POLICY_IDLE_SETTINGS_INIT(&idleSettings, IdleUsbSelectiveSuspend);
	idleSettings.IdleTimeout = 10000; // 10-sec

	status = WdfDeviceAssignS0IdleSettings(Device, &idleSettings);
	if (!NT_SUCCESS(status)) {
		/*TraceEvents(TRACE_LEVEL_ERROR, DBG_PNP,
			"WdfDeviceSetPowerPolicyS0IdlePolicy failed %x\n", status);*/
		return status;
	}

	//
	// Init wait-wake policy structure.
	//
	WDF_DEVICE_POWER_POLICY_WAKE_SETTINGS_INIT(&wakeSettings);

	status = WdfDeviceAssignSxWakeSettings(Device, &wakeSettings);
	if (!NT_SUCCESS(status)) {
	/*	TraceEvents(TRACE_LEVEL_ERROR, DBG_PNP,
			"WdfDeviceAssignSxWakeSettings failed %x\n", status);*/
		return status;
	}

	return status;
}


_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
SelectInterfaces(
	_In_ WDFDEVICE Device
)
/*++

Routine Description:

This helper routine selects the configuration, interface and
creates a context for every pipe (end point) in that interface.

Arguments:

Device - Handle to a framework device

Return Value:

NT status value

--*/
{
	WDF_USB_DEVICE_SELECT_CONFIG_PARAMS configParams;
	NTSTATUS                            status = STATUS_SUCCESS;
	PDEVICE_CONTEXT                     pDeviceContext;
	WDFUSBPIPE                          pipe;
	WDF_USB_PIPE_INFORMATION            pipeInfo;
	UCHAR                               index;
	UCHAR                               numberConfiguredPipes;

	PAGED_CODE();

	pDeviceContext = GetDeviceContext(Device);

	WDF_USB_DEVICE_SELECT_CONFIG_PARAMS_INIT_SINGLE_INTERFACE(&configParams);

	status = WdfUsbTargetDeviceSelectConfig(pDeviceContext->UsbDevice,
		WDF_NO_OBJECT_ATTRIBUTES,
		&configParams);
	if (!NT_SUCCESS(status)) {

	/*	TraceEvents(TRACE_LEVEL_ERROR, DBG_PNP,
			"WdfUsbTargetDeviceSelectConfig failed %!STATUS! \n",
			status);*/

		//
		// Since the Osr USB fx2 device is capable of working at high speed, the only reason
		// the device would not be working at high speed is if the port doesn't
		// support it. If the port doesn't support high speed it is a 1.1 port
		//
		if ((pDeviceContext->UsbDeviceTraits & WDF_USB_DEVICE_TRAIT_AT_HIGH_SPEED) == 0) {
			GUID activity = DeviceToActivityId(Device);

			/*TraceEvents(TRACE_LEVEL_ERROR, DBG_PNP,
				" On a 1.1 USB port on Windows Vista"
				" this is expected as the OSR USB Fx2 board's Interrupt EndPoint descriptor"
				" doesn't conform to the USB specification. Windows Vista detects this and"
				" returns an error. \n"
			);*/
			/*EventWriteSelectConfigFailure(
				&activity,
				pDeviceContext->DeviceName,
				pDeviceContext->Location,
				status
			);*/
		}

		return status;
	}

	pDeviceContext->UsbInterface =
		configParams.Types.SingleInterface.ConfiguredUsbInterface;

	numberConfiguredPipes = configParams.Types.SingleInterface.NumberConfiguredPipes;

	//
	// Get pipe handles
	//
	for (index = 0; index < numberConfiguredPipes; index++) {

		WDF_USB_PIPE_INFORMATION_INIT(&pipeInfo);

		pipe = WdfUsbInterfaceGetConfiguredPipe(
			pDeviceContext->UsbInterface,
			index, //PipeIndex,
			&pipeInfo
		);
		//
		// Tell the framework that it's okay to read less than
		// MaximumPacketSize
		//
		WdfUsbTargetPipeSetNoMaximumPacketSizeCheck(pipe);

		if (WdfUsbPipeTypeInterrupt == pipeInfo.PipeType) {
			/*TraceEvents(TRACE_LEVEL_INFORMATION, DBG_IOCTL,
				"Interrupt Pipe is 0x%p\n", pipe);*/
			pDeviceContext->InterruptPipe = pipe;
		}

		//if (WdfUsbPipeTypeBulk == pipeInfo.PipeType &&
		//	WdfUsbTargetPipeIsInEndpoint(pipe)) {
		//	/*TraceEvents(TRACE_LEVEL_INFORMATION, DBG_IOCTL,
		//		"BulkInput Pipe is 0x%p\n", pipe);*/
		//	pDeviceContext->BulkReadPipe = pipe;
		//}

		//if (WdfUsbPipeTypeBulk == pipeInfo.PipeType &&
		//	WdfUsbTargetPipeIsOutEndpoint(pipe)) {
		//	/*TraceEvents(TRACE_LEVEL_INFORMATION, DBG_IOCTL,
		//		"BulkOutput Pipe is 0x%p\n", pipe);*/
		//	pDeviceContext->BulkWritePipe = pipe;
		//}

	}

	//
	// If we didn't find all the 3 pipes, fail the start.
	//
	if (!(/*pDeviceContext->BulkWritePipe
		&& pDeviceContext->BulkReadPipe && */pDeviceContext->InterruptPipe)) {
		status = STATUS_INVALID_DEVICE_STATE;
	/*	TraceEvents(TRACE_LEVEL_ERROR, DBG_PNP,
			"Device is not configured properly %!STATUS!\n",
			status);
*/
		return status;
	}

	return status;
}

NTSTATUS
LearningKitKMDFEvtDeviceD0Entry(
	_In_
	WDFDEVICE Device,
	_In_
	WDF_POWER_DEVICE_STATE PreviousState
)
{
	PDEVICE_CONTEXT         pDeviceContext;
	NTSTATUS                status;
	BOOLEAN                 isTargetStarted;

	pDeviceContext = GetDeviceContext(Device);
	isTargetStarted = FALSE;

	//TraceEvents(TRACE_LEVEL_INFORMATION, DBG_POWER, "-->OsrFxEvtEvtDeviceD0Entry - coming from %s\n", DbgDevicePowerString(PreviousState));

	//
	// Since continuous reader is configured for this interrupt-pipe, we must explicitly start
	// the I/O target to get the framework to post read requests.
	//
	status = WdfIoTargetStart(WdfUsbTargetPipeGetIoTarget(pDeviceContext->InterruptPipe));
	if (!NT_SUCCESS(status)) {
		//TraceEvents(TRACE_LEVEL_ERROR, DBG_POWER, "Failed to start interrupt pipe %!STATUS!\n", status);
		goto End;
	}

	isTargetStarted = TRUE;

End:

	if (!NT_SUCCESS(status)) {
		//
		// Failure in D0Entry will lead to device being removed. So let us stop the continuous
		// reader in preparation for the ensuing remove.
		//
		if (isTargetStarted) {
			WdfIoTargetStop(WdfUsbTargetPipeGetIoTarget(pDeviceContext->InterruptPipe), WdfIoTargetCancelSentIo);
		}
	}

	//TraceEvents(TRACE_LEVEL_INFORMATION, DBG_POWER, "<--OsrFxEvtEvtDeviceD0Entry\n");

	return status;
}


NTSTATUS
LearningKitKMDFEvtDeviceD0Exit(
	_In_
	WDFDEVICE Device,
	_In_
	WDF_POWER_DEVICE_STATE TargetState
)
	/*++

	Routine Description:

	This routine undoes anything done in EvtDeviceD0Entry.  It is called
	whenever the device leaves the D0 state, which happens when the device is
	stopped, when it is removed, and when it is powered off.

	The device is still in D0 when this callback is invoked, which means that
	the driver can still touch hardware in this routine.


	EvtDeviceD0Exit event callback must perform any operations that are
	necessary before the specified device is moved out of the D0 state.  If the
	driver needs to save hardware state before the device is powered down, then
	that should be done here.

	This function runs at PASSIVE_LEVEL, though it is generally not paged.  A
	driver can optionally make this function pageable if DO_POWER_PAGABLE is set.

	Even if DO_POWER_PAGABLE isn't set, this function still runs at
	PASSIVE_LEVEL.  In this case, though, the function absolutely must not do
	anything that will cause a page fault.

	Arguments:

	Device - Handle to a framework device object.

	TargetState - Device power state which the device will be put in once this
	callback is complete.

	Return Value:

	Success implies that the device can be used.  Failure will result in the
	device stack being torn down.

	--*/
{
	PDEVICE_CONTEXT  pDeviceContext;

	PAGED_CODE();

	//TraceEvents(TRACE_LEVEL_INFORMATION, DBG_POWER, "-->OsrFxEvtDeviceD0Exit - moving to %s\n",DbgDevicePowerString(TargetState));

	pDeviceContext = GetDeviceContext(Device);

	WdfIoTargetStop(WdfUsbTargetPipeGetIoTarget(pDeviceContext->InterruptPipe), WdfIoTargetCancelSentIo);

	//TraceEvents(TRACE_LEVEL_INFORMATION, DBG_POWER, "<--OsrFxEvtDeviceD0Exit\n");

	return STATUS_SUCCESS;
}

_IRQL_requires_(PASSIVE_LEVEL)
PCHAR
DbgDevicePowerString(
	_In_ WDF_POWER_DEVICE_STATE Type
)
{
	switch (Type)
	{
	case WdfPowerDeviceInvalid:
		return "WdfPowerDeviceInvalid";
	case WdfPowerDeviceD0:
		return "WdfPowerDeviceD0";
	case WdfPowerDeviceD1:
		return "WdfPowerDeviceD1";
	case WdfPowerDeviceD2:
		return "WdfPowerDeviceD2";
	case WdfPowerDeviceD3:
		return "WdfPowerDeviceD3";
	case WdfPowerDeviceD3Final:
		return "WdfPowerDeviceD3Final";
	case WdfPowerDevicePrepareForHibernation:
		return "WdfPowerDevicePrepareForHibernation";
	case WdfPowerDeviceMaximum:
		return "WdfPowerDeviceMaximum";
	default:
		return "UnKnown Device Power State";
	}
}