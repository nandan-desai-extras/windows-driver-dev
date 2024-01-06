#include <ntifs.h>
#include <ntddk.h>

DRIVER_UNLOAD   DriverUnload;
PVOID registrationHandle = NULL;
UNICODE_STRING altitude;
OB_OPERATION_REGISTRATION obOperationRegistration = { 0 };
OB_CALLBACK_REGISTRATION obCallbackRegistration = { 0 };

PUNICODE_STRING GetProcessNameFromEPROCESS(PEPROCESS process) {
    if (process != NULL) {
        int IMAGE_FILE_PTR_OFFSET = 0x5a0;
        FILE_OBJECT* imageFileObj = (FILE_OBJECT*)*(UINT64*)(((UINT64)process) + IMAGE_FILE_PTR_OFFSET);
        if (imageFileObj == NULL) {
            return NULL;
        }
        return (PUNICODE_STRING)&imageFileObj->FileName;
    }
    return NULL;
}


OB_PREOP_CALLBACK_STATUS MyPreOperationCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION PreOperationInfo) {
    UNREFERENCED_PARAMETER(RegistrationContext);
    if (PreOperationInfo->ObjectType == *PsProcessType) {
        PUNICODE_STRING targetProcName = GetProcessNameFromEPROCESS((PEPROCESS)PreOperationInfo->Object);
        PUNICODE_STRING clientProcName = GetProcessNameFromEPROCESS(PsGetCurrentProcess());
        if (targetProcName == NULL) {
            DbgPrintEx(0, 0, "[ProcessInteraction] Could not get target proc image filename\n");
        }
        if (clientProcName == NULL) {
            DbgPrintEx(0, 0, "[ProcessInteraction] Could not get client proc image filename\n");
        }
        if (targetProcName == NULL && clientProcName == NULL) {
            DbgPrintEx(0, 0, "[ProcessInteraction] Could not get both client and target proc image filename\n");
        }
        if (targetProcName != NULL && clientProcName != NULL) {
            if (targetProcName->Buffer != NULL && clientProcName->Buffer != NULL) {
                DbgPrintEx(0, 0, "[ProcessInteraction] Access to process %ws from %ws\n", targetProcName->Buffer, clientProcName->Buffer);
            }
        }
    }
    return OB_PREOP_SUCCESS;
}

DriverEntry(PDRIVER_OBJECT   DriverObject, PUNICODE_STRING  RegistryPath) {
    DbgPrintEx(0, 0, "DriverEntry!!!!!!!!!!!!!\n");
    NTSTATUS status;
    UNREFERENCED_PARAMETER(RegistryPath);

    obOperationRegistration.ObjectType = PsProcessType;
    obOperationRegistration.Operations = OB_OPERATION_HANDLE_CREATE;
    obOperationRegistration.PreOperation = MyPreOperationCallback;

    RtlInitUnicodeString(&altitude, L"1000");

    obCallbackRegistration.Version = OB_FLT_REGISTRATION_VERSION;
    obCallbackRegistration.OperationRegistrationCount = 1;
    obCallbackRegistration.OperationRegistration = &obOperationRegistration;
    obCallbackRegistration.Altitude = altitude;
    status = ObRegisterCallbacks(&obCallbackRegistration, &registrationHandle);

    if (NT_SUCCESS(status)) {
        DbgPrintEx(0, 0, "Callback registered successfully\n");
    }
    else {
        DbgPrintEx(0, 0, "Callback registration failed with status 0x%x\n", status);
    }

    // Driver unload routine
    DriverObject->DriverUnload = DriverUnload;

    return status;
}

VOID DriverUnload(_In_ PDRIVER_OBJECT DriverObject) {
    UNREFERENCED_PARAMETER(DriverObject);
    if (registrationHandle) {
        ObUnRegisterCallbacks(registrationHandle);
        DbgPrintEx(0, 0, "Callback unregistered\n");
    }
}