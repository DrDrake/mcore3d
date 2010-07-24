namespace Studio
{
	partial class CodeWindow
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.scintilla = new ScintillaNet.Scintilla();
			((System.ComponentModel.ISupportInitialize)(this.scintilla)).BeginInit();
			this.SuspendLayout();
			// 
			// scintilla
			// 
			this.scintilla.ConfigurationManager.CustomLocation = "SquirrelForScintilla.xml";
			this.scintilla.ConfigurationManager.Language = "squirrel";
			this.scintilla.Dock = System.Windows.Forms.DockStyle.Fill;
			this.scintilla.IsBraceMatching = true;
			this.scintilla.Location = new System.Drawing.Point(0, 0);
			this.scintilla.Margins.Margin2.Width = 10;
			this.scintilla.Margins.Margin3.Width = 10;
			this.scintilla.Name = "scintilla";
			this.scintilla.Size = new System.Drawing.Size(292, 250);
			this.scintilla.TabIndex = 0;
			this.scintilla.TextChanged += new System.EventHandler(this.scintilla_TextChanged);
			// 
			// CodeWindow
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(292, 250);
			this.Controls.Add(this.scintilla);
			this.Name = "CodeWindow";
			this.TabText = "CodeWindow";
			this.Text = "CodeWindow";
			((System.ComponentModel.ISupportInitialize)(this.scintilla)).EndInit();
			this.ResumeLayout(false);

		}

		#endregion

		public ScintillaNet.Scintilla scintilla;

	}
}