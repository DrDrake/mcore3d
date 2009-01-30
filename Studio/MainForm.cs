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
			content.Controls.Add(new RenderPanel.RenderPanelControl());
			content.TabText = "Scene 1";

			content = new DockContent();
			content.Show(dockPanel, DockState.Document);
			content.Controls.Add(new RenderPanel.RenderPanelControl());
			content.TabText = "Scene 2";

			PropertyWindow propertyWindow = new PropertyWindow();
			propertyWindow.Show(dockPanel);

			dockPanel.ResumeLayout(true, true);
		}
	}
}
