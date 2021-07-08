
/*

	Operações do minifiltro, é aqui que a mágica acontece

*/

#define TAG 'Tag1'

BOOLEAN FileExits(
	_In_ UNICODE_STRING FileName
)
{
	HANDLE Handle;
	BOOLEAN Exists = FALSE;

	IO_STATUS_BLOCK Block;
	OBJECT_ATTRIBUTES Attrib;

	InitializeObjectAttributes(
		&Attrib,
		&FileName,
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
		NULL, NULL
	);

	NTSTATUS Status = ZwOpenFile(
		&Handle,
		GENERIC_READ | SYNCHRONIZE,
		&Attrib,
		&Block,
		FILE_SHARE_READ,
		FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
	);

	if (NT_SUCCESS(Status))
	{
		Exists = TRUE;
		ZwClose(Handle);
	}

	return Exists;
}

BOOLEAN UnicodeStringToChar(
	_In_ PUNICODE_STRING OldName,
	_Inout_ char NewName[]
)
{
	__try {

		ANSI_STRING AnsiString;
		NTSTATUS Status;

		char* NameConverted;

		Status = RtlUnicodeStringToAnsiString(
			&AnsiString,
			OldName,
			TRUE
		);

		if (NT_SUCCESS(Status) && AnsiString.Length < 600)
		{
			NameConverted = (PCHAR)AnsiString.Buffer;

			strcpy(
				NewName,
				_strupr(NameConverted)
			);
		}

		else {
			return FALSE;
		}

		RtlFreeAnsiString(&AnsiString);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return FALSE;
	}

	return TRUE;
}

VOID SendToUserMode(
	_In_ PCHAR FileToLog,
	_In_ PCHAR MessageToLog,
	_In_ ACCESS_MASK Mask
)
{
	if (KeGetCurrentIrql() != PASSIVE_LEVEL)
	{
		return;
	}

	for (ULONG i = 0; i < 6; i++)
	{
		Sleep();

		if (Notifying == FALSE)
			break;
	}

	Notifying = TRUE;

	UNICODE_STRING UnicodeFile;
	OBJECT_ATTRIBUTES Attrib;

	RtlInitUnicodeString(&UnicodeFile, FileToLog);
	
	InitializeObjectAttributes(
		&Attrib,
		&UnicodeFile,
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
		NULL, NULL
	);

	HANDLE Handle;
	IO_STATUS_BLOCK Block;

	NTSTATUS Status = ZwCreateFile(
		&Handle,
		Mask | SYNCHRONIZE,
		&Attrib,
		&Block,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		0,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE,
		NULL,
		0
	);

	if (NT_SUCCESS(Status))
	{
		CHAR Buffer[BUFFER_SIZE];
		size_t tb;

		Status = RtlStringCbPrintfA(
			Buffer,
			sizeof(Buffer),
			MessageToLog,
			0x0
		);

		if (NT_SUCCESS(Status))
		{
			Status = RtlStringCbLengthA(
				Buffer,
				sizeof(Buffer),
				&tb
			);

			if (NT_SUCCESS(Status))
			{
				Status = ZwWriteFile(
					Handle,
					NULL,
					NULL,
					NULL,
					&Block,
					Buffer,
					tb,
					NULL,
					NULL
				);
			}
		}

		Status = ZwClose(Handle);
	}

	Notifying = FALSE;
}


