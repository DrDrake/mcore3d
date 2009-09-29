namespace Studio
{
	partial class AssertBrowsingForm
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
			System.Windows.Forms.Panel panel1;
			System.Windows.Forms.Panel panel2;
			System.Windows.Forms.Label label1;
			System.Windows.Forms.Panel panel3;
			System.Windows.Forms.SplitContainer splitContainer2;
			System.Windows.Forms.SplitContainer splitContainer1;
			this.listView1 = new System.Windows.Forms.ListView();
			this.textBoxFolderPath = new System.Windows.Forms.TextBox();
			this.buttonUpFolder = new System.Windows.Forms.Button();
			this.button1 = new System.Windows.Forms.Button();
			this.pictureBoxPreview = new System.Windows.Forms.PictureBox();
			this.treeView1 = new Studio.AssertTreeViewControl();
			panel1 = new System.Windows.Forms.Panel();
			panel2 = new System.Windows.Forms.Panel();
			label1 = new System.Windows.Forms.Label();
			panel3 = new System.Windows.Forms.Panel();
			splitContainer2 = new System.Windows.Forms.SplitContainer();
			splitContainer1 = new System.Windows.Forms.SplitContainer();
			panel1.SuspendLayout();
			panel2.SuspendLayout();
			panel3.SuspendLayout();
			splitContainer2.Panel1.SuspendLayout();
			splitContainer2.Panel2.SuspendLayout();
			splitContainer2.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.pictureBoxPreview)).BeginInit();
			splitContainer1.Panel1.SuspendLayout();
			splitContainer1.Panel2.SuspendLayout();
			splitContainer1.SuspendLayout();
			this.SuspendLayout();
			// 
			// panel1
			// 
			panel1.Controls.Add(this.listView1);
			panel1.Controls.Add(panel2);
			panel1.Controls.Add(panel3);
			panel1.Dock = System.Windows.Forms.DockStyle.Fill;
			panel1.Location = new System.Drawing.Point(0, 0);
			panel1.Name = "panel1";
			panel1.Size = new System.Drawing.Size(480, 564);
			panel1.TabIndex = 1;
			// 
			// listView1
			// 
			this.listView1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.listView1.Location = new System.Drawing.Point(0, 26);
			this.listView1.Name = "listView1";
			this.listView1.Size = new System.Drawing.Size(480, 509);
			this.listView1.TabIndex = 0;
			this.listView1.UseCompatibleStateImageBehavior = false;
			this.listView1.View = System.Windows.Forms.View.Tile;
			// 
			// panel2
			// 
			panel2.AutoSize = true;
			panel2.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			panel2.Controls.Add(this.textBoxFolderPath);
			panel2.Controls.Add(label1);
			panel2.Controls.Add(this.buttonUpFolder);
			panel2.Dock = System.Windows.Forms.DockStyle.Top;
			panel2.Location = new System.Drawing.Point(0, 0);
			panel2.MinimumSize = new System.Drawing.Size(2, 26);
			panel2.Name = "panel2";
			panel2.Padding = new System.Windows.Forms.Padding(2);
			panel2.Size = new System.Drawing.Size(480, 26);
			panel2.TabIndex = 2;
			// 
			// textBoxFolderPath
			// 
			this.textBoxFolderPath.Dock = System.Windows.Forms.DockStyle.Fill;
			this.textBoxFolderPath.Location = new System.Drawing.Point(40, 2);
			this.textBoxFolderPath.Name = "textBoxFolderPath";
			this.textBoxFolderPath.Size = new System.Drawing.Size(409, 22);
			this.textBoxFolderPath.TabIndex = 1;
			// 
			// label1
			// 
			label1.Dock = System.Windows.Forms.DockStyle.Left;
			label1.Location = new System.Drawing.Point(2, 2);
			label1.Name = "label1";
			label1.Size = new System.Drawing.Size(38, 22);
			label1.TabIndex = 2;
			label1.Text = "Folder:";
			label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// buttonUpFolder
			// 
			this.buttonUpFolder.AutoSize = true;
			this.buttonUpFolder.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.buttonUpFolder.Dock = System.Windows.Forms.DockStyle.Right;
			this.buttonUpFolder.Location = new System.Drawing.Point(449, 2);
			this.buttonUpFolder.Name = "buttonUpFolder";
			this.buttonUpFolder.Size = new System.Drawing.Size(29, 22);
			this.buttonUpFolder.TabIndex = 3;
			this.buttonUpFolder.Text = "Up";
			this.buttonUpFolder.UseVisualStyleBackColor = true;
			// 
			// panel3
			// 
			panel3.Controls.Add(this.button1);
			panel3.Dock = System.Windows.Forms.DockStyle.Bottom;
			panel3.Location = new System.Drawing.Point(0, 535);
			panel3.Name = "panel3";
			panel3.Size = new System.Drawing.Size(480, 29);
			panel3.TabIndex = 3;
			// 
			// button1
			// 
			this.button1.Dock = System.Windows.Forms.DockStyle.Right;
			this.button1.Location = new System.Drawing.Point(405, 0);
			this.button1.Name = "button1";
			this.button1.Size = new System.Drawing.Size(75, 29);
			this.button1.TabIndex = 0;
			this.button1.Text = "Select";
			this.button1.UseVisualStyleBackColor = true;
			this.button1.Click += new System.EventHandler(this.button1_Click);
			// 
			// splitContainer2
			// 
			splitContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
			splitContainer2.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
			splitContainer2.Location = new System.Drawing.Point(0, 0);
			splitContainer2.Name = "splitContainer2";
			splitContainer2.Orientation = System.Windows.Forms.Orientation.Horizontal;
			// 
			// splitContainer2.Panel1
			// 
			splitContainer2.Panel1.Controls.Add(this.treeView1);
			// 
			// splitContainer2.Panel2
			// 
			splitContainer2.Panel2.Controls.Add(this.pictureBoxPreview);
			splitContainer2.Size = new System.Drawing.Size(300, 564);
			splitContainer2.SplitterDistance = 300;
			splitContainer2.TabIndex = 2;
			// 
			// pictureBoxPreview
			// 
			this.pictureBoxPreview.Dock = System.Windows.Forms.DockStyle.Fill;
			this.pictureBoxPreview.Location = new System.Drawing.Point(0, 0);
			this.pictureBoxPreview.Name = "pictureBoxPreview";
			this.pictureBoxPreview.Size = new System.Drawing.Size(300, 260);
			this.pictureBoxPreview.TabIndex = 0;
			this.pictureBoxPreview.TabStop = false;
			// 
			// splitContainer1
			// 
			splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
			splitContainer1.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
			splitContainer1.Location = new System.Drawing.Point(0, 0);
			splitContainer1.Name = "splitContainer1";
			// 
			// splitContainer1.Panel1
			// 
			splitContainer1.Panel1.Controls.Add(splitContainer2);
			// 
			// splitContainer1.Panel2
			// 
			splitContainer1.Panel2.Controls.Add(panel1);
			splitContainer1.Size = new System.Drawing.Size(784, 564);
			splitContainer1.SplitterDistance = 300;
			splitContainer1.TabIndex = 0;
			// 
			// treeView1
			// 
			this.treeView1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.treeView1.Location = new System.Drawing.Point(0, 0);
			this.treeView1.Name = "treeView1";
			this.treeView1.SelectedNode = null;
			this.treeView1.Size = new System.Drawing.Size(300, 300);
			this.treeView1.TabIndex = 1;
			this.treeView1.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeView1_AfterSelect);
			// 
			// AssertBrowsingForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(784, 564);
			this.Controls.Add(splitContainer1);
			this.Name = "AssertBrowsingForm";
			this.Text = "Assert Browser";
			this.Load += new System.EventHandler(this.AssertBrowsingForm_Load);
			this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.AssertBrowsingForm_FormClosing);
			panel1.ResumeLayout(false);
			panel1.PerformLayout();
			panel2.ResumeLayout(false);
			panel2.PerformLayout();
			panel3.ResumeLayout(false);
			splitContainer2.Panel1.ResumeLayout(false);
			splitContainer2.Panel2.ResumeLayout(false);
			splitContainer2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.pictureBoxPreview)).EndInit();
			splitContainer1.Panel1.ResumeLayout(false);
			splitContainer1.Panel2.ResumeLayout(false);
			splitContainer1.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.ListView listView1;
		private Studio.AssertTreeViewControl treeView1;
		private System.Windows.Forms.TextBox textBoxFolderPath;
		private System.Windows.Forms.Button buttonUpFolder;
		private System.Windows.Forms.PictureBox pictureBoxPreview;
		private System.Windows.Forms.Button button1;


	}
}
