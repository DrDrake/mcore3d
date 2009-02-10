using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using WeifenLuo.WinFormsUI.Docking;

namespace Studio
{
	public partial class MainForm: Form
	{
		public MainForm()
		{
			InitializeComponent();
		}

		private void MainForm_Load(object sender, EventArgs e)
		{
			dockPanel.SuspendLayout(true);

			DockContent content = new DockContent();
			content.Show(dockPanel, DockState.Document);
			RenderPanel.RenderPanelControl renderPanel1 = new RenderPanel.RenderPanelControl();
			renderPanel1.Dock = DockStyle.Fill;
			content.Controls.Add(renderPanel1);
			content.TabText = "Scene 1";

			content = new DockContent();
			content.Show(dockPanel, DockState.Document);
			RenderPanel.RenderPanelControl renderPanel2 = new RenderPanel.RenderPanelControl();
			renderPanel2.Dock = DockStyle.Fill;
			content.Controls.Add(renderPanel2);
			content.TabText = "Scene 2";

			PropertyWindow propertyWindow = new PropertyWindow();
			propertyWindow.Show(dockPanel);

			EntityWindow entityWindow = new EntityWindow();
			entityWindow.Show(dockPanel);

			TreeView treeView = (TreeView)entityWindow.Tag;
			TreeNode node;
			node = treeView.Nodes.Add("Fruits");
			node.Nodes.Add("Apple");
			node.Nodes.Add("Peach");

			dockPanel.ResumeLayout(true, true);
		}

		private void exitToolStripMenuItem_Click(object sender, EventArgs e)
		{
			Close();
		}
	}
}
