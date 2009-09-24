﻿namespace Studio
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(CodeWindow));
			this.scintilla1 = new ScintillaNet.Scintilla();
			((System.ComponentModel.ISupportInitialize)(this.scintilla1)).BeginInit();
			this.SuspendLayout();
			// 
			// scintilla1
			// 
			this.scintilla1.ConfigurationManager.CustomLocation = "SquirrelForScintilla.xml";
			this.scintilla1.ConfigurationManager.Language = "squirrel";
			this.scintilla1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.scintilla1.IsBraceMatching = true;
			this.scintilla1.Location = new System.Drawing.Point(0, 0);
			this.scintilla1.Margins.Margin1.Width = 0;
			this.scintilla1.Margins.Margin2.Width = 10;
			this.scintilla1.Margins.Margin3.Width = 10;
			this.scintilla1.Name = "scintilla1";
			this.scintilla1.Size = new System.Drawing.Size(292, 250);
			this.scintilla1.TabIndex = 0;
			this.scintilla1.Text = resources.GetString("scintilla1.Text");
			// 
			// CodeWindow
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(292, 250);
			this.Controls.Add(this.scintilla1);
			this.Name = "CodeWindow";
			this.TabText = "CodeWindow";
			this.Text = "CodeWindow";
			((System.ComponentModel.ISupportInitialize)(this.scintilla1)).EndInit();
			this.ResumeLayout(false);

		}

		#endregion

		private ScintillaNet.Scintilla scintilla1;
	}
}