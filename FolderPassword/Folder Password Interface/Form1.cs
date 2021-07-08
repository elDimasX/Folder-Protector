using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.ServiceProcess;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Folder_Password_Interface
{
    public partial class Form1 : Form
    {
        string pasta = "C:\\ProgramData\\FolderPassword\\";
        string localSalvar = "C:\\ProgramData\\FolderPassword\\save.txt";

        /// <summary>
        /// Importação da DLL para alterar o cursor
        /// </summary>
        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        private static extern IntPtr LoadCursor(IntPtr hInstance, int lpCursorName);

        // Novo cursor
        private static readonly Cursor CursorMao = new Cursor(LoadCursor(IntPtr.Zero, 32649));

        /// <summary>
        /// Configurar o cursor
        /// </summary>
        public static void CursorNovo(Control body)
        {
            // Procure todos os controles na FORM
            foreach (Control control in body.Controls)
            {
                try
                {
                    // Int
                    int i;

                    // Se for um 
                    if (control.Cursor == Cursors.Hand)
                    {
                        // Altere o cursor
                        control.Cursor = CursorMao;
                    }

                    // Procure outros paineis na FORM
                    for (i = 0; i < 2; i++)
                    {
                        // Sete de novo
                        CursorNovo(control);
                    }
                }
                catch (Exception ex) { MessageBox.Show(ex.Message); }
            }
        }

        /// <summary>
        /// Nova senha
        /// </summary>
        private void SenhaNova()
        {
            try
            {
                if (!File.Exists("C:\\ProgramData\\FolderPassword\\password.txt"))
                {
                    Senha sn = new Senha("");
                    sn.ShowDialog();

                    if (!File.Exists("C:\\ProgramData\\FolderPassword\\password.txt"))
                    {
                        Environment.Exit(0);
                    }
                } else
                {
                    if (VerificarSenha() == false)
                    {
                        Environment.Exit(0);
                    }
                }
            } catch (Exception) { }
        }

        /// <summary>
        /// SHFILEINFO
        /// </summary>
        public struct SHFILEINFO
        {
            public IntPtr hIcon;

            public IntPtr iIcon;

            public uint dwAttributes;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
            public string szDisplayName;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 80)]
            public string szTypeName;
        };

        /// <summary>
        /// Win32
        /// </summary>
        class Win32
        {
            public const uint SHGFI_ICON = 0x100;
            public const uint SHGFI_LARGEICON = 0x0;
            public const uint SHGFI_SMALLICON = 0x1;

            /// <summary>
            /// Importação da DLL 32
            /// </summary>
            [DllImport("shell32.dll")]
            public static extern IntPtr SHGetFileInfo(
                string pszPath, // Local
                uint dwFileAttributes, // Atributos 
                ref SHFILEINFO psfi, // Informações
                uint cbSizeFileInfo, // Tamanho
                uint uFlags // Bandeira
           );
        }

        // Novo SHFILEINFO
        SHFILEINFO iconInfo = new SHFILEINFO();
        IntPtr hImgLarge;
        int iconIndex = -1;

        /// <summary>
        /// Carrega os últimos arquivos
        /// </summary>
        private void CarregarArquivos()
        {
            try
            {
                string[] linhas = File.ReadAllLines(localSalvar);

                foreach (string linha in linhas)
                {
                    AdicionarNoListView(linha);
                }
            } catch (Exception) { }
        }

        /// <summary>
        /// Verifica senha
        /// </summary>
        private bool VerificarSenha()
        {
            Senha senha = new Senha("Digite a sua senha para acessar o aplicativo");
            senha.ShowDialog();

            return senha.senhaCorreta;
        }

        /// <summary>
        /// Inicia tudo
        /// </summary>
        public Form1()
        {
            try
            {
                ServiceController kern = new ServiceController("FolderPassword");
                kern.Start();
            } catch (Exception) { }
            
            Thread th = new Thread(() => kernel.LerArquivos());
            th.SetApartmentState(ApartmentState.STA);
            th.Start();

            InitializeComponent();
            CursorNovo(this);

            try
            {
                Directory.CreateDirectory(pasta);
            } catch (Exception) { }

            CarregarArquivos();

            if (File.Exists("C:\\ProgramData\\FolderPassword\\CRASH"))
            {
                travarSistemaCheckbox.Checked = true;
            }

            SenhaNova();

            kernel.KernelOperations.ConectarAoDriver();
        }

        /// <summary>
        /// Adiciona a lista de arquivos ou pastas
        /// </summary>
        private void AdicionarArquivoOuPasta(string local)
        {
            try
            {
                if (!Directory.Exists(pasta))
                {
                    Directory.CreateDirectory(pasta);
                }
                
                if (!File.Exists(localSalvar))
                {
                    File.Create(localSalvar).Close();
                }

                for (int i = 0; i < listView1.Items.Count; i++)
                {
                    string arquivos = listView1.Items[i].SubItems[0].Text;

                    File.AppendAllText(localSalvar, arquivos + Environment.NewLine, Encoding.GetEncoding("iso-8859-1"));
                }
            } catch (Exception) { }
        }

        /// <summary>
        /// Adiciona um arquivo na listView
        /// </summary>
        private void AdicionarNoListView(string local)
        {
            try
            {
                // Procure todos os items na listView
                for (int i = 0; i < listView1.Items.Count; i++)
                {
                    try
                    {
                        string items = listView1.Items[i].SubItems[0].Text;

                        if (local == items)
                        {
                            return;
                        }
                    }
                    catch (Exception) { }
                }
            }
            catch (Exception) { }

            try
            {
                // Obtenha o icone
                hImgLarge = Win32.SHGetFileInfo(
                    local,
                    0,
                    ref iconInfo,
                    (uint)Marshal.SizeOf(iconInfo),
                    Win32.SHGFI_ICON | Win32.SHGFI_LARGEICON
                );

                Icon icon = Icon.FromHandle(iconInfo.hIcon);
                iconListImages.Images.Add(icon);
                iconIndex++;
            }
            catch (Exception) { }

            ListViewItem item = new ListViewItem(local, iconIndex);

            if (File.Exists(local))
            {
                item.SubItems.Add("Arquivo");
            } else if (Directory.Exists(local))
            {
                item.SubItems.Add("Pasta");
            } else
            {
                item.SubItems.Add("Desconhecido");
            }

            listView1.Items.Add(item);
        }

        /// <summary>
        /// Botão de adicionar
        /// </summary>
        private void adicionarArquivo_Click(object sender, EventArgs e)
        {
            try
            {
                OpenFileDialog arquivoDialogo = new OpenFileDialog();
                arquivoDialogo.Filter = "|*.*";
                arquivoDialogo.Multiselect = true;

                if (arquivoDialogo.ShowDialog() == DialogResult.OK)
                {
                    foreach (string arquivo in arquivoDialogo.FileNames)
                    {
                        AdicionarNoListView(arquivo);
                        AdicionarArquivoOuPasta(arquivo);
                    }
                }
            } catch (Exception) { }
        }

        /// <summary>
        /// Botão de adicionar pasta
        /// </summary>
        private void adicionarPasta_Click(object sender, EventArgs e)
        {
            try
            {


                FolderBrowserDialog dialog = new FolderBrowserDialog();
                if (dialog.ShowDialog() == DialogResult.OK)
                {
                    AdicionarNoListView(dialog.SelectedPath);
                    AdicionarArquivoOuPasta(dialog.SelectedPath);
                }
            }
            catch (Exception) { }
        }

        /// <summary>
        /// Botão de remover
        /// </summary>
        private void remover_Click(object sender, EventArgs e)
        {
            try
            {
                if (listView1.SelectedItems == null || listView1.Items.Count == 0)
                {
                    return;
                }

                foreach (ListViewItem items in listView1.SelectedItems)
                {
                    listView1.Items.Remove(items);
                }

                File.WriteAllText(localSalvar, "");
                foreach (ListViewItem items in listView1.Items)
                {
                    AdicionarArquivoOuPasta(items.Text);
                }
            }
            catch (Exception)
            {

            }
        }

        /// <summary>
        /// Travar ou não o sistema
        /// </summary>
        private void travarSistemaCheckbox_Click(object sender, EventArgs e)
        {
            try
            {
                if (travarSistemaCheckbox.Checked == true)
                {
                    File.Create("C:\\ProgramData\\FolderPassword\\CRASH").Close();
                } else
                {
                    File.Delete("C:\\ProgramData\\FolderPassword\\CRASH");
                }
            } catch (Exception) { }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            e.Cancel = true;
            Hide();
        }

        /// <summary>
        /// Notificação
        /// </summary>
        private void notifyIcon1_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            if (VerificarSenha() == true)
            {
                Show();
            }

        }

        /// <summary>
        /// Troca a senha
        /// </summary>
        private void label5_Click(object sender, EventArgs e)
        {
            try
            {
                string senhaAtual = File.ReadAllText("C:\\ProgramData\\FolderPassword\\password.txt");
                File.Delete("C:\\ProgramData\\FolderPassword\\password.txt");

                Senha sn = new Senha("");
                sn.ShowDialog();

                if (!File.Exists("C:\\ProgramData\\FolderPassword\\password.txt"))
                {
                    File.WriteAllText("C:\\ProgramData\\FolderPassword\\password.txt", senhaAtual);
                }
                else {
                    MessageBox.Show("Senha alterada com sucesso!", "Erro", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Erro", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
    }
}
