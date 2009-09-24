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
			this.menuStrip1 = new System.Windows.Forms.MenuStrip();
			this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.newProjectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.openProjectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.recentProjectsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.saveProjectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.saveProjectAsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
			this.mediaPathMenuStrip.SuspendLayout();
			this.sceneMenuStrip.SuspendLayout();
			this.menuStrip1.SuspendLayout();
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
			this.treeViewAdv.Size = new System.Drawing.Size(292, 250);
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
			this.mediaPathMenuStrip.Size = new System.Drawing.Size(130, 26);
			// 
			// addPathToolStripMenuItem
			// 
			this.addPathToolStripMenuItem.Name = "addPathToolStripMenuItem";
			this.addPathToolStripMenuItem.Size = new System.Drawing.Size(129, 22);
			this.addPathToolStripMenuItem.Text = "Add path";
			this.addPathToolStripMenuItem.Click += new System.EventHandler(this.addPathToolStripMenuItem_Click);
			// 
			// sceneMenuStrip
			// 
			this.sceneMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.addSceneToolStripMenuItem});
			this.sceneMenuStrip.Name = "sceneMenuStrip";
			this.sceneMenuStrip.Size = new System.Drawing.Size(136, 26);
			// 
			// addSceneToolStripMenuItem
			// 
			this.addSceneToolStripMenuItem.Name = "addSceneToolStripMenuItem";
			this.addSceneToolStripMenuItem.Size = new System.Drawing.Size(135, 22);
			this.addSceneToolStripMenuItem.Text = "Add scene";
			this.addSceneToolStripMenuItem.Click += new System.EventHandler(this.addSceneToolStripMenuItem_Click);
			// 
			// menuStrip1
			// 
			this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
			this.menuStrip1.Location = new System.Drawing.Point(0, 0);
			this.menuStrip1.Name = "menuStrip1";
			this.menuStrip1.Size = new System.Drawing.Size(292, 24);
			this.menuStrip1.TabIndex = 2;
			this.menuStrip1.Text = "menuStrip1";
			this.menuStrip1.Visible = false;
			// 
			// fileToolStripMenuItem
			// 
			this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.newProjectToolStripMenuItem,
            this.openProjectToolStripMenuItem,
            this.recentProjectsToolStripMenuItem,
            this.saveProjectToolStripMenuItem,
            this.saveProjectAsToolStripMenuItem,
            this.toolStripSeparator1});
			this.fileToolStripMenuItem.MergeAction = System.Windows.Forms.MergeAction.MatchOnly;
			this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
			this.fileToolStripMenuItem.Size = new System.Drawing.Size(39, 20);
			this.fileToolStripMenuItem.Text = "&File";
			// 
			// newProjectToolStripMenuItem
			// 
			this.newProjectToolStripMenuItem.MergeAction = System.Windows.Forms.MergeAction.Insert;
			this.newProjectToolStripMenuItem.MergeIndex = 0;
			this.newProjectToolStripMenuItem.Name = "newProjectToolStripMenuItem";
			this.newProjectToolStripMenuItem.Size = new System.Drawing.Size(170, 22);
			this.newProjectToolStripMenuItem.Text = "New project...";
			this.newProjectToolStripMenuItem.Click += new System.EventHandler(this.newProjectToolStripMenuItem_Click);
			// 
			// openProjectToolStripMenuItem
			// 
			this.openProjectToolStripMenuItem.MergeAction = System.Windows.Forms.MergeAction.Insert;
			this.openProjectToolStripMenuItem.MergeIndex = 1;
			this.openProjectToolStripMenuItem.Name = "openProjectToolStripMenuItem";
			this.openProjectToolStripMenuItem.Size = new System.Drawing.Size(170, 22);
			this.openProjectToolStripMenuItem.Text = "Open project...";
			this.openProjectToolStripMenuItem.Click += new System.EventHandler(this.openProjectToolStripMenuItem_Click);
			// 
			// recentProjectsToolStripMenuItem
			// 
			this.recentProjectsToolStripMenuItem.MergeAction = System.Windows.Forms.MergeAction.Insert;
			this.recentProjectsToolStripMenuItem.MergeIndex = 2;
			this.recentProjectsToolStripMenuItem.Name = "recentProjectsToolStripMenuItem";
			this.recentProjectsToolStripMenuItem.Size = new System.Drawing.Size(170, 22);
			this.recentProjectsToolStripMenuItem.Text = "Recent projects";
			// 
			// saveProjectToolStripMenuItem
			// 
			this.saveProjectToolStripMenuItem.MergeAction = System.Windows.Forms.MergeAction.Insert;
			this.saveProjectToolStripMenuItem.MergeIndex = 3;
			this.saveProjectToolStripMenuItem.Name = "saveProjectToolStripMenuItem";
			this.saveProjectToolStripMenuItem.Size = new System.Drawing.Size(170, 22);
			this.saveProjectToolStripMenuItem.Text = "Save project";
			this.saveProjectToolStripMenuItem.Click += new System.EventHandler(this.saveProjectToolStripMenuItem_Click);
			// 
			// saveProjectAsToolStripMenuItem
			// 
			this.saveProjectAsToolStripMenuItem.MergeAction = System.Windows.Forms.MergeAction.Insert;
			this.saveProjectAsToolStripMenuItem.MergeIndex = 4;
			this.saveProjectAsToolStripMenuItem.Name = "saveProjectAsToolStripMenuItem";
			this.saveProjectAsToolStripMenuItem.Size = new System.Drawing.Size(170, 22);
			this.saveProjectAsToolStripMenuItem.Text = "Save project as...";
			// 
			// toolStripSeparator1
			// 
			this.toolStripSeparator1.MergeAction = System.Windows.Forms.MergeAction.Insert;
			this.toolStripSeparator1.MergeIndex = 5;
			this.toolStripSeparator1.Name = "toolStripSeparator1";
			this.toolStripSeparator1.Size = new System.Drawing.Size(167, 6);
			// 
			// ProjectWindow
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(292, 250);
			this.Controls.Add(this.menuStrip1);
			this.Controls.Add(this.treeViewAdv);
			this.DockAreas = ((WeifenLuo.WinFormsUI.Docking.DockAreas)((((((WeifenLuo.WinFormsUI.Docking.DockAreas.Float | WeifenLuo.WinFormsUI.Docking.DockAreas.DockLeft)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.DockRight)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.DockTop)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.DockBottom)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.Document)));
			this.HideOnClose = true;
			this.MainMenuStrip = this.menuStrip1;
			this.Name = "ProjectWindow";
			this.ShowHint = WeifenLuo.WinFormsUI.Docking.DockState.DockLeft;
			this.TabText = "Project";
			this.Text = "Project";
			this.Load += new System.EventHandler(this.ProjectWindow_Load);
			this.mediaPathMenuStrip.ResumeLayout(false);
			this.sceneMenuStrip.ResumeLayout(false);
			this.menuStrip1.ResumeLayout(false);
			this.menuStrip1.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private Aga.Controls.Tree.TreeColumn treeColumn1;
		private Aga.Controls.Tree.NodeControls.NodeTextBox nodeTextBox1;
		private System.Windows.Forms.ContextMenuStrip mediaPathMenuStrip;
		private System.Windows.Forms.ToolStripMenuItem addPathToolStripMenuItem;
		private System.Windows.Forms.ContextMenuStrip sceneMenuStrip;
		private System.Windows.Forms.ToolStripMenuItem addSceneToolStripMenuItem;
		public Aga.Controls.Tree.TreeViewAdv treeViewAdv;
		private System.Windows.Forms.MenuStrip menuStrip1;
		private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem newProjectToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem openProjectToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem recentProjectsToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem saveProjectToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem saveProjectAsToolStripMenuItem;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
	}
}