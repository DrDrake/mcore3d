namespace Studio
{
	partial class RenderWindow
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(RenderWindow));
			this.toolStripGizmo = new System.Windows.Forms.ToolStrip();
			this.toolStripButtonMove = new System.Windows.Forms.ToolStripButton();
			this.toolStripButtonTranslate = new System.Windows.Forms.ToolStripButton();
			this.toolStripButtonRotate = new System.Windows.Forms.ToolStripButton();
			this.toolStripButtonScale = new System.Windows.Forms.ToolStripButton();
			this.toolStripButtonPlay = new System.Windows.Forms.ToolStripButton();
			this.panel1 = new System.Windows.Forms.Panel();
			this.toolStripGizmo.SuspendLayout();
			this.SuspendLayout();
			// 
			// toolStripGizmo
			// 
			this.toolStripGizmo.AccessibleRole = System.Windows.Forms.AccessibleRole.ToolBar;
			this.toolStripGizmo.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripButtonMove,
            this.toolStripButtonTranslate,
            this.toolStripButtonRotate,
            this.toolStripButtonScale,
            this.toolStripButtonPlay});
			this.toolStripGizmo.Location = new System.Drawing.Point(0, 0);
			this.toolStripGizmo.Name = "toolStripGizmo";
			this.toolStripGizmo.Size = new System.Drawing.Size(292, 25);
			this.toolStripGizmo.TabIndex = 0;
			// 
			// toolStripButtonMove
			// 
			this.toolStripButtonMove.CheckOnClick = true;
			this.toolStripButtonMove.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.toolStripButtonMove.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonMove.Image")));
			this.toolStripButtonMove.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButtonMove.Name = "toolStripButtonMove";
			this.toolStripButtonMove.Size = new System.Drawing.Size(23, 22);
			this.toolStripButtonMove.Text = "Move";
			this.toolStripButtonMove.Click += new System.EventHandler(this.toolStripButtonMove_Click);
			// 
			// toolStripButtonTranslate
			// 
			this.toolStripButtonTranslate.CheckOnClick = true;
			this.toolStripButtonTranslate.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.toolStripButtonTranslate.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonTranslate.Image")));
			this.toolStripButtonTranslate.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButtonTranslate.Name = "toolStripButtonTranslate";
			this.toolStripButtonTranslate.Size = new System.Drawing.Size(23, 22);
			this.toolStripButtonTranslate.Text = "Translate";
			this.toolStripButtonTranslate.Click += new System.EventHandler(this.toolStripButtonTranslate_Click);
			// 
			// toolStripButtonRotate
			// 
			this.toolStripButtonRotate.CheckOnClick = true;
			this.toolStripButtonRotate.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.toolStripButtonRotate.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonRotate.Image")));
			this.toolStripButtonRotate.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButtonRotate.Name = "toolStripButtonRotate";
			this.toolStripButtonRotate.Size = new System.Drawing.Size(23, 22);
			this.toolStripButtonRotate.Text = "Rotate";
			this.toolStripButtonRotate.Click += new System.EventHandler(this.toolStripButtonRotate_Click);
			// 
			// toolStripButtonScale
			// 
			this.toolStripButtonScale.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.toolStripButtonScale.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonScale.Image")));
			this.toolStripButtonScale.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButtonScale.Name = "toolStripButtonScale";
			this.toolStripButtonScale.Size = new System.Drawing.Size(23, 22);
			this.toolStripButtonScale.Text = "Scale";
			this.toolStripButtonScale.Click += new System.EventHandler(this.toolStripButtonScale_Click);
			// 
			// toolStripButtonPlay
			// 
			this.toolStripButtonPlay.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.toolStripButtonPlay.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonPlay.Image")));
			this.toolStripButtonPlay.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButtonPlay.Name = "toolStripButtonPlay";
			this.toolStripButtonPlay.Size = new System.Drawing.Size(23, 22);
			this.toolStripButtonPlay.Text = "toolStripButton1";
			// 
			// panel1
			// 
			this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel1.Location = new System.Drawing.Point(0, 25);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(292, 225);
			this.panel1.TabIndex = 1;
			// 
			// RenderWindow
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(292, 250);
			this.Controls.Add(this.panel1);
			this.Controls.Add(this.toolStripGizmo);
			this.Name = "RenderWindow";
			this.TabText = "RenderWindow";
			this.Text = "RenderWindow";
			this.Enter += new System.EventHandler(this.RenderWindow_Enter);
			this.toolStripGizmo.ResumeLayout(false);
			this.toolStripGizmo.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.ToolStrip toolStripGizmo;
		private System.Windows.Forms.ToolStripButton toolStripButtonMove;
		private System.Windows.Forms.ToolStripButton toolStripButtonTranslate;
		private System.Windows.Forms.ToolStripButton toolStripButtonRotate;
		private System.Windows.Forms.ToolStripButton toolStripButtonScale;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.ToolStripButton toolStripButtonPlay;

	}
}