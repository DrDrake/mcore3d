namespace Studio
{
	partial class CpuProfilerWindow
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
			this.treeColumn1 = new Aga.Controls.Tree.TreeColumn();
			this.treeColumn2 = new Aga.Controls.Tree.TreeColumn();
			this.treeColumn3 = new Aga.Controls.Tree.TreeColumn();
			this.treeColumn4 = new Aga.Controls.Tree.TreeColumn();
			this.treeColumn5 = new Aga.Controls.Tree.TreeColumn();
			this.nodeTextBox1 = new Aga.Controls.Tree.NodeControls.NodeTextBox();
			this.nodeDecimalTextBox1 = new Aga.Controls.Tree.NodeControls.NodeDecimalTextBox();
			this.nodeDecimalTextBox2 = new Aga.Controls.Tree.NodeControls.NodeDecimalTextBox();
			this.nodeTextBox2 = new Aga.Controls.Tree.NodeControls.NodeTextBox();
			this.nodeTextBox3 = new Aga.Controls.Tree.NodeControls.NodeTextBox();
			this.flowLayoutPanel1 = new System.Windows.Forms.FlowLayoutPanel();
			this.button1 = new System.Windows.Forms.Button();
			this.button2 = new System.Windows.Forms.Button();
			this.panel1 = new System.Windows.Forms.Panel();
			this.textBox1 = new System.Windows.Forms.TextBox();
			this.timer1 = new System.Windows.Forms.Timer(this.components);
			this.backgroundWorker1 = new System.ComponentModel.BackgroundWorker();
			this.treeViewAdv1 = new Aga.Controls.Tree.TreeViewAdv();
			this.flowLayoutPanel1.SuspendLayout();
			this.panel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// treeColumn1
			// 
			this.treeColumn1.Header = "Callstack";
			this.treeColumn1.SortOrder = System.Windows.Forms.SortOrder.None;
			this.treeColumn1.TooltipText = "Function name";
			this.treeColumn1.Width = 300;
			// 
			// treeColumn2
			// 
			this.treeColumn2.Header = "TT/F %";
			this.treeColumn2.SortOrder = System.Windows.Forms.SortOrder.None;
			this.treeColumn2.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			this.treeColumn2.TooltipText = "Total time per frame (including child call) in percentage";
			this.treeColumn2.Width = 60;
			// 
			// treeColumn3
			// 
			this.treeColumn3.Header = "ST/F %";
			this.treeColumn3.SortOrder = System.Windows.Forms.SortOrder.None;
			this.treeColumn3.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			this.treeColumn3.TooltipText = "Self time per frame (child call not included) in percentage";
			this.treeColumn3.Width = 60;
			// 
			// treeColumn4
			// 
			this.treeColumn4.Header = "TT/C";
			this.treeColumn4.SortOrder = System.Windows.Forms.SortOrder.None;
			this.treeColumn4.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			this.treeColumn4.TooltipText = "Total time per call (including child call) in second";
			this.treeColumn4.Width = 60;
			// 
			// treeColumn5
			// 
			this.treeColumn5.Header = "C/F";
			this.treeColumn5.SortOrder = System.Windows.Forms.SortOrder.None;
			this.treeColumn5.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			this.treeColumn5.TooltipText = "Call per frame";
			this.treeColumn5.Width = 60;
			// 
			// nodeTextBox1
			// 
			this.nodeTextBox1.DataPropertyName = "Name";
			this.nodeTextBox1.IncrementalSearchEnabled = true;
			this.nodeTextBox1.LeftMargin = 3;
			this.nodeTextBox1.ParentColumn = this.treeColumn1;
			// 
			// nodeDecimalTextBox1
			// 
			this.nodeDecimalTextBox1.DataPropertyName = "TTime";
			this.nodeDecimalTextBox1.IncrementalSearchEnabled = true;
			this.nodeDecimalTextBox1.LeftMargin = 3;
			this.nodeDecimalTextBox1.ParentColumn = this.treeColumn2;
			this.nodeDecimalTextBox1.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			// 
			// nodeDecimalTextBox2
			// 
			this.nodeDecimalTextBox2.DataPropertyName = "STime";
			this.nodeDecimalTextBox2.IncrementalSearchEnabled = true;
			this.nodeDecimalTextBox2.LeftMargin = 3;
			this.nodeDecimalTextBox2.ParentColumn = this.treeColumn3;
			this.nodeDecimalTextBox2.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			// 
			// nodeTextBox2
			// 
			this.nodeTextBox2.DataPropertyName = "TTimePerCall";
			this.nodeTextBox2.IncrementalSearchEnabled = true;
			this.nodeTextBox2.LeftMargin = 3;
			this.nodeTextBox2.ParentColumn = this.treeColumn4;
			this.nodeTextBox2.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			// 
			// nodeTextBox3
			// 
			this.nodeTextBox3.DataPropertyName = "CallPerFrame";
			this.nodeTextBox3.IncrementalSearchEnabled = true;
			this.nodeTextBox3.LeftMargin = 3;
			this.nodeTextBox3.ParentColumn = this.treeColumn5;
			this.nodeTextBox3.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			// 
			// flowLayoutPanel1
			// 
			this.flowLayoutPanel1.AutoSize = true;
			this.flowLayoutPanel1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.flowLayoutPanel1.Controls.Add(this.button1);
			this.flowLayoutPanel1.Controls.Add(this.button2);
			this.flowLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Right;
			this.flowLayoutPanel1.Location = new System.Drawing.Point(568, 0);
			this.flowLayoutPanel1.Name = "flowLayoutPanel1";
			this.flowLayoutPanel1.Size = new System.Drawing.Size(101, 23);
			this.flowLayoutPanel1.TabIndex = 2;
			this.flowLayoutPanel1.WrapContents = false;
			// 
			// button1
			// 
			this.button1.AutoSize = true;
			this.button1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.button1.Location = new System.Drawing.Point(3, 0);
			this.button1.Margin = new System.Windows.Forms.Padding(3, 0, 0, 0);
			this.button1.Name = "button1";
			this.button1.Size = new System.Drawing.Size(54, 22);
			this.button1.TabIndex = 0;
			this.button1.Text = "Connect";
			this.button1.UseVisualStyleBackColor = true;
			this.button1.Click += new System.EventHandler(this.button1_Click);
			// 
			// button2
			// 
			this.button2.AutoSize = true;
			this.button2.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.button2.Enabled = false;
			this.button2.Location = new System.Drawing.Point(60, 0);
			this.button2.Margin = new System.Windows.Forms.Padding(3, 0, 0, 0);
			this.button2.Name = "button2";
			this.button2.Size = new System.Drawing.Size(41, 22);
			this.button2.TabIndex = 1;
			this.button2.Text = "Pause";
			this.button2.UseVisualStyleBackColor = true;
			this.button2.Click += new System.EventHandler(this.button2_Click);
			// 
			// panel1
			// 
			this.panel1.Controls.Add(this.textBox1);
			this.panel1.Controls.Add(this.flowLayoutPanel1);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel1.Location = new System.Drawing.Point(0, 0);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(669, 23);
			this.panel1.TabIndex = 3;
			// 
			// textBox1
			// 
			this.textBox1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.textBox1.Location = new System.Drawing.Point(0, 0);
			this.textBox1.Name = "textBox1";
			this.textBox1.Size = new System.Drawing.Size(568, 22);
			this.textBox1.TabIndex = 1;
			this.textBox1.Text = "localhost:5001";
			this.textBox1.WordWrap = false;
			// 
			// timer1
			// 
			this.timer1.Interval = 500;
			this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
			// 
			// backgroundWorker1
			// 
			this.backgroundWorker1.WorkerSupportsCancellation = true;
			this.backgroundWorker1.DoWork += new System.ComponentModel.DoWorkEventHandler(this.backgroundWorker1_DoWork);
			this.backgroundWorker1.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.backgroundWorker1_RunWorkerCompleted);
			// 
			// treeViewAdv1
			// 
			this.treeViewAdv1.AllowColumnReorder = true;
			this.treeViewAdv1.BackColor = System.Drawing.SystemColors.Window;
			this.treeViewAdv1.Columns.Add(this.treeColumn1);
			this.treeViewAdv1.Columns.Add(this.treeColumn2);
			this.treeViewAdv1.Columns.Add(this.treeColumn3);
			this.treeViewAdv1.Columns.Add(this.treeColumn4);
			this.treeViewAdv1.Columns.Add(this.treeColumn5);
			this.treeViewAdv1.DefaultToolTipProvider = null;
			this.treeViewAdv1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.treeViewAdv1.DragDropMarkColor = System.Drawing.Color.Black;
			this.treeViewAdv1.FullRowSelect = true;
			this.treeViewAdv1.GridLineStyle = ((Aga.Controls.Tree.GridLineStyle)((Aga.Controls.Tree.GridLineStyle.Horizontal | Aga.Controls.Tree.GridLineStyle.Vertical)));
			this.treeViewAdv1.Indent = 12;
			this.treeViewAdv1.LineColor = System.Drawing.SystemColors.ControlDark;
			this.treeViewAdv1.Location = new System.Drawing.Point(0, 23);
			this.treeViewAdv1.Model = null;
			this.treeViewAdv1.Name = "treeViewAdv1";
			this.treeViewAdv1.NodeControls.Add(this.nodeTextBox1);
			this.treeViewAdv1.NodeControls.Add(this.nodeDecimalTextBox1);
			this.treeViewAdv1.NodeControls.Add(this.nodeDecimalTextBox2);
			this.treeViewAdv1.NodeControls.Add(this.nodeTextBox2);
			this.treeViewAdv1.NodeControls.Add(this.nodeTextBox3);
			this.treeViewAdv1.SelectedNode = null;
			this.treeViewAdv1.Size = new System.Drawing.Size(669, 318);
			this.treeViewAdv1.TabIndex = 2;
			this.treeViewAdv1.Text = "treeViewAdv";
			this.treeViewAdv1.UseColumns = true;
			// 
			// CpuProfilerWindow
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(669, 341);
			this.Controls.Add(this.treeViewAdv1);
			this.Controls.Add(this.panel1);
			this.DockAreas = ((WeifenLuo.WinFormsUI.Docking.DockAreas)((((((WeifenLuo.WinFormsUI.Docking.DockAreas.Float | WeifenLuo.WinFormsUI.Docking.DockAreas.DockLeft)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.DockRight)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.DockTop)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.DockBottom)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.Document)));
			this.Name = "CpuProfilerWindow";
			this.ShowHint = WeifenLuo.WinFormsUI.Docking.DockState.DockBottom;
			this.TabText = "Cpu profiler";
			this.Text = "Cpu profiler";
			this.flowLayoutPanel1.ResumeLayout(false);
			this.flowLayoutPanel1.PerformLayout();
			this.panel1.ResumeLayout(false);
			this.panel1.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Button button1;
		private System.Windows.Forms.Timer timer1;
		private System.ComponentModel.BackgroundWorker backgroundWorker1;
		private System.Windows.Forms.TextBox textBox1;
		private Aga.Controls.Tree.TreeViewAdv treeViewAdv1;
		private System.Windows.Forms.Button button2;
		private Aga.Controls.Tree.TreeColumn treeColumn1;
		private Aga.Controls.Tree.TreeColumn treeColumn2;
		private Aga.Controls.Tree.TreeColumn treeColumn3;
		private Aga.Controls.Tree.TreeColumn treeColumn4;
		private Aga.Controls.Tree.TreeColumn treeColumn5;
		private Aga.Controls.Tree.NodeControls.NodeTextBox nodeTextBox1;
		private Aga.Controls.Tree.NodeControls.NodeDecimalTextBox nodeDecimalTextBox1;
		private Aga.Controls.Tree.NodeControls.NodeDecimalTextBox nodeDecimalTextBox2;
		private Aga.Controls.Tree.NodeControls.NodeTextBox nodeTextBox2;
		private Aga.Controls.Tree.NodeControls.NodeTextBox nodeTextBox3;
		private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel1;
		private System.Windows.Forms.Panel panel1;
	}
}
