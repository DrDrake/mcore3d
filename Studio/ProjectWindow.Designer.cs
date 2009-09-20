namespace Studio
{
	partial class ProjectWindow
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
			this.components = new System.ComponentModel.Container();
			this.treeViewAdv = new Aga.Controls.Tree.TreeViewAdv();
			this.treeColumn1 = new Aga.Controls.Tree.TreeColumn();
			this.nodeTextBox1 = new Aga.Controls.Tree.NodeControls.NodeTextBox();
			this.mediaPathMenuStrip = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.addPathToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.sceneMenuStrip = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.addSceneToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mediaPathMenuStrip.SuspendLayout();
			this.sceneMenuStrip.SuspendLayout();
			this.SuspendLayout();
			// 
			// treeViewAdv
			// 
			this.treeViewAdv.BackColor = System.Drawing.SystemColors.Window;
			this.treeViewAdv.Columns.Add(this.treeColumn1);
			this.treeViewAdv.DefaultToolTipProvider = null;
			this.treeViewAdv.Dock = System.Windows.Forms.DockStyle.Fill;
			this.treeViewAdv.DragDropMarkColor = System.Drawing.Color.Black;
			this.treeViewAdv.LineColor = System.Drawing.SystemColors.ControlDark;
			this.treeViewAdv.Location = new System.Drawing.Point(0, 0);
			this.treeViewAdv.Model = null;
			this.treeViewAdv.Name = "treeViewAdv";
			this.treeViewAdv.NodeControls.Add(this.nodeTextBox1);
			this.treeViewAdv.SelectedNode = null;
			this.treeViewAdv.Size = new System.Drawing.Size(292, 271);
			this.treeViewAdv.TabIndex = 0;
			this.treeViewAdv.Text = "treeViewAdv";
			this.treeViewAdv.MouseDown += new System.Windows.Forms.MouseEventHandler(this.treeViewAdv1_MouseDown);
			// 
			// treeColumn1
			// 
			this.treeColumn1.Header = "";
			this.treeColumn1.SortOrder = System.Windows.Forms.SortOrder.None;
			this.treeColumn1.TooltipText = null;
			// 
			// nodeTextBox1
			// 
			this.nodeTextBox1.DataPropertyName = "Text";
			this.nodeTextBox1.EditEnabled = true;
			this.nodeTextBox1.IncrementalSearchEnabled = true;
			this.nodeTextBox1.LeftMargin = 3;
			this.nodeTextBox1.ParentColumn = this.treeColumn1;
			// 
			// mediaPathMenuStrip
			// 
			this.mediaPathMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.addPathToolStripMenuItem});
			this.mediaPathMenuStrip.Name = "mediaPathMenuStrip";
			this.mediaPathMenuStrip.Size = new System.Drawing.Size(119, 26);
			// 
			// addPathToolStripMenuItem
			// 
			this.addPathToolStripMenuItem.Name = "addPathToolStripMenuItem";
			this.addPathToolStripMenuItem.Size = new System.Drawing.Size(118, 22);
			this.addPathToolStripMenuItem.Text = "Add path";
			this.addPathToolStripMenuItem.Click += new System.EventHandler(this.addPathToolStripMenuItem_Click);
			// 
			// sceneMenuStrip
			// 
			this.sceneMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.addSceneToolStripMenuItem});
			this.sceneMenuStrip.Name = "sceneMenuStrip";
			this.sceneMenuStrip.Size = new System.Drawing.Size(125, 26);
			// 
			// addSceneToolStripMenuItem
			// 
			this.addSceneToolStripMenuItem.Name = "addSceneToolStripMenuItem";
			this.addSceneToolStripMenuItem.Size = new System.Drawing.Size(124, 22);
			this.addSceneToolStripMenuItem.Text = "Add scene";
			this.addSceneToolStripMenuItem.Click += new System.EventHandler(this.addSceneToolStripMenuItem_Click);
			// 
			// ProjectWindow
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(292, 271);
			this.Controls.Add(this.treeViewAdv);
			this.DockAreas = ((WeifenLuo.WinFormsUI.Docking.DockAreas)((((((WeifenLuo.WinFormsUI.Docking.DockAreas.Float | WeifenLuo.WinFormsUI.Docking.DockAreas.DockLeft)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.DockRight)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.DockTop)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.DockBottom)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.Document)));
			this.HideOnClose = true;
			this.Name = "ProjectWindow";
			this.ShowHint = WeifenLuo.WinFormsUI.Docking.DockState.DockLeft;
			this.TabText = "Project";
			this.Text = "Project";
			this.Load += new System.EventHandler(this.ProjectWindow_Load);
			this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.ProjectWindow_FormClosing);
			this.mediaPathMenuStrip.ResumeLayout(false);
			this.sceneMenuStrip.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private Aga.Controls.Tree.TreeColumn treeColumn1;
		private Aga.Controls.Tree.NodeControls.NodeTextBox nodeTextBox1;
		private System.Windows.Forms.ContextMenuStrip mediaPathMenuStrip;
		private System.Windows.Forms.ToolStripMenuItem addPathToolStripMenuItem;
		private System.Windows.Forms.ContextMenuStrip sceneMenuStrip;
		private System.Windows.Forms.ToolStripMenuItem addSceneToolStripMenuItem;
		public Aga.Controls.Tree.TreeViewAdv treeViewAdv;
	}
}