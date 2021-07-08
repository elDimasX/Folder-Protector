
/*

	Operações de IRP / Mensagens

*/

NTSTATUS Create(
	_In_ PDEVICE_OBJECT DeviceObject,
	_Inout_ PIRP Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, STATUS_SUCCESS);

	// Não coloque o valor: Irp->IoStatus.Status aqui
	// O irp já foi completado
	return STATUS_SUCCESS;
}

NTSTATUS Message(
	_In_ PDEVICE_OBJECT DeviceObject,
	_Inout_ PIRP Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	PIO_STACK_LOCATION Io = IoGetCurrentIrpStackLocation(Irp);
	PCHAR UserMsg = (PCHAR)Irp->AssociatedIrp.SystemBuffer;

	Irp->IoStatus.Information = 0;
	if (ClientPort != NULL)
	{
		if (Io->Parameters.DeviceIoControl.IoControlCode == GRANT_ACCESS)
		{
			DenyAccess = 2;
		}

		else if (Io->Parameters.DeviceIoControl.IoControlCode == DENY_ACCESS)
		{
			DenyAccess = 1;
		}
	}
	
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, STATUS_SUCCESS);
	return STATUS_SUCCESS;
}

NTSTATUS Close(
	_In_ PDEVICE_OBJECT DeviceObject,
	_Inout_ PIRP Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, STATUS_SUCCESS);
	return STATUS_SUCCESS;
}
