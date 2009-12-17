namespace Studio
{
	partial class EntityWindow
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
			Aga.Controls.Tree.TreeColumn treeColumn1;
			Aga.Controls.Tree.NodeControls.NodeTextBox nodeTextBox1;
			this.treeView = new Aga.Controls.Tree.TreeViewAdv();
			treeColumn1 = new Aga.Controls.Tree.TreeColumn();
			nodeTextBox1 = new Aga.Controls.Tree.NodeControls.NodeTextBox();
			this.SuspendLayout();
			// 
			// treeColumn1
			// 
			treeColumn1.Header = "";
			treeColumn1.SortOrder = System.Windows.Forms.SortOrder.None;
			treeColumn1.TooltipText = null;
			// 
			// treeView
			// 
			this.treeView.BackColor = System.Drawing.SystemColors.Window;
			this.treeView.Columns.Add(treeColumn1);
			this.treeView.DefaultToolTipProvider = null;
			this.treeView.Dock = System.Windows.Forms.DockStyle.Fill;
			this.treeView.DragDropMarkColor = System.Drawing.Color.Black;
			this.treeView.LineColor = System.Drawing.SystemColors.ControlDark;
			this.treeView.Location = new System.Drawing.Point(0, 0);
			this.treeView.Model = null;
			this.treeView.Name = "treeView";
			this.treeView.NodeControls.Add(nodeTextBox1);
			this.treeView.SelectedNode = null;
			this.treeView.Size = new System.Drawing.Size(292, 250);
			this.treeView.TabIndex = 1;
			this.treeView.Text = "treeViewAdv1";
			this.treeView.SelectionChanged += new System.EventHandler(this.treeView_SelectionChanged);
			this.treeView.KeyUp += new System.Windows.Forms.KeyEventHandler(this.treeView_KeyUp);
			// 
			// nodeTextBox1
			// 
			nodeTextBox1.DataPropertyName = "name";
			nodeTextBox1.IncrementalSearchEnabled = true;
			nodeTextBox1.LeftMargin = 3;
			nodeTextBox1.ParentColumn = treeColumn1;
			// 
			// EntityWindow
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(292, 250);
			this.Controls.Add(this.treeView);
			this.DockAreas = ((WeifenLuo.WinFormsUI.Docking.DockAreas)((((((WeifenLuo.WinFormsUI.Docking.DockAreas.Float | WeifenLuo.WinFormsUI.Docking.DockAreas.DockLeft)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.DockRight)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.DockTop)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.DockBottom)
						| WeifenLuo.WinFormsUI.Docking.DockAreas.Document)));
			this.Name = "EntityWindow";
			this.ShowHint = WeifenLuo.WinFormsUI.Docking.DockState.DockLeft;
			this.TabText = "Entity Explorer";
			this.Text = "Entity Explorer";
			this.ResumeLayout(false);

		}

		#endregion

		private Aga.Controls.Tree.TreeViewAdv treeView;


	}
}