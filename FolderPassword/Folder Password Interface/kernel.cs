using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Folder_Password_Interface
{
    class kernel
    {

        // Locais
        public static string portName = "\\FolderPasswordPort";
        public static IntPtr port = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(IntPtr)));

        public static string localArquivo = "C:\\ProgramData\\nTfiles.txt";
        static string localSalvar = "C:\\ProgramData\\FolderPassword\\save.txt";

        // Importação
        [DllImport("Kernel32")]
        internal extern static bool CloseHandle(IntPtr handle);

        // Serve para conectar ao kernel
        [DllImport("FltLib.dll", SetLastError = true)]
        internal extern static int FilterConnectCommunicationPort(
            [MarshalAs(UnmanagedType.LPWStr)] string lpPortName,
            uint dwOptions,
            IntPtr lpContext,
            uint dwSizeOfContext,
            IntPtr lpSecurityAttributes,
            IntPtr hPort
        );

        // Valores
        internal const uint FILE_DEVICE_UNKNOWN = 0x00000022;
        internal const uint FILE_ANY_ACCESS = 0;
        internal const uint METHOD_BUFFERED = 0;
        private const int GENERIC_WRITE = 0x40000000;
        private const int GENERIC_READ = unchecked((int)0x80000000);
        private const int FILE_SHARE_READ = 1;
        private const int FILE_SHARE_WRITE = 2;
        private const int OPEN_EXISTING = 3;

        /// <summary>
        /// Código CTL
        /// </summary>
        /// <returns></returns>
        public static uint CTL_CODE(uint DeviceType, uint Function, uint Method, uint Access)
        {
            return ((DeviceType << 16) | (Access << 14) | (Function << 2) | Method);
        }

        /// <summary>
        /// Códigos CTL
        /// </summary>
        public class CtlCodes
        {
            // Continue as operações
            public static uint GRANT_ACCESS = CTL_CODE(
                FILE_DEVICE_UNKNOWN,
                0x2000,
                METHOD_BUFFERED,
                FILE_ANY_ACCESS
            );

            // Negue as operações
            public static uint DENY_ACCESS = CTL_CODE(
                FILE_DEVICE_UNKNOWN,
                0x1000,
                METHOD_BUFFERED,
                FILE_ANY_ACCESS
            );
        }

        /// <summary>
        /// Operações de comunicação com o kernel
        /// </summary>
        public static class KernelOperations
        {
            /// <summary>
            /// Abrir o dispositivo
            /// </summary>
            [DllImport("Kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
            public static extern int CreateFile(
                String lpFileName, // Nome da porta
                int dwDesiredAccess, // Acesso
                int dwShareMode, // Compartilhamento
                IntPtr lpSecurityAttributes, // Atributos
                int dwCreationDisposition, // Disposition
                int dwFlagsAndAttributes, // Atributos
                int hTemplateFile // Arquivo
            );

            /// <summary>
            /// Fechar o dispositivo
            /// </summary>
            [DllImport("kernel32", SetLastError = true)]
            static extern bool CloseHandle(
                IntPtr handle // Alça
            );

            /// <summary>
            /// DeviceIoControl, necessário para receber e enviar mensagens
            /// </summary>
            [DllImport("kernel32.dll", SetLastError = true)]
            public static extern int DeviceIoControl(
                IntPtr hDevice, // Dispositivo
                uint dwIoControlCode, // Control
                StringBuilder lpInBuffer, // Buffer
                int nInBufferSize, // BufferSize
                StringBuilder lpOutBuffer, // Outbuffer
                Int32 nOutBufferSize, // OutbufferSize
                ref Int32 lpBytesReturned, // Retorno
                IntPtr lpOverlapped //
            );

            /// <summary>
            /// Envia a mensagem ao kernel
            /// </summary>
            public static void SendMessage(string message, uint ctl)
            {
                try
                {
                    IntPtr device = (IntPtr)CreateFile(
                        "\\\\.\\FolderPasswordDevice",
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        IntPtr.Zero,
                        OPEN_EXISTING,
                        0,
                        0
                    );

                    int uCnt = 10;

                    StringBuilder sendToKernel = new StringBuilder(message);

                    StringBuilder reciveToKernel = new StringBuilder();

                    DeviceIoControl(
                        device,
                        ctl,
                        sendToKernel,
                        sendToKernel.Length + 5,

                        reciveToKernel,
                        1,

                        ref uCnt,
                        IntPtr.Zero
                    );

                    CloseHandle(
                        device
                    );
                }
                catch (Exception) { }
            }

            /// <summary>
            /// Se conecta ao driver
            /// </summary>
            public static void ConectarAoDriver()
            {
                FilterConnectCommunicationPort(portName, 0, IntPtr.Zero, 0, IntPtr.Zero, port);
            }
        }

        /// <summary>
        /// Analisa quais arquivos estão sendo modificados, e nega ou continua a operação
        /// </summary>
        public static async void LerArquivos()
        {
            while (true)
            {
                await Task.Delay(50);

                if (!File.Exists(localSalvar))
                {
                    KernelOperations.SendMessage("Permita", CtlCodes.GRANT_ACCESS);
                    return;
                }

                try
                {
                    string arquivo = File.ReadAllText(localArquivo).ToLower();

                    foreach (string linha in File.ReadAllLines(localSalvar))
                    {
                        if (arquivo.Contains(linha.ToLower()))
                        {
                            Senha senha = new Senha("Um processo tentou modificar um arquivo protegido '" + Path.GetFileName(arquivo).ToUpper() + "', digite sua senha para permitir a operação");
                            senha.ShowDialog();

                            if (senha.senhaCorreta == false)
                            {
                                KernelOperations.SendMessage("Bloqueie", CtlCodes.DENY_ACCESS);
                            } else
                            {
                                KernelOperations.SendMessage("Permita", CtlCodes.GRANT_ACCESS);
                            }
                        }
                        else
                        {
                            KernelOperations.SendMessage("Permita", CtlCodes.GRANT_ACCESS);
                        }
                    }
                }
                catch (Exception)
                {
                }

                try
                {
                    File.Delete(localArquivo);
                } catch (Exception) { }
            }
        }

    }
}
