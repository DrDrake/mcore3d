using System;
using System.Windows.Forms;
using System.IO;

namespace Studio
{
	static class Program
	{
		/// <summary>
		/// Guess MCore's bin directory
		/// </summary>
		static string GetMCoreBinDir()
		{
			string[] paths = new string[] {
				@".\",
				@"Bin\",
				@"..\Bin\",
				@"..\..\Bin\",
				@"..\..\..\Bin\",
			};

			foreach (string path in paths)
			{
				if (File.Exists(path + "MCDCore.dll") || File.Exists(path + "MCDCored.dll"))
					return Path.GetFullPath(path);
			}

			return "";
		}

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main()
		{
			string mcoreBinDir = GetMCoreBinDir();
			if (mcoreBinDir.Length > 0)
			{
				string path = Environment.GetEnvironmentVariable("Path");
				Environment.SetEnvironmentVariable("Path", mcoreBinDir + ";" + path);
			}

			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			Application.Run(new MainForm());
		}
	}
}
