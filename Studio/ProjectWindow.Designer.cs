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
			this.menuStripMediaPath = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.addPathToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.menuStripScene = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.addSceneToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.menuStripMain = new System.Windows.Forms.MenuStrip();
			this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.newProjectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.openProjectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.recentProjectsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.saveProjectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.saveProjectAsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.closeProjectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
			this.menuStripAddScript = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.addSceneScriptToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.addRunScriptToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.deleteSceneToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.menuStripMediaPath.SuspendLayout();
			this.menuStripScene.SuspendLayout();
			this.menuStripMain.SuspendLayout();
			this.menuStripAddScript.SuspendLayout();
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
			// menuStripMediaPath
			// 
			this.menuStripMediaPath.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.addPathToolStripMenuItem});
			this.menuStripMediaPath.Name = "mediaPathMenuStrip";
			this.menuStripMediaPath.Size = new System.Drawing.Size(130, 26);
			// 
			// addPathToolStripMenuItem
			// 
			this.addPathToolStripMenuItem.Name = "addPathToolStripMenuItem";
			this.addPathToolStripMenuItem.Size = new System.Drawing.Size(129, 22);
			this.addPathToolStripMenuItem.Text = "Add path";
			this.addPathToolStripMenuItem.Click += new System.EventHandler(this.addPathToolStripMenuItem_Click);
			// 
			// menuStripScene
			// 
			this.menuStripScene.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.addSceneToolStripMenuItem});
			this.menuStripScene.Name = "sceneMenuStrip";
			this.menuStripScene.Size = new System.Drawing.Size(136, 26);
			// 
			// addSceneToolStripMenuItem
			// 
			this.addSceneToolStripMenuItem.Name = "addSceneToolStripMenuItem";
			this.addSceneToolStripMenuItem.Size = new System.Drawing.Size(135, 22);
			this.addSceneToolStripMenuItem.Text = "Add scene";
			this.addSceneToolStripMenuItem.Click += new System.EventHandler(this.addSceneToolStripMenuItem_Click);
			// 
			// menuStripMain
			// 
			this.menuStripMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
			this.menuStripMain.Location = new System.Drawing.Point(0, 0);
			this.menuStripMain.Name = "menuStripMain";
			this.menuStripMain.Size = new System.Drawing.Size(292, 24);
			this.menuStripMain.TabIndex = 2;
			this.menuStripMain.Text = "menuStrip1";
			this.menuStripMain.Visible = false;
			// 
			// fileToolStripMenuItem
			// 
			this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.newProjectToolStripMenuItem,
            this.openProjectToolStripMenuItem,
            this.recentProjectsToolStripMenuItem,
            this.saveProjectToolStripMenuItem,
            this.saveProjectAsToolStripMenuItem,
            this.closeProjectToolStripMenuItem,
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
			this.saveProjectAsToolStripMenuItem.Click += new System.EventHandler(this.saveProjectAsToolStripMenuItem_Click);
			// 
			// closeProjectToolStripMenuItem
			// 
			this.closeProjectToolStripMenuItem.MergeAction = System.Windows.Forms.MergeAction.Insert;
			this.closeProjectToolStripMenuItem.MergeIndex = 5;
			this.closeProjectToolStripMenuItem.Name = "closeProjectToolStripMenuItem";
			this.closeProjectToolStripMenuItem.Size = new System.Drawing.Size(170, 22);
			this.closeProjectToolStripMenuItem.Text = "Close project";
			this.closeProjectToolStripMenuItem.Click += new System.EventHandler(this.closeProjectToolStripMenuItem_Click);
			// 
			// toolStripSeparator1
			// 
			this.toolStripSeparator1.MergeAction = System.Windows.Forms.MergeAction.Insert;
			this.toolStripSeparator1.MergeIndex = 6;
			this.toolStripSeparator1.Name = "toolStripSeparator1";
			this.toolStripSeparator1.Size = new System.Drawing.Size(167, 6);
			// 
			// menuStripAddScript
			// 
			this.menuStripAddScript.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.addSceneScriptToolStripMenuItem,
            this.addRunScriptToolStripMenuItem,
            this.deleteSceneToolStripMenuItem});
			this.menuStripAddScript.Name = "sceneMenuStrip";
			this.menuStripAddScript.Size = new System.Drawing.Size(169, 92);
			// 
			// addSceneScriptToolStripMenuItem
			// 
			this.addSceneScriptToolStripMenuItem.Name = "addSceneScriptToolStripMenuItem";
			this.addSceneScriptToolStripMenuItem.Size = new System.Drawing.Size(168, 22);
			this.addSceneScriptToolStripMenuItem.Text = "Add scene script";
			this.addSceneScriptToolStripMenuItem.Click += new System.EventHandler(this.addSceneScriptToolStripMenuItem_Click);
			// 
			// addRunScriptToolStripMenuItem
			// 
			this.addRunScriptToolStripMenuItem.Name = "addRunScriptToolStripMenuItem";
			this.addRunScriptToolStripMenuItem.Size = new System.Drawing.Size(168, 22);
			this.addRunScriptToolStripMenuItem.Text = "Add run script";
			this.addRunScriptToolStripMenuItem.Click += new System.EventHandler(this.addRunScriptToolStripMenuItem_Click);
			// 
			// deleteSceneToolStripMenuItem
			// 
			this.deleteSceneToolStripMenuItem.Name = "deleteSceneToolStripMenuItem";
			this.deleteSceneToolStripMenuItem.Size = new System.Drawing.Size(168, 22);
			this.deleteSceneToolStripMenuItem.Text = "Delete";
			this.deleteSceneToolStripMenuItem.Click += new System.EventHandler(this.deleteSceneToolStripMenuItem_Click);
			// 
			// ProjectWindow
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(292, 250);
			this.Controls.Add(this.menuStripMain);
			this.Controls.Add(this.treeViewAdv);
			this.DockAreas = ((WeifenLuo.WinFormsUI.Docking.DockAreas)((((((WeifenLuo.WinFormsUI.Docking.DockAreas.Float | WeifenLuo.WinFormsUI.Docking.DockAreas.DockLeft)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.DockRight)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.DockTop)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.DockBottom)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.Document)));
			this.HideOnClose = true;
			this.MainMenuStrip = this.menuStripMain;
			this.Name = "ProjectWindow";
			this.ShowHint = WeifenLuo.WinFormsUI.Docking.DockState.DockLeft;
			this.TabText = "Project";
			this.Text = "Project";
			this.Load += new System.EventHandler(this.ProjectWindow_Load);
			this.menuStripMediaPath.ResumeLayout(false);
			this.menuStripScene.ResumeLayout(false);
			this.menuStripMain.ResumeLayout(false);
			this.menuStripMain.PerformLayout();
			this.menuStripAddScript.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private Aga.Controls.Tree.TreeColumn treeColumn1;
		private Aga.Controls.Tree.NodeControls.NodeTextBox nodeTextBox1;
		private System.Windows.Forms.ContextMenuStrip menuStripMediaPath;
		private System.Windows.Forms.ToolStripMenuItem addPathToolStripMenuItem;
		private System.Windows.Forms.ContextMenuStrip menuStripScene;
		private System.Windows.Forms.ToolStripMenuItem addSceneToolStripMenuItem;
		public Aga.Controls.Tree.TreeViewAdv treeViewAdv;
		private System.Windows.Forms.MenuStrip menuStripMain;
		private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem newProjectToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem openProjectToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem recentProjectsToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem saveProjectToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem saveProjectAsToolStripMenuItem;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
		private System.Windows.Forms.ToolStripMenuItem closeProjectToolStripMenuItem;
		private System.Windows.Forms.ContextMenuStrip menuStripAddScript;
		private System.Windows.Forms.ToolStripMenuItem addSceneScriptToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem addRunScriptToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem deleteSceneToolStripMenuItem;
	}
}