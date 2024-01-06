/*
* @author Nandan Desai
My first Windows Driver.
Here, I'll try to print the Filesystem activity of all the processes to DbgPrintEx().
This is a Filesystem Mini-Filter Driver.
*/

#include "mycallbacks.h"
#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

// This below function pointer is defined here 
// because ZwQueryInformationProcess is an internal function to Windows Kernel
// https://learn.microsoft.com/en-us/windows/win32/procthread/zwqueryinformationprocess#remarks
// taken from: https://stackoverflow.com/a/40507407
typedef NTSTATUS(*QUERY_INFO_PROCESS) (
    HANDLE ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength
    );
QUERY_INFO_PROCESS ZwQueryInformationProcess;

typedef struct MY_MINI_FILTER_DATA {
    PFLT_FILTER FilterHandle;
} MY_MINI_FILTER_DATA;

MY_MINI_FILTER_DATA MyFilterFilterData;

CONST FLT_REGISTRATION MyFilterRegistration = {
    sizeof(FLT_REGISTRATION),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags
    NULL,                               //  Context
    Callbacks,                               //  Operation callbacks
    MyFilterUnload,                         //  FilterUnload
    NULL,                               //  InstanceSetup
    MyFilterQueryTeardown,                  //  InstanceQueryTeardown
    NULL,                               //  InstanceTeardownStart
    NULL,                               //  InstanceTeardownComplete
    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent
};

ULONG_PTR OperationStatusCtx = 1;

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    DbgPrintEx(0, 0, "DriverEntry!!!!!\n");

    NTSTATUS status;
    UNREFERENCED_PARAMETER(RegistryPath);

    status = FltRegisterFilter(DriverObject, &MyFilterRegistration, &MyFilterFilterData.FilterHandle);
    //FLT_ASSERT(NT_SUCCESS(status));
    if (NT_SUCCESS(status)) {
        status = FltStartFiltering(MyFilterFilterData.FilterHandle);
        if (!NT_SUCCESS(status)) {
            FltUnregisterFilter(MyFilterFilterData.FilterHandle);
        }
    }
    return status;
}


// Taken from https://stackoverflow.com/a/39353474/7090731
NTSTATUS MyGetProcessImageName(PEPROCESS eProcess, PUNICODE_STRING* ProcessImageName) {
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    ULONG returnedLength;
    HANDLE hProcess = NULL;

    if (eProcess == NULL) {
        return STATUS_INVALID_PARAMETER_1;
    }

    status = ObOpenObjectByPointer(eProcess, 0, NULL, 0, 0, KernelMode, &hProcess);
    if (!NT_SUCCESS(status)) {
        DbgPrintEx(0, 0, "ObOpenObjectByPointer Failed: %08x\n", status);
        return status;
    }

    if (ZwQueryInformationProcess == NULL) {
        UNICODE_STRING routineName = RTL_CONSTANT_STRING(L"ZwQueryInformationProcess");

        ZwQueryInformationProcess = (QUERY_INFO_PROCESS)MmGetSystemRoutineAddress(&routineName);

        if (ZwQueryInformationProcess == NULL) {
            DbgPrintEx(0, 0, "Cannot resolve ZwQueryInformationProcess\n");
            status = STATUS_UNSUCCESSFUL;
            goto cleanUp;
        }
    }

    /* Query the actual size of the process path */
    status = ZwQueryInformationProcess(hProcess, ProcessImageFileName,
        NULL, // buffer
        0,    // buffer size
        &returnedLength);

    if (STATUS_INFO_LENGTH_MISMATCH != status) {
        DbgPrint("ZwQueryInformationProcess status = %x\n", status);
        goto cleanUp;
    }

    *ProcessImageName = ExAllocatePool(PagedPool, returnedLength);

    if (ProcessImageName == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanUp;
    }

    /* Retrieve the process path from the handle to the process */
    status = ZwQueryInformationProcess(hProcess, ProcessImageFileName, *ProcessImageName, returnedLength, &returnedLength);
    if (!NT_SUCCESS(status)) {
        ExFreePool(*ProcessImageName);
    }
cleanUp:
    ZwClose(hProcess);
    return status;
}

FLT_PREOP_CALLBACK_STATUS MyPreOperationCallback(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {
    NTSTATUS status;
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    // this is the file name that the process is trying to access on the disk
    PFLT_FILE_NAME_INFORMATION nameInfo;

    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &nameInfo);

    if (!NT_SUCCESS(status)) {
        // ignore
        return FLT_PREOP_SUCCESS_WITH_CALLBACK;
    }

    FltParseFileNameInformation(nameInfo);

    PEPROCESS process = IoThreadToProcess(Data->Thread);
    HANDLE pid = PsGetProcessId(process);
    PUNICODE_STRING processName = NULL;
    if (!NT_SUCCESS(MyGetProcessImageName(process, &processName))) {
        DbgPrintEx(0, 0, "failed to get Process Name\n");
        FltReleaseFileNameInformation(nameInfo);
        return FLT_PREOP_SUCCESS_WITH_CALLBACK;
    }

    DbgPrintEx(0, 0, "FilesystemFilter: File operation from PID=%d, ProcessName=%ws, Filename=%wZ\n", (int)pid, processName->Buffer, &nameInfo->Name);

    ExFreePool(processName);
    FltReleaseFileNameInformation(nameInfo);

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS MyPostOperationCallback(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID CompletionContext, FLT_POST_OPERATION_FLAGS Flags) {
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(Flags);

    //DbgPrintEx(0, 0, ("MyFilterFilter: MyPostOperationCallback Entered!\n"));

    return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_PREOP_CALLBACK_STATUS MyPreOperationNoPostOperationCallback(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    DbgPrintEx(0, 0, ("FilesystemFilter: MyPreOperationNoPostOperationCallback Entered!\n"));

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

NTSTATUS MyFilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags) {
    DbgPrintEx(0, 0, ("FilesystemFilter: MyFilterUnload!\n"));
    UNREFERENCED_PARAMETER(Flags);
    FltUnregisterFilter(MyFilterFilterData.FilterHandle);
    return STATUS_SUCCESS;
}

NTSTATUS MyFilterQueryTeardown(PCFLT_RELATED_OBJECTS FltObjects, FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags) {
    DbgPrintEx(0, 0, ("FilesystemFilter: MyFilterQueryTeardown!\n"));
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);
    return STATUS_SUCCESS;
}

