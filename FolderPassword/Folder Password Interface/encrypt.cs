using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Folder_Password_Interface
{
    class encrypt
    {
        // Senha usada na função de criptografia
        static string passwordForEncrypt = "testOfCryptographyTest";

        /// <summary>
        /// Criptografa uma string, meu metódo porco foi feito para esse projeto
        /// Eu quero evitar o máximo o uso de outros códigos
        /// </summary>
        public static string CriptografarString(string text)
        {
            string encryptedText = "";

            foreach (char chars in passwordForEncrypt)
            {
                string c = chars.ToString();
                encryptedText +=
                    text.Replace(c, "Aa9d3i" + c + "3t4s22F" + c + "21dF3g4" + c + c)
                   .Replace("a", "32FK3").Replace("1", "3radf4").Replace("d", "asd32rjiID")
                   .Replace("e", "fgk3J").Replace("3", "f239kf3").Replace("5", "6lhg4")
                   .Replace("g", "fk5k3").Replace("i", "lkh4").Replace("u", "u2").Replace("o", "5");
            }

            return encryptedText;
        }

        /// <summary>
        /// Verifica se a senha é original igual do arquivo
        /// </summary>
        public static bool SenhaCorreta(string text)
        {
            string readed = File.ReadAllText("C:\\ProgramData\\FolderPassword\\password.txt");

            if (CriptografarString(text) == readed)
            {
                return true;
            }

            return false;
        }

    }
}