FLT_POSTOP_CALLBACK_STATUS IrpInformationBefore(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS Objects,
	_In_ PVOID* Context
)
{
	UNREFERENCED_PARAMETER(Context);
	UNREFERENCED_PARAMETER(Objects);

	if (ClientPort == NULL || Data->RequestorMode == KernelMode || ProcessGranted(PsGetCurrentProcess()) == TRUE)
	{
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	PFILE_OBJECT FileObject = Data->Iopb->TargetFileObject;
	PFLT_INSTANCE Instance = Objects->Instance;

	NTSTATUS ReturnStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;

	if (KeGetCurrentIrql() != PASSIVE_LEVEL)
	{
		return ReturnStatus;
	}

	PFLT_FILE_NAME_INFORMATION FileNameInfo;
	PVOID DosFileName = NULL;

	PCHAR FileNameLocation = FltAllocatePoolAlignedWithTag(Instance, NonPagedPool, 1024, TAG);

	NTSTATUS Status = FltGetFileNameInformation(
		Data, FLT_FILE_NAME_NORMALIZED, &FileNameInfo
	);

	if (NT_SUCCESS(Status))
	{
		Status = FltParseFileNameInformation(FileNameInfo);

		if (NT_SUCCESS(Status))
		{
			PFLT_VOLUME volume;
			PDEVICE_OBJECT DiskDeviceObject;
			UNICODE_STRING dosName;
			volume = Objects->Volume;

			Status = FltGetDiskDeviceObject(volume, &DiskDeviceObject);

			RtlInitUnicodeString(&dosName, NULL);
			Status = IoVolumeDeviceToDosName(DiskDeviceObject, &dosName);

			ULONG lengthVolume = max(BUFFER_SIZE, DiskDeviceObject->SectorSize);
			ObDereferenceObject(DiskDeviceObject);

			Status = FltGetDiskDeviceObject(volume, &DiskDeviceObject);

			if (!NT_SUCCESS(Status))
			{
				goto exit;
			}

			DosFileName = FltAllocatePoolAlignedWithTag(Instance, NonPagedPool, lengthVolume, TAG);

			if (DosFileName == NULL)
			{
				goto exit;
			}

			if (!UnicodeStringToChar(&FileObject->FileName, FileNameLocation) ||
				!UnicodeStringToChar(&dosName, DosFileName))
			{

				//KdPrint(("Falhou ao obter o nome do arquivo..."));
				goto exit;
			}

			if (stopFileSystem == TRUE)
			{
				while (1)
				{
					Sleep();

					if (ClientPort == NULL || stopFileSystem == FALSE)
						break;
				}
			}

			stopFileSystem = TRUE;

			DenyAccess = 0;

			// Agora, o DosFileName está completo, com o nome do driver e o nome do arquivo
			strcat(
				DosFileName,
				FileNameLocation
			);

			// Se for a nossa pasta, nós proteja
			if (strstr(DosFileName, "C:\\PROGRAMDATA\\FOLDERPASSWORD"))
			{
				Data->IoStatus.Status = STATUS_ACCESS_DENIED;
				Data->IoStatus.Information = 0;

				ReturnStatus = FLT_PREOP_COMPLETE;
				goto exit;
			}
			else if (strstr(DosFileName, "C:\\PROGRAMDATA\\NTFILES.TXT"))
			{
				goto exit;
			}

			SendToUserMode(NOTIFY_FILE, DosFileName, FILE_GENERIC_WRITE);

			for (int i = 0; i < 30; i++)
			{
				Sleep();

				// Negar acesso
				if (DenyAccess == 1)
				{
					Data->IoStatus.Status = STATUS_ACCESS_DENIED;
					Data->IoStatus.Information = 0;

					ReturnStatus = FLT_PREOP_COMPLETE;
					break;
				}

				// Permitir acesso
				else if (DenyAccess == 2)
				{
					break;
				}
			}
		}
	}

	goto exit;

exit:

	if (FileNameInformation != NULL)
	{
		FltReleaseFileNameInformation(FileNameInfo);
	}

	if (FileNameLocation != NULL)
	{
		FltFreePoolAlignedWithTag(Instance, FileNameLocation, TAG);
	}

	if (DosFileName != NULL)
	{
		FltFreePoolAlignedWithTag(Instance, DosFileName, TAG);
	}

	stopFileSystem = FALSE;
	return ReturnStatus;
}
