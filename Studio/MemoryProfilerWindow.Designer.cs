﻿namespace Studio
{
	partial class MemoryProfilerWindow
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
			Aga.Controls.Tree.TreeColumn treeColumn1;
			Aga.Controls.Tree.TreeColumn treeColumn2;
			Aga.Controls.Tree.TreeColumn treeColumn3;
			Aga.Controls.Tree.TreeColumn treeColumn4;
			Aga.Controls.Tree.TreeColumn treeColumn5;
			Aga.Controls.Tree.TreeColumn treeColumn6;
			Aga.Controls.Tree.TreeColumn treeColumn7;
			Aga.Controls.Tree.NodeControls.NodeTextBox nodeTextBox1;
			Aga.Controls.Tree.NodeControls.NodeDecimalTextBox nodeDecimalTextBox1;
			Aga.Controls.Tree.NodeControls.NodeDecimalTextBox nodeDecimalTextBox2;
			Aga.Controls.Tree.NodeControls.NodeDecimalTextBox nodeDecimalTextBox3;
			Aga.Controls.Tree.NodeControls.NodeDecimalTextBox nodeDecimalTextBox4;
			Aga.Controls.Tree.NodeControls.NodeDecimalTextBox nodeDecimalTextBox5;
			Aga.Controls.Tree.NodeControls.NodeDecimalTextBox nodeDecimalTextBox6;
			this.button1 = new System.Windows.Forms.Button();
			this.timer1 = new System.Windows.Forms.Timer(this.components);
			this.backgroundWorker1 = new System.ComponentModel.BackgroundWorker();
			this.textBox1 = new System.Windows.Forms.TextBox();
			this.treeViewAdv1 = new Aga.Controls.Tree.TreeViewAdv();
			this.panel1 = new System.Windows.Forms.Panel();
			treeColumn1 = new Aga.Controls.Tree.TreeColumn();
			treeColumn2 = new Aga.Controls.Tree.TreeColumn();
			treeColumn3 = new Aga.Controls.Tree.TreeColumn();
			treeColumn4 = new Aga.Controls.Tree.TreeColumn();
			treeColumn5 = new Aga.Controls.Tree.TreeColumn();
			treeColumn6 = new Aga.Controls.Tree.TreeColumn();
			treeColumn7 = new Aga.Controls.Tree.TreeColumn();
			nodeTextBox1 = new Aga.Controls.Tree.NodeControls.NodeTextBox();
			nodeDecimalTextBox1 = new Aga.Controls.Tree.NodeControls.NodeDecimalTextBox();
			nodeDecimalTextBox2 = new Aga.Controls.Tree.NodeControls.NodeDecimalTextBox();
			nodeDecimalTextBox3 = new Aga.Controls.Tree.NodeControls.NodeDecimalTextBox();
			nodeDecimalTextBox4 = new Aga.Controls.Tree.NodeControls.NodeDecimalTextBox();
			nodeDecimalTextBox5 = new Aga.Controls.Tree.NodeControls.NodeDecimalTextBox();
			nodeDecimalTextBox6 = new Aga.Controls.Tree.NodeControls.NodeDecimalTextBox();
			this.panel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// treeColumn1
			// 
			treeColumn1.Header = "Callstack";
			treeColumn1.SortOrder = System.Windows.Forms.SortOrder.None;
			treeColumn1.TooltipText = "Function name";
			treeColumn1.Width = 300;
			// 
			// treeColumn2
			// 
			treeColumn2.Header = "TCount";
			treeColumn2.SortOrder = System.Windows.Forms.SortOrder.None;
			treeColumn2.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			treeColumn2.TooltipText = "Total no. of allocation count (including child call)";
			treeColumn2.Width = 60;
			// 
			// treeColumn3
			// 
			treeColumn3.Header = "SCount";
			treeColumn3.SortOrder = System.Windows.Forms.SortOrder.None;
			treeColumn3.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			treeColumn3.TooltipText = "Self no. of allocation count (child call not included)";
			treeColumn3.Width = 60;
			// 
			// treeColumn4
			// 
			treeColumn4.Header = "TkBytes";
			treeColumn4.SortOrder = System.Windows.Forms.SortOrder.None;
			treeColumn4.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			treeColumn4.TooltipText = "Total allocated memory in kilo bytes (including child call)";
			treeColumn4.Width = 60;
			// 
			// treeColumn5
			// 
			treeColumn5.Header = "SkBytes";
			treeColumn5.SortOrder = System.Windows.Forms.SortOrder.None;
			treeColumn5.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			treeColumn5.TooltipText = "Self allocated memory in kilo bytes (child call not included)";
			treeColumn5.Width = 60;
			// 
			// treeColumn6
			// 
			treeColumn6.Header = "SCount/F";
			treeColumn6.SortOrder = System.Windows.Forms.SortOrder.None;
			treeColumn6.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			treeColumn6.TooltipText = "Self no. of allocation count per frame";
			treeColumn6.Width = 60;
			// 
			// treeColumn7
			// 
			treeColumn7.Header = "Call/F";
			treeColumn7.SortOrder = System.Windows.Forms.SortOrder.None;
			treeColumn7.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			treeColumn7.TooltipText = "No. of call per frame";
			treeColumn7.Width = 60;
			// 
			// nodeTextBox1
			// 
			nodeTextBox1.DataPropertyName = "Name";
			nodeTextBox1.EditEnabled = false;
			nodeTextBox1.IncrementalSearchEnabled = true;
			nodeTextBox1.LeftMargin = 3;
			nodeTextBox1.ParentColumn = treeColumn1;
			// 
			// nodeDecimalTextBox1
			// 
			nodeDecimalTextBox1.DataPropertyName = "TCount";
			nodeDecimalTextBox1.EditEnabled = false;
			nodeDecimalTextBox1.IncrementalSearchEnabled = true;
			nodeDecimalTextBox1.LeftMargin = 3;
			nodeDecimalTextBox1.ParentColumn = treeColumn2;
			nodeDecimalTextBox1.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			// 
			// nodeDecimalTextBox2
			// 
			nodeDecimalTextBox2.DataPropertyName = "SCount";
			nodeDecimalTextBox2.EditEnabled = false;
			nodeDecimalTextBox2.IncrementalSearchEnabled = true;
			nodeDecimalTextBox2.LeftMargin = 3;
			nodeDecimalTextBox2.ParentColumn = treeColumn3;
			nodeDecimalTextBox2.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			// 
			// nodeDecimalTextBox3
			// 
			nodeDecimalTextBox3.DataPropertyName = "TkBytes";
			nodeDecimalTextBox3.EditEnabled = false;
			nodeDecimalTextBox3.IncrementalSearchEnabled = true;
			nodeDecimalTextBox3.LeftMargin = 3;
			nodeDecimalTextBox3.ParentColumn = treeColumn4;
			nodeDecimalTextBox3.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			// 
			// nodeDecimalTextBox4
			// 
			nodeDecimalTextBox4.DataPropertyName = "SkBytes";
			nodeDecimalTextBox4.EditEnabled = false;
			nodeDecimalTextBox4.IncrementalSearchEnabled = true;
			nodeDecimalTextBox4.LeftMargin = 3;
			nodeDecimalTextBox4.ParentColumn = treeColumn5;
			nodeDecimalTextBox4.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			// 
			// nodeDecimalTextBox5
			// 
			nodeDecimalTextBox5.DataPropertyName = "SCountPerFrame";
			nodeDecimalTextBox5.IncrementalSearchEnabled = true;
			nodeDecimalTextBox5.LeftMargin = 3;
			nodeDecimalTextBox5.ParentColumn = treeColumn6;
			nodeDecimalTextBox5.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			// 
			// nodeDecimalTextBox6
			// 
			nodeDecimalTextBox6.DataPropertyName = "CallPerFrame";
			nodeDecimalTextBox6.IncrementalSearchEnabled = true;
			nodeDecimalTextBox6.LeftMargin = 3;
			nodeDecimalTextBox6.ParentColumn = treeColumn7;
			nodeDecimalTextBox6.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			// 
			// button1
			// 
			this.button1.AutoSize = true;
			this.button1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.button1.Dock = System.Windows.Forms.DockStyle.Right;
			this.button1.Location = new System.Drawing.Point(612, 0);
			this.button1.Name = "button1";
			this.button1.Size = new System.Drawing.Size(57, 25);
			this.button1.TabIndex = 0;
			this.button1.Text = "Connect";
			this.button1.UseVisualStyleBackColor = true;
			this.button1.Click += new System.EventHandler(this.button1_Click);
			// 
			// timer1
			// 
			this.timer1.Enabled = true;
			this.timer1.Interval = 500;
			this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
			// 
			// backgroundWorker1
			// 
			this.backgroundWorker1.WorkerSupportsCancellation = true;
			this.backgroundWorker1.DoWork += new System.ComponentModel.DoWorkEventHandler(this.backgroundWorker1_DoWork);
			this.backgroundWorker1.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.backgroundWorker1_RunWorkerCompleted);
			// 
			// textBox1
			// 
			this.textBox1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.textBox1.Location = new System.Drawing.Point(0, 0);
			this.textBox1.Name = "textBox1";
			this.textBox1.Size = new System.Drawing.Size(612, 20);
			this.textBox1.TabIndex = 1;
			this.textBox1.Text = "localhost:5000";
			this.textBox1.WordWrap = false;
			// 
			// treeViewAdv1
			// 
			this.treeViewAdv1.AllowColumnReorder = true;
			this.treeViewAdv1.BackColor = System.Drawing.SystemColors.Window;
			this.treeViewAdv1.Columns.Add(treeColumn1);
			this.treeViewAdv1.Columns.Add(treeColumn2);
			this.treeViewAdv1.Columns.Add(treeColumn3);
			this.treeViewAdv1.Columns.Add(treeColumn4);
			this.treeViewAdv1.Columns.Add(treeColumn5);
			this.treeViewAdv1.Columns.Add(treeColumn6);
			this.treeViewAdv1.Columns.Add(treeColumn7);
			this.treeViewAdv1.DefaultToolTipProvider = null;
			this.treeViewAdv1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.treeViewAdv1.DragDropMarkColor = System.Drawing.Color.Black;
			this.treeViewAdv1.FullRowSelect = true;
			this.treeViewAdv1.GridLineStyle = ((Aga.Controls.Tree.GridLineStyle)((Aga.Controls.Tree.GridLineStyle.Horizontal | Aga.Controls.Tree.GridLineStyle.Vertical)));
			this.treeViewAdv1.Indent = 12;
			this.treeViewAdv1.LineColor = System.Drawing.SystemColors.ControlDark;
			this.treeViewAdv1.Location = new System.Drawing.Point(0, 25);
			this.treeViewAdv1.Model = null;
			this.treeViewAdv1.Name = "treeViewAdv1";
			this.treeViewAdv1.NodeControls.Add(nodeTextBox1);
			this.treeViewAdv1.NodeControls.Add(nodeDecimalTextBox1);
			this.treeViewAdv1.NodeControls.Add(nodeDecimalTextBox2);
			this.treeViewAdv1.NodeControls.Add(nodeDecimalTextBox3);
			this.treeViewAdv1.NodeControls.Add(nodeDecimalTextBox4);
			this.treeViewAdv1.NodeControls.Add(nodeDecimalTextBox5);
			this.treeViewAdv1.NodeControls.Add(nodeDecimalTextBox6);
			this.treeViewAdv1.SelectedNode = null;
			this.treeViewAdv1.Size = new System.Drawing.Size(669, 344);
			this.treeViewAdv1.TabIndex = 2;
			this.treeViewAdv1.Text = "treeViewAdv1";
			this.treeViewAdv1.UseColumns = true;
			// 
			// panel1
			// 
			this.panel1.Controls.Add(this.textBox1);
			this.panel1.Controls.Add(this.button1);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel1.Location = new System.Drawing.Point(0, 0);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(669, 25);
			this.panel1.TabIndex = 3;
			// 
			// MemoryProfilerWindow
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(669, 369);
			this.Controls.Add(this.treeViewAdv1);
			this.Controls.Add(this.panel1);
			this.DockAreas = ((WeifenLuo.WinFormsUI.Docking.DockAreas)((((((WeifenLuo.WinFormsUI.Docking.DockAreas.Float | WeifenLuo.WinFormsUI.Docking.DockAreas.DockLeft)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.DockRight)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.DockTop)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.DockBottom)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.Document)));
			this.Name = "MemoryProfilerWindow";
			this.ShowHint = WeifenLuo.WinFormsUI.Docking.DockState.DockBottom;
			this.TabText = "Memory profiler";
			this.Text = "Memory profiler";
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
		private System.Windows.Forms.Panel panel1;
	}
}
