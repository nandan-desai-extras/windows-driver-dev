#pragma once
#include <fltKernel.h>

// Function prototypes
FLT_PREOP_CALLBACK_STATUS MyPreOperationCallback(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext);
FLT_POSTOP_CALLBACK_STATUS MyPostOperationCallback(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID CompletionContext, FLT_POST_OPERATION_FLAGS Flags);
FLT_PREOP_CALLBACK_STATUS MyPreOperationNoPostOperationCallback(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext);
FLT_POSTOP_CALLBACK_STATUS MyPostOperationCallback(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID CompletionContext, FLT_POST_OPERATION_FLAGS Flags);
NTSTATUS MyFilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags);
NTSTATUS MyFilterQueryTeardown(PCFLT_RELATED_OBJECTS FltObjects, FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags);

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {
    { IRP_MJ_CREATE,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_CREATE_NAMED_PIPE,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_CLOSE,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_READ,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_WRITE,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_QUERY_INFORMATION,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_SET_INFORMATION,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_QUERY_EA,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_SET_EA,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_FLUSH_BUFFERS,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_QUERY_VOLUME_INFORMATION,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_SET_VOLUME_INFORMATION,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_DIRECTORY_CONTROL,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_FILE_SYSTEM_CONTROL,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_DEVICE_CONTROL,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_INTERNAL_DEVICE_CONTROL,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_SHUTDOWN,
      0,
      MyPreOperationNoPostOperationCallback,
      NULL },                               //post operations not supported

    { IRP_MJ_LOCK_CONTROL,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_CLEANUP,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_CREATE_MAILSLOT,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_QUERY_SECURITY,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_SET_SECURITY,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_QUERY_QUOTA,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_SET_QUOTA,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_PNP,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_RELEASE_FOR_SECTION_SYNCHRONIZATION,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_ACQUIRE_FOR_MOD_WRITE,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_RELEASE_FOR_MOD_WRITE,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_ACQUIRE_FOR_CC_FLUSH,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_RELEASE_FOR_CC_FLUSH,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_FAST_IO_CHECK_IF_POSSIBLE,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_NETWORK_QUERY_OPEN,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_MDL_READ,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_MDL_READ_COMPLETE,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_PREPARE_MDL_WRITE,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_MDL_WRITE_COMPLETE,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_VOLUME_MOUNT,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_VOLUME_DISMOUNT,
      0,
      MyPreOperationCallback,
      MyPostOperationCallback },

    { IRP_MJ_OPERATION_END }
};