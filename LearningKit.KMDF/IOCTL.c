#include "LearningKit.h"

VOID
LearningKitIoctlGetInterruptMessage(
	_In_ WDFDEVICE Device,
	_In_ NTSTATUS  ReaderStatus
)
/*++

Routine Description

This method handles the completion of the pended request for the IOCTL
IOCTL_OSRUSBFX2_GET_INTERRUPT_MESSAGE.

Arguments:

Device - Handle to a framework device.

Return Value:

None.

--*/
{
	NTSTATUS            status;
	WDFREQUEST          request;
	PDEVICE_CONTEXT     pDevContext;
	size_t              bytesReturned = 0;
	PSWITCH_STATE       switchState = NULL;

	pDevContext = GetDeviceContext(Device);

	do {

		//
		// Check if there are any pending requests in the Interrupt Message Queue.
		// If a request is found then complete the pending request.
		//
		status = WdfIoQueueRetrieveNextRequest(pDevContext->InterruptMsgQueue, &request);

		if (NT_SUCCESS(status)) {
			status = WdfRequestRetrieveOutputBuffer(request,
				sizeof(SWITCH_STATE),
				&switchState,
				NULL);// BufferLength

			if (!NT_SUCCESS(status)) {

			/*	TraceEvents(TRACE_LEVEL_ERROR, DBG_IOCTL,
					"User's output buffer is too small for this IOCTL, expecting a SWITCH_STATE\n");*/
				bytesReturned = sizeof(SWITCH_STATE);

			}
			else {

				//
				// Copy the state information saved by the continuous reader.
				//
				if (NT_SUCCESS(ReaderStatus)) {
					switchState->SwitchesAsUChar = pDevContext->CurrentSwitchState;
					bytesReturned = sizeof(SWITCH_STATE);
				}
				else {
					bytesReturned = 0;
				}
			}

			//
			// Complete the request.  If we failed to get the output buffer then
			// complete with that status.  Otherwise complete with the status from the reader.
			//
			WdfRequestCompleteWithInformation(request,
				NT_SUCCESS(status) ? ReaderStatus : status,
				bytesReturned);
			status = STATUS_SUCCESS;

		}
		else if (status != STATUS_NO_MORE_ENTRIES) {
			KdPrint(("WdfIoQueueRetrieveNextRequest status %08x\n", status));
		}

		request = NULL;

	} while (status == STATUS_SUCCESS);

	return;

}
