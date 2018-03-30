#include "LearningKit.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, LearningKitKMDFQueueInitialize)
#endif

NTSTATUS
LearningKitKMDFQueueInitialize(
	_In_ WDFDEVICE Device
)
/*++

Routine Description:


	 The I/O dispatch callbacks for the frameworks Device object
	 are configured in this function.

	 A single default I/O Queue is configured for parallel request
	 processing, and a driver context memory allocation is created
	 to hold our structure QUEUE_CONTEXT.

Arguments:

	Device - Handle to a framework Device object.

Return Value:

	VOID

--*/
{
	WDFQUEUE queue;
	NTSTATUS status;
	WDF_IO_QUEUE_CONFIG queueConfig;
	PDEVICE_CONTEXT pDeviceContext;

	PAGED_CODE();

	pDeviceContext = GetDeviceContext(Device);

	//
	// Create a parallel default queue and register an event callback to
	// receive ioctl requests. We will create separate queues for
	// handling read and write requests. All other requests will be
	// completed with error status automatically by the framework.
	//
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig,
		WdfIoQueueDispatchParallel);

	queueConfig.EvtIoDeviceControl = LearningKitKMDFEvtIoDeviceControl;

	//
	// By default, Static Driver Verifier (SDV) displays a warning if it
	// doesn't find the EvtIoStop callback on a power-managed queue.
	// The 'assume' below causes SDV to suppress this warning. If the driver
	// has not explicitly set PowerManaged to WdfFalse, the framework creates
	// power-managed queues when the Device is not a filter driver.  Normally
	// the EvtIoStop is required for power-managed queues, but for this driver
	// it is not needed b/c the driver doesn't hold on to the requests for
	// long time or forward them to other drivers.
	// If the EvtIoStop callback is not implemented, the framework waits for
	// all driver-owned requests to be done before moving in the Dx/sleep
	// states or before removing the Device, which is the correct behavior
	// for this type of driver. If the requests were taking an indeterminate
	// amount of time to complete, or if the driver forwarded the requests
	// to a lower driver/another stack, the queue should have an
	// EvtIoStop/EvtIoResume.
	//
	__analysis_assume(queueConfig.EvtIoStop != 0);
	status = WdfIoQueueCreate(Device,
		&queueConfig,
		WDF_NO_OBJECT_ATTRIBUTES,
		&queue);// pointer to default queue
	__analysis_assume(queueConfig.EvtIoStop == 0);

	if (!NT_SUCCESS(status)) {
		/*TraceEvents(TRACE_LEVEL_ERROR, DBG_PNP,
			"WdfIoQueueCreate failed  %!STATUS!\n", status);*/
		return status;
	}

	//
	// Register a manual I/O queue for handling Interrupt Message Read Requests.
	// This queue will be used for storing Requests that need to wait for an
	// interrupt to occur before they can be completed.
	//
	WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchManual);

	//
	// This queue is used for requests that dont directly access the Device. The
	// requests in this queue are serviced only when the Device is in a fully
	// powered state and sends an interrupt. So we can use a non-power managed
	// queue to park the requests since we dont care whether the Device is idle
	// or fully powered up.
	//
	queueConfig.PowerManaged = WdfFalse;

	status = WdfIoQueueCreate(Device,
		&queueConfig,
		WDF_NO_OBJECT_ATTRIBUTES,
		&pDeviceContext->InterruptMsgQueue
	);

	if (!NT_SUCCESS(status)) {
		/*TraceEvents(TRACE_LEVEL_ERROR, DBG_PNP,
			"WdfIoQueueCreate failed 0x%x\n", status);*/
		return status;
	}


	return status;
}

VOID
LearningKitKMDFEvtIoStop(
	_In_ WDFQUEUE Queue,
	_In_ WDFREQUEST Request,
	_In_ ULONG ActionFlags
)
/*++

Routine Description:

	This event is invoked for a power-managed queue before the Device leaves the working state (D0).

Arguments:

	Queue -  Handle to the framework queue object that is associated with the
			 I/O request.

	Request - Handle to a framework request object.

	ActionFlags - A bitwise OR of one or more WDF_REQUEST_STOP_ACTION_FLAGS-typed flags
				  that identify the reason that the callback function is being called
				  and whether the request is cancelable.

Return Value:

	VOID

--*/
{
	/*TraceEvents(TRACE_LEVEL_INFORMATION,
				TRACE_QUEUE,
				"%!FUNC! Queue 0x%p, Request 0x%p ActionFlags %d",
				Queue, Request, ActionFlags);
*/
//
// In most cases, the EvtIoStop callback function completes, cancels, or postpones
// further processing of the I/O request.
//
// Typically, the driver uses the following rules:
//
// - If the driver owns the I/O request, it either postpones further processing
//   of the request and calls WdfRequestStopAcknowledge, or it calls WdfRequestComplete
//   with a completion status value of STATUS_SUCCESS or STATUS_CANCELLED.
//  
//   The driver must call WdfRequestComplete only once, to either complete or cancel
//   the request. To ensure that another thread does not call WdfRequestComplete
//   for the same request, the EvtIoStop callback must synchronize with the driver's
//   other event callback functions, for instance by using interlocked operations.
//
// - If the driver has forwarded the I/O request to an I/O target, it either calls
//   WdfRequestCancelSentRequest to attempt to cancel the request, or it postpones
//   further processing of the request and calls WdfRequestStopAcknowledge.
//
// A driver might choose to take no action in EvtIoStop for requests that are
// guaranteed to complete in a small amount of time. For example, the driver might
// take no action for requests that are completed in one of the driver’s request handlers.
//

	UNREFERENCED_PARAMETER(Queue);
	UNREFERENCED_PARAMETER(ActionFlags);

	if (ActionFlags &  WdfRequestStopActionSuspend) {
		WdfRequestStopAcknowledge(Request, FALSE); // Don't requeue
	}
	else if (ActionFlags &  WdfRequestStopActionPurge) {
		WdfRequestCancelSentRequest(Request);
	}
	return;

}
