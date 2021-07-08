
/*

	FolderLock versão 1.0
	Driver de kernel + user-mode (C#)
	Meu GitHub: https://github.com/elDimasX

*/

#include "header.h"


NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	Time.QuadPart = -10 * 1000 * 1000; // 1 Segundo
	NTSTATUS Status = FltRegisterFilter(DriverObject, &Registration, &Filter);

	if (!NT_SUCCESS(Status))
	{
		goto exit;
	}

	Status = FltStartFiltering(Filter);

	if (!NT_SUCCESS(Status))
	{
		goto exit;
	}

	PSECURITY_DESCRIPTOR Sd = NULL;
	OBJECT_ATTRIBUTES Attrib;

	Status = FltBuildDefaultSecurityDescriptor(
		&Sd,
		FLT_PORT_ALL_ACCESS
	);

	if (!NT_SUCCESS(Status))
	{
		goto exit;
	}

	InitializeObjectAttributes(
		&Attrib,
		&PortName,
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
		NULL, Sd
	);

	Status = FltCreateCommunicationPort(
		Filter,
		&Port,
		&Attrib,
		NULL,
		Connect,
		Disconnect,
		NULL,
		1
	);

	if (!NT_SUCCESS(Status))
	{
		FltFreeSecurityDescriptor(Sd);
		goto exit;
	}

	FltFreeSecurityDescriptor(Sd);

	Status = IoCreateDevice(
		DriverObject,
		0,
		&DeviceName,
		FILE_DEVICE_UNKNOWN,
		NULL,
		TRUE,
		&GlobalDevice
	);
	
	if (!NT_SUCCESS(Status))
	{
		goto exit;
	}

	Status = IoCreateSymbolicLink(&SymbolicName, &DeviceName);

	if (!NT_SUCCESS(Status))
	{
		goto exit;
	}

	DriverObject->MajorFunction[IRP_MJ_CREATE] = Create;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = Message;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = Close;

	return Status;

exit:
	
	KdPrint(("FolderPassword driver falhou ao carregar, erro: 0x%x", Status));

	if (GlobalDevice != NULL)
	{
		IoDeleteDevice(GlobalDevice);
		IoDeleteSymbolicLink(&SymbolicName);
	}

	if (Port != NULL)
	{
		FltCloseCommunicationPort(Port);
		ClientPort = NULL;
	}

	if (Filter != NULL)
	{
		FltUnregisterFilter(Filter);
	}

	return Status;
}

VOID Sleep(

)
{
	KeDelayExecutionThread(
		KernelMode,
		FALSE,
		&Time
	);
}

NTSTATUS MinifilterUnload(
	_In_ FLT_FILTER_UNLOAD_FLAGS Flags
)
{
	UNREFERENCED_PARAMETER(Flags);

	FltCloseCommunicationPort(Port);
	ClientPort = NULL;
	stopFileSystem = FALSE;

	IoDeleteDevice(GlobalDevice);
	IoDeleteSymbolicLink(&SymbolicName);

	FltUnregisterFilter(Filter);

	return STATUS_SUCCESS;
}

BOOLEAN ProcessGranted(
	_In_ PEPROCESS Process
)
{
	ANSI_STRING ProcessName;
	BOOLEAN Granted = FALSE;

	__try {

		NTSTATUS Status = RtlUnicodeStringToAnsiString(
			&ProcessName,
			(UNICODE_STRING*)GetFullProcessName(Process),
			TRUE
		);

		if (!NT_SUCCESS(Status))
		{
			return Granted;
		}

		if (strstr(_strupr(ProcessName.Buffer), "FOLDER PASSWORD"))
		{
			Granted = TRUE;
		}

		RtlFreeAnsiString(&ProcessName);

	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}

	return Granted;
}

PUNICODE_STRING GetFullProcessName(
	_In_ PEPROCESS Process
)
{
	__try {

		PFILE_OBJECT FileObject;
		POBJECT_NAME_INFORMATION FileObjectInfo;

		NTSTATUS Status = PsReferenceProcessFilePointer(Process, &FileObject);

		if (!NT_SUCCESS(Status))
		{
			return NULL;
		}

		Status = IoQueryFileDosDeviceName(FileObject, &FileObjectInfo);

		if (!NT_SUCCESS(Status))
		{
			return NULL;
		}

		return &(FileObjectInfo->Name);

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}

	return NULL;
}
