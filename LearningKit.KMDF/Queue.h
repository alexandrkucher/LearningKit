/*++

Module Name:

    queue.h

Abstract:

    This file contains the queue definitions.

Environment:

    Kernel-mode Driver Framework

--*/

EXTERN_C_START

//
// This is the context that can be placed per queue
// and would contain per queue information.
//
typedef struct _QUEUE_CONTEXT {

    ULONG PrivateDeviceData;  // just a placeholder

} QUEUE_CONTEXT, *PQUEUE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_CONTEXT, QueueGetContext)

NTSTATUS
LearningKitKMDFQueueInitialize(
    _In_ WDFDEVICE Device
    );

//
// Events from the IoQueue object
//


EXTERN_C_END
