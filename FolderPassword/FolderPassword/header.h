
/*

	Arquivo de cabeçalho

*/

#define _NO_CRT_STDIO_INLINE

#include <fltKernel.h>
#include <ntstrsafe.h>

BOOLEAN stopFileSystem = FALSE;

// http://www.codewarrior.cn/ntdoc/wrk/ps/PsReferenceProcessFilePointer.htm
NTSTATUS
PsReferenceProcessFilePointer(
	IN  PEPROCESS Process,
	OUT PVOID* OutFileObject
);

PFLT_FILTER Filter;

// 0 = Sem valor, o user-mode não recebeu nada
// 1 = Nega a operação
// 2 = Permite a operação
int DenyAccess = 0;

UNICODE_STRING PortName = RTL_CONSTANT_STRING(L"\\FolderPasswordPort"),
DeviceName = RTL_CONSTANT_STRING(L"\\Device\\FolderPasswordDevice"),
SymbolicName = RTL_CONSTANT_STRING(L"\\??\\FolderPasswordDevice");

PFLT_PORT Port, ClientPort;
PDEVICE_OBJECT GlobalDevice;

#define GRANT_ACCESS CTL_CODE(FILE_DEVICE_UNKNOWN, 0x2000, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define DENY_ACCESS CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1000, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define NOTIFY_FILE L"\\??\\C:\\ProgramData\\nTfiles.txt"
#define BUFFER_SIZE 500

UNICODE_STRING CrashSystemFile = RTL_CONSTANT_STRING(L"\\??\\C:\\ProgramData\\FolderPassword\\CRASH");



/* _In_ = A variável é somente leitura
   _Inout_ = A variável vai ser potencialmente modificada
*/

// Dorme 1 segundo
LARGE_INTEGER Time;
VOID Sleep(

);

BOOLEAN Notifying = FALSE;

VOID SendToUserMode(
	_In_ PCHAR FileToLog,
	_In_ PCHAR MessageToLog,
	_In_ ACCESS_MASK Mask
);

PUNICODE_STRING GetFullProcessName(
	_In_ PEPROCESS Process
);

BOOLEAN ProcessGranted(
	_In_ PEPROCESS Process
);

BOOLEAN UnicodeStringToChar(
	_In_ PUNICODE_STRING OldName,
	_Inout_ char NewName[]
);

BOOLEAN FileExits(
	_In_ UNICODE_STRING FileName
);

NTSTATUS Create(
	_In_ PDEVICE_OBJECT DeviceObject,
	_Inout_ PIRP Irp
);

NTSTATUS Message(
	_In_ PDEVICE_OBJECT DeviceObject,
	_Inout_ PIRP Irp
);

NTSTATUS Close(
	_In_ PDEVICE_OBJECT DeviceObject,
	_Inout_ PIRP Irp
);

NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
);

NTSTATUS MinifilterUnload(
	_In_ FLT_FILTER_UNLOAD_FLAGS Flags
);

NTSTATUS Connect(
	_In_ PFLT_PORT Clientport,
	_In_ PVOID ServerPortCookie,
	_In_ PVOID Context,
	_In_ ULONG Size,
	_In_ PVOID ConnectionCookie
);

NTSTATUS Disconnect(
	_In_ PVOID ConnectionCookie
);

FLT_POSTOP_CALLBACK_STATUS IrpInformationBefore(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS Objects,
	_In_ PVOID* Context
);

const FLT_OPERATION_REGISTRATION Callbacks[] = {
	//{IRP_MJ_SET_INFORMATION, 0, IrpInformationBefore, NULL},
	{IRP_MJ_SET_INFORMATION, 0, IrpInformationBefore, NULL},
	{IRP_MJ_OPERATION_END}
};

const FLT_REGISTRATION Registration = {
	sizeof(FLT_REGISTRATION),
	FLT_REGISTRATION_VERSION,
	0,
	NULL,
	Callbacks,
	MinifilterUnload,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

#include "portconnection.h"
#include "filesystem.h"
#include "irp_funcions.h"
