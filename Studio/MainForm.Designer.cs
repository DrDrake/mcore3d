namespace Studio
{
	partial class MainForm
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
			this.renderPanelControl1 = new RenderPanel.RenderPanelControl();
			this.SuspendLayout();
			// 
			// renderPanelControl1
			// 
			this.renderPanelControl1.Location = new System.Drawing.Point(54, 30);
			this.renderPanelControl1.Name = "renderPanelControl1";
			this.renderPanelControl1.Size = new System.Drawing.Size(447, 329);
			this.renderPanelControl1.TabIndex = 0;
			// 
			// MainForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(556, 410);
			this.Controls.Add(this.renderPanelControl1);
			this.Name = "MainForm";
			this.Text = "Form1";
			this.ResumeLayout(false);

		}

		#endregion

		private RenderPanel.RenderPanelControl renderPanelControl1;
	}
}