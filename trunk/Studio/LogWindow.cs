using System;

namespace Studio
{
	public partial class LogWindow : DockContent
	{
		public LogWindow()
		{
			if (instance != null)
				throw new Exception("Cannot create more than one LogWindow");

			InitializeComponent();
			instance = this;
		}

		void appendText(string text)
		{
			textBox.AppendText(text);
		}

		static LogWindow instance;
	}
}
