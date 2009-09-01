﻿namespace Studio
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
			this.mainMenu = new System.Windows.Forms.MenuStrip();
			this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.newToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.openToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.editToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.dockPanel = new WeifenLuo.WinFormsUI.Docking.DockPanel();
			this.toolStripContainer1 = new System.Windows.Forms.ToolStripContainer();
			this.toolStripMain = new System.Windows.Forms.ToolStrip();
			this.newToolStripButton = new System.Windows.Forms.ToolStripButton();
			this.openToolStripButton = new System.Windows.Forms.ToolStripButton();
			this.saveToolStripButton = new System.Windows.Forms.ToolStripButton();
			this.printToolStripButton = new System.Windows.Forms.ToolStripButton();
			this.toolStripSeparator = new System.Windows.Forms.ToolStripSeparator();
			this.cutToolStripButton = new System.Windows.Forms.ToolStripButton();
			this.copyToolStripButton = new System.Windows.Forms.ToolStripButton();
			this.pasteToolStripButton = new System.Windows.Forms.ToolStripButton();
			this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
			this.helpToolStripButton = new System.Windows.Forms.ToolStripButton();
			this.toolStripGizmo = new System.Windows.Forms.ToolStrip();
			this.toolStripButtonMove = new System.Windows.Forms.ToolStripButton();
			this.toolStripButtonTranslate = new System.Windows.Forms.ToolStripButton();
			this.toolStripButtonRotate = new System.Windows.Forms.ToolStripButton();
			this.toolStripButtonScale = new System.Windows.Forms.ToolStripButton();
			this.toolStripDebug = new System.Windows.Forms.ToolStrip();
			this.toolStripButtonPlay = new System.Windows.Forms.ToolStripButton();
			this.mainMenu.SuspendLayout();
			this.toolStripContainer1.ContentPanel.SuspendLayout();
			this.toolStripContainer1.TopToolStripPanel.SuspendLayout();
			this.toolStripContainer1.SuspendLayout();
			this.toolStripMain.SuspendLayout();
			this.toolStripGizmo.SuspendLayout();
			this.toolStripDebug.SuspendLayout();
			this.SuspendLayout();
			// 
			// mainMenu
			// 
			this.mainMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
			this.fileToolStripMenuItem,
			this.editToolStripMenuItem,
			this.helpToolStripMenuItem});
			this.mainMenu.Location = new System.Drawing.Point(0, 0);
			this.mainMenu.Name = "mainMenu";
			this.mainMenu.Size = new System.Drawing.Size(792, 24);
			this.mainMenu.TabIndex = 1;
			this.mainMenu.Text = "menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
			this.newToolStripMenuItem,
			this.openToolStripMenuItem,
			this.exitToolStripMenuItem});
			this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
			this.fileToolStripMenuItem.Size = new System.Drawing.Size(35, 20);
			this.fileToolStripMenuItem.Text = "&File";
			// 
			// newToolStripMenuItem
			// 
			this.newToolStripMenuItem.Name = "newToolStripMenuItem";
			this.newToolStripMenuItem.Size = new System.Drawing.Size(100, 22);
			this.newToolStripMenuItem.Text = "New";
			this.newToolStripMenuItem.Click += new System.EventHandler(this.newToolStripMenuItem_Click);
			// 
			// openToolStripMenuItem
			// 
			this.openToolStripMenuItem.Name = "openToolStripMenuItem";
			this.openToolStripMenuItem.Size = new System.Drawing.Size(100, 22);
			this.openToolStripMenuItem.Text = "Open";
			// 
			// exitToolStripMenuItem
			// 
			this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
			this.exitToolStripMenuItem.Size = new System.Drawing.Size(100, 22);
			this.exitToolStripMenuItem.Text = "E&xit";
			this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
			// 
			// editToolStripMenuItem
			// 
			this.editToolStripMenuItem.Name = "editToolStripMenuItem";
			this.editToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
			this.editToolStripMenuItem.Text = "&Edit";
			// 
			// helpToolStripMenuItem
			// 
			this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
			this.aboutToolStripMenuItem});
			this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
			this.helpToolStripMenuItem.Size = new System.Drawing.Size(40, 20);
			this.helpToolStripMenuItem.Text = "&Help";
			// 
			// aboutToolStripMenuItem
			// 
			this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
			this.aboutToolStripMenuItem.Size = new System.Drawing.Size(103, 22);
			this.aboutToolStripMenuItem.Text = "About";
			// 
			// dockPanel
			// 
			this.dockPanel.ActiveAutoHideContent = null;
			this.dockPanel.Dock = System.Windows.Forms.DockStyle.Fill;
			this.dockPanel.DocumentStyle = WeifenLuo.WinFormsUI.Docking.DocumentStyle.DockingWindow;
			this.dockPanel.Location = new System.Drawing.Point(0, 0);
			this.dockPanel.Name = "dockPanel";
			this.dockPanel.Size = new System.Drawing.Size(792, 522);
			this.dockPanel.TabIndex = 2;
			// 
			// toolStripContainer1
			// 
			// 
			// toolStripContainer1.ContentPanel
			// 
			this.toolStripContainer1.ContentPanel.Controls.Add(this.dockPanel);
			this.toolStripContainer1.ContentPanel.Size = new System.Drawing.Size(792, 522);
			this.toolStripContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.toolStripContainer1.Location = new System.Drawing.Point(0, 24);
			this.toolStripContainer1.Name = "toolStripContainer1";
			this.toolStripContainer1.Size = new System.Drawing.Size(792, 547);
			this.toolStripContainer1.TabIndex = 3;
			this.toolStripContainer1.Text = "toolStripContainer1";
			// 
			// toolStripContainer1.TopToolStripPanel
			// 
			this.toolStripContainer1.TopToolStripPanel.AccessibleRole = System.Windows.Forms.AccessibleRole.ToolBar;
			this.toolStripContainer1.TopToolStripPanel.Controls.Add(this.toolStripMain);
			this.toolStripContainer1.TopToolStripPanel.Controls.Add(this.toolStripGizmo);
			this.toolStripContainer1.TopToolStripPanel.Controls.Add(this.toolStripDebug);
			this.toolStripContainer1.TopToolStripPanel.MaximumSize = new System.Drawing.Size(0, 25);
			// 
			// toolStripMain
			// 
			this.toolStripMain.AccessibleRole = System.Windows.Forms.AccessibleRole.ToolBar;
			this.toolStripMain.Dock = System.Windows.Forms.DockStyle.None;
			this.toolStripMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
			this.newToolStripButton,
			this.openToolStripButton,
			this.saveToolStripButton,
			this.printToolStripButton,
			this.toolStripSeparator,
			this.cutToolStripButton,
			this.copyToolStripButton,
			this.pasteToolStripButton,
			this.toolStripSeparator1,
			this.helpToolStripButton});
			this.toolStripMain.Location = new System.Drawing.Point(3, 0);
			this.toolStripMain.Name = "toolStripMain";
			this.toolStripMain.Size = new System.Drawing.Size(208, 25);
			this.toolStripMain.TabIndex = 0;
			this.toolStripMain.ItemClicked += new System.Windows.Forms.ToolStripItemClickedEventHandler(this.toolStripMain_ItemClicked);
			// 
			// newToolStripButton
			// 
			this.newToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.newToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("newToolStripButton.Image")));
			this.newToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.newToolStripButton.Name = "newToolStripButton";
			this.newToolStripButton.Size = new System.Drawing.Size(23, 22);
			this.newToolStripButton.Tag = this.newToolStripMenuItem;
			this.newToolStripButton.Text = "&New";
			// 
			// openToolStripButton
			// 
			this.openToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.openToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("openToolStripButton.Image")));
			this.openToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.openToolStripButton.Name = "openToolStripButton";
			this.openToolStripButton.Size = new System.Drawing.Size(23, 22);
			this.openToolStripButton.Text = "&Open";
			// 
			// saveToolStripButton
			// 
			this.saveToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.saveToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("saveToolStripButton.Image")));
			this.saveToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.saveToolStripButton.Name = "saveToolStripButton";
			this.saveToolStripButton.Size = new System.Drawing.Size(23, 22);
			this.saveToolStripButton.Text = "&Save";
			// 
			// printToolStripButton
			// 
			this.printToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.printToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("printToolStripButton.Image")));
			this.printToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.printToolStripButton.Name = "printToolStripButton";
			this.printToolStripButton.Size = new System.Drawing.Size(23, 22);
			this.printToolStripButton.Text = "&Print";
			// 
			// toolStripSeparator
			// 
			this.toolStripSeparator.Name = "toolStripSeparator";
			this.toolStripSeparator.Size = new System.Drawing.Size(6, 25);
			// 
			// cutToolStripButton
			// 
			this.cutToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.cutToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("cutToolStripButton.Image")));
			this.cutToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.cutToolStripButton.Name = "cutToolStripButton";
			this.cutToolStripButton.Size = new System.Drawing.Size(23, 22);
			this.cutToolStripButton.Text = "C&ut";
			// 
			// copyToolStripButton
			// 
			this.copyToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.copyToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("copyToolStripButton.Image")));
			this.copyToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.copyToolStripButton.Name = "copyToolStripButton";
			this.copyToolStripButton.Size = new System.Drawing.Size(23, 22);
			this.copyToolStripButton.Text = "&Copy";
			// 
			// pasteToolStripButton
			// 
			this.pasteToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.pasteToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("pasteToolStripButton.Image")));
			this.pasteToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.pasteToolStripButton.Name = "pasteToolStripButton";
			this.pasteToolStripButton.Size = new System.Drawing.Size(23, 22);
			this.pasteToolStripButton.Text = "&Paste";
			// 
			// toolStripSeparator1
			// 
			this.toolStripSeparator1.Name = "toolStripSeparator1";
			this.toolStripSeparator1.Size = new System.Drawing.Size(6, 25);
			// 
			// helpToolStripButton
			// 
			this.helpToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.helpToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("helpToolStripButton.Image")));
			this.helpToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.helpToolStripButton.Name = "helpToolStripButton";
			this.helpToolStripButton.Size = new System.Drawing.Size(23, 22);
			this.helpToolStripButton.Text = "He&lp";
			// 
			// toolStripGizmo
			// 
			this.toolStripGizmo.AccessibleRole = System.Windows.Forms.AccessibleRole.ToolBar;
			this.toolStripGizmo.Dock = System.Windows.Forms.DockStyle.None;
			this.toolStripGizmo.Enabled = false;
			this.toolStripGizmo.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
			this.toolStripButtonMove,
			this.toolStripButtonTranslate,
			this.toolStripButtonRotate,
			this.toolStripButtonScale});
			this.toolStripGizmo.Location = new System.Drawing.Point(211, 0);
			this.toolStripGizmo.Name = "toolStripGizmo";
			this.toolStripGizmo.Size = new System.Drawing.Size(104, 25);
			this.toolStripGizmo.TabIndex = 1;
			this.toolStripGizmo.ItemClicked += new System.Windows.Forms.ToolStripItemClickedEventHandler(this.toolStripGizmo_ItemClicked);
			// 
			// toolStripButtonMove
			// 
			this.toolStripButtonMove.CheckOnClick = true;
			this.toolStripButtonMove.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.toolStripButtonMove.Enabled = false;
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
			this.toolStripButtonTranslate.Enabled = false;
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
			this.toolStripButtonRotate.Enabled = false;
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
			this.toolStripButtonScale.Enabled = false;
			this.toolStripButtonScale.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonScale.Image")));
			this.toolStripButtonScale.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButtonScale.Name = "toolStripButtonScale";
			this.toolStripButtonScale.Size = new System.Drawing.Size(23, 22);
			this.toolStripButtonScale.Text = "Scale";
			this.toolStripButtonScale.Click += new System.EventHandler(this.toolStripButtonScale_Click);
			// 
			// toolStripDebug
			// 
			this.toolStripDebug.Dock = System.Windows.Forms.DockStyle.None;
			this.toolStripDebug.Enabled = false;
			this.toolStripDebug.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
			this.toolStripButtonPlay});
			this.toolStripDebug.Location = new System.Drawing.Point(315, 0);
			this.toolStripDebug.Name = "toolStripDebug";
			this.toolStripDebug.Size = new System.Drawing.Size(66, 25);
			this.toolStripDebug.TabIndex = 2;
			// 
			// toolStripButtonPlay
			// 
			this.toolStripButtonPlay.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.toolStripButtonPlay.Image = ((System.Drawing.Image)(resources.GetObject("playToolStripButton.Image")));
			this.toolStripButtonPlay.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButtonPlay.Name = "toolStripButtonPlay";
			this.toolStripButtonPlay.Size = new System.Drawing.Size(23, 22);
			this.toolStripButtonPlay.Text = "toolStripButton1";
			this.toolStripButtonPlay.Click += new System.EventHandler(this.toolStripButtonPlay_Click);
			// 
			// MainForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(792, 571);
			this.Controls.Add(this.toolStripContainer1);
			this.Controls.Add(this.mainMenu);
			this.MainMenuStrip = this.mainMenu;
			this.Name = "MainForm";
			this.Text = "MCore3d Studio";
			this.Load += new System.EventHandler(this.MainForm_Load);
			this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.MainForm_FormClosed);
			this.mainMenu.ResumeLayout(false);
			this.mainMenu.PerformLayout();
			this.toolStripContainer1.ContentPanel.ResumeLayout(false);
			this.toolStripContainer1.TopToolStripPanel.ResumeLayout(false);
			this.toolStripContainer1.TopToolStripPanel.PerformLayout();
			this.toolStripContainer1.ResumeLayout(false);
			this.toolStripContainer1.PerformLayout();
			this.toolStripMain.ResumeLayout(false);
			this.toolStripMain.PerformLayout();
			this.toolStripGizmo.ResumeLayout(false);
			this.toolStripGizmo.PerformLayout();
			this.toolStripDebug.ResumeLayout(false);
			this.toolStripDebug.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.MenuStrip mainMenu;
		private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem newToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem openToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
		private WeifenLuo.WinFormsUI.Docking.DockPanel dockPanel;
		private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem editToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
		private System.Windows.Forms.ToolStripContainer toolStripContainer1;
		private System.Windows.Forms.ToolStrip toolStripMain;
		private System.Windows.Forms.ToolStripButton newToolStripButton;
		private System.Windows.Forms.ToolStripButton openToolStripButton;
		private System.Windows.Forms.ToolStripButton saveToolStripButton;
		private System.Windows.Forms.ToolStripButton printToolStripButton;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator;
		private System.Windows.Forms.ToolStripButton cutToolStripButton;
		private System.Windows.Forms.ToolStripButton copyToolStripButton;
		private System.Windows.Forms.ToolStripButton pasteToolStripButton;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
		private System.Windows.Forms.ToolStripButton helpToolStripButton;
		private System.Windows.Forms.ToolStrip toolStripGizmo;
		private System.Windows.Forms.ToolStripButton toolStripButtonMove;
		private System.Windows.Forms.ToolStripButton toolStripButtonTranslate;
		private System.Windows.Forms.ToolStripButton toolStripButtonRotate;
		private System.Windows.Forms.ToolStripButton toolStripButtonScale;
		private System.Windows.Forms.ToolStrip toolStripDebug;
		private System.Windows.Forms.ToolStripButton toolStripButtonPlay;
	}
}