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
			this.nodeIcon1 = new Aga.Controls.Tree.NodeControls.NodeIcon();
			this.nodeTextBox1 = new Aga.Controls.Tree.NodeControls.NodeTextBox();
			this.menuStripMediaPaths = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.addPathToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.menuStripScenes = new System.Windows.Forms.ContextMenuStrip(this.components);
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
			this.menuStripScene = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.addScriptToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.deleteSceneToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
			this.openToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.runToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.menuStripScript = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.setAsStartupScriptToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.deleteToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
			this.openToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
			this.menuStripMediaPaths.SuspendLayout();
			this.menuStripScenes.SuspendLayout();
			this.menuStripMain.SuspendLayout();
			this.menuStripScene.SuspendLayout();
			this.menuStripScript.SuspendLayout();
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
			this.treeViewAdv.NodeControls.Add(this.nodeIcon1);
			this.treeViewAdv.NodeControls.Add(this.nodeTextBox1);
			this.treeViewAdv.SelectedNode = null;
			this.treeViewAdv.ShowNodeToolTips = true;
			this.treeViewAdv.Size = new System.Drawing.Size(292, 250);
			this.treeViewAdv.TabIndex = 0;
			this.treeViewAdv.Text = "treeViewAdv";
			this.treeViewAdv.DoubleClick += new System.EventHandler(this.treeViewAdv_DoubleClick);
			this.treeViewAdv.MouseDown += new System.Windows.Forms.MouseEventHandler(this.treeViewAdv1_MouseDown);
			// 
			// treeColumn1
			// 
			this.treeColumn1.Header = "";
			this.treeColumn1.SortOrder = System.Windows.Forms.SortOrder.None;
			this.treeColumn1.TooltipText = null;
			// 
			// nodeIcon1
			// 
			this.nodeIcon1.DataPropertyName = "Icon";
			this.nodeIcon1.LeftMargin = 1;
			this.nodeIcon1.ParentColumn = this.treeColumn1;
			this.nodeIcon1.ScaleMode = Aga.Controls.Tree.ImageScaleMode.AlwaysScale;
			// 
			// nodeTextBox1
			// 
			this.nodeTextBox1.DataPropertyName = "Text";
			this.nodeTextBox1.EditEnabled = true;
			this.nodeTextBox1.IncrementalSearchEnabled = true;
			this.nodeTextBox1.LeftMargin = 3;
			this.nodeTextBox1.ParentColumn = this.treeColumn1;
			// 
			// menuStripMediaPaths
			// 
			this.menuStripMediaPaths.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.addPathToolStripMenuItem});
			this.menuStripMediaPaths.Name = "mediaPathMenuStrip";
			this.menuStripMediaPaths.Size = new System.Drawing.Size(130, 26);
			// 
			// addPathToolStripMenuItem
			// 
			this.addPathToolStripMenuItem.Name = "addPathToolStripMenuItem";
			this.addPathToolStripMenuItem.Size = new System.Drawing.Size(129, 22);
			this.addPathToolStripMenuItem.Text = "Add path";
			this.addPathToolStripMenuItem.Click += new System.EventHandler(this.addPathToolStripMenuItem_Click);
			// 
			// menuStripScenes
			// 
			this.menuStripScenes.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.addSceneToolStripMenuItem});
			this.menuStripScenes.Name = "sceneMenuStrip";
			this.menuStripScenes.Size = new System.Drawing.Size(136, 26);
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
			// menuStripScene
			// 
			this.menuStripScene.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.addScriptToolStripMenuItem,
            this.deleteSceneToolStripMenuItem,
            this.toolStripSeparator2,
            this.openToolStripMenuItem,
            this.runToolStripMenuItem});
			this.menuStripScene.Name = "sceneMenuStrip";
			this.menuStripScene.Size = new System.Drawing.Size(134, 98);
			// 
			// addScriptToolStripMenuItem
			// 
			this.addScriptToolStripMenuItem.Name = "addScriptToolStripMenuItem";
			this.addScriptToolStripMenuItem.Size = new System.Drawing.Size(133, 22);
			this.addScriptToolStripMenuItem.Text = "Add script";
			this.addScriptToolStripMenuItem.Click += new System.EventHandler(this.addSceneScriptToolStripMenuItem_Click);
			// 
			// deleteSceneToolStripMenuItem
			// 
			this.deleteSceneToolStripMenuItem.Name = "deleteSceneToolStripMenuItem";
			this.deleteSceneToolStripMenuItem.Size = new System.Drawing.Size(133, 22);
			this.deleteSceneToolStripMenuItem.Text = "Delete";
			this.deleteSceneToolStripMenuItem.Click += new System.EventHandler(this.deleteSceneToolStripMenuItem_Click);
			// 
			// toolStripSeparator2
			// 
			this.toolStripSeparator2.Name = "toolStripSeparator2";
			this.toolStripSeparator2.Size = new System.Drawing.Size(130, 6);
			// 
			// openToolStripMenuItem
			// 
			this.openToolStripMenuItem.Name = "openToolStripMenuItem";
			this.openToolStripMenuItem.Size = new System.Drawing.Size(133, 22);
			this.openToolStripMenuItem.Text = "Open";
			this.openToolStripMenuItem.Click += new System.EventHandler(this.openSceneToolStripMenuItem_Click);
			// 
			// runToolStripMenuItem
			// 
			this.runToolStripMenuItem.Name = "runToolStripMenuItem";
			this.runToolStripMenuItem.Size = new System.Drawing.Size(133, 22);
			this.runToolStripMenuItem.Text = "Run";
			// 
			// menuStripScript
			// 
			this.menuStripScript.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.setAsStartupScriptToolStripMenuItem,
            this.deleteToolStripMenuItem,
            this.toolStripSeparator3,
            this.openToolStripMenuItem1});
			this.menuStripScript.Name = "menuStripScript";
			this.menuStripScript.Size = new System.Drawing.Size(185, 76);
			// 
			// setAsStartupScriptToolStripMenuItem
			// 
			this.setAsStartupScriptToolStripMenuItem.Name = "setAsStartupScriptToolStripMenuItem";
			this.setAsStartupScriptToolStripMenuItem.Size = new System.Drawing.Size(184, 22);
			this.setAsStartupScriptToolStripMenuItem.Text = "Set as startup script";
			this.setAsStartupScriptToolStripMenuItem.Click += new System.EventHandler(this.setAsStartupScriptToolStripMenuItem_Click);
			// 
			// deleteToolStripMenuItem
			// 
			this.deleteToolStripMenuItem.Name = "deleteToolStripMenuItem";
			this.deleteToolStripMenuItem.Size = new System.Drawing.Size(184, 22);
			this.deleteToolStripMenuItem.Text = "Delete";
			this.deleteToolStripMenuItem.Click += new System.EventHandler(this.deleteToolStripMenuItem_Click);
			// 
			// toolStripSeparator3
			// 
			this.toolStripSeparator3.Name = "toolStripSeparator3";
			this.toolStripSeparator3.Size = new System.Drawing.Size(181, 6);
			// 
			// openToolStripMenuItem1
			// 
			this.openToolStripMenuItem1.Name = "openToolStripMenuItem1";
			this.openToolStripMenuItem1.Size = new System.Drawing.Size(184, 22);
			this.openToolStripMenuItem1.Text = "Open";
			this.openToolStripMenuItem1.Click += new System.EventHandler(this.openScriptToolStripMenuItem_Click);
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
			this.menuStripMediaPaths.ResumeLayout(false);
			this.menuStripScenes.ResumeLayout(false);
			this.menuStripMain.ResumeLayout(false);
			this.menuStripMain.PerformLayout();
			this.menuStripScene.ResumeLayout(false);
			this.menuStripScript.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private Aga.Controls.Tree.TreeColumn treeColumn1;
		private Aga.Controls.Tree.NodeControls.NodeTextBox nodeTextBox1;
		private System.Windows.Forms.ContextMenuStrip menuStripMediaPaths;
		private System.Windows.Forms.ToolStripMenuItem addPathToolStripMenuItem;
		private System.Windows.Forms.ContextMenuStrip menuStripScenes;
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
		private System.Windows.Forms.ContextMenuStrip menuStripScene;
		private System.Windows.Forms.ToolStripMenuItem addScriptToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem deleteSceneToolStripMenuItem;
		private System.Windows.Forms.ContextMenuStrip menuStripScript;
		private System.Windows.Forms.ToolStripMenuItem setAsStartupScriptToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem deleteToolStripMenuItem;
		private Aga.Controls.Tree.NodeControls.NodeIcon nodeIcon1;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
		private System.Windows.Forms.ToolStripMenuItem openToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem runToolStripMenuItem;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
		private System.Windows.Forms.ToolStripMenuItem openToolStripMenuItem1;
	}
}