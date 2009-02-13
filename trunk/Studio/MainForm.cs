using RenderPanel;
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
			renderControls = new List<RenderPanelControl>();
		}

	// Operations

	// Events
		private void MainForm_Load(object sender, EventArgs e)
		{
			dockPanel.SuspendLayout(true);

			propertyWindow = new PropertyWindow();
			propertyWindow.Show(dockPanel);

			entityWindow = new EntityWindow();
			entityWindow.Show(dockPanel);
			entityWindow.propertyWindow = propertyWindow;

			dockPanel.ResumeLayout(true, true);
		}

		private void newToolStripMenuItem_Click(object sender, EventArgs e)
		{
			DockContent content = new DockContent();
			content.Show(dockPanel, DockState.Document);
			RenderPanelControl renderPanel = new RenderPanelControl();
			content.Tag = renderPanel;
			renderControls.Add(renderPanel);
			renderPanel.Enter += new EventHandler(sceneActivated);
			content.FormClosing += new FormClosingEventHandler(sceneClosing);
			renderPanel.Dock = DockStyle.Fill;
			content.Controls.Add(renderPanel);
			content.TabText = "Scene " + renderControls.Count;

			sceneActivated(renderPanel, new EventArgs());
		}

		/// <summary>
		/// Invoked when a particular rendering panel is selected
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void sceneActivated(object sender, EventArgs e)
		{
			RenderPanelControl backup = currentRenderControl;
			currentRenderControl = (RenderPanelControl)sender;

			// Disable all but only the selected control will continue auto updating.
			// This way, we are REALLY sure that only one scene is keep rendering.
			foreach (RenderPanelControl r in renderControls)
				r.enableAutoUpdate(false);
			currentRenderControl.enableAutoUpdate(true);

			// Refresh the Entity explorer and the property window as well,
			// if the current render panel is switched to another
			if (currentRenderControl != backup)
			{
				entityWindow.selectEntityRoot(currentRenderControl.rootEntity);
				entityWindow.treeView.SelectedNodes.Clear();
			}
		}

		void sceneClosing(object sender, FormClosingEventArgs e)
		{
/*			if (DialogResult.Yes != MessageBox.Show("Recent changes have not been saved. Close the scene anyway?",
				"Close scene?", MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button2))
			{
				e.Cancel = true;
			}
			else*/
			{
				DockContent d = (DockContent)sender;
				RenderPanelControl c = (RenderPanelControl)d.Tag;
				c.destroy();
				currentRenderControl = null;
				renderControls.Remove(c);
				entityWindow.selectEntityRoot(null);
			}
		}

		private void exitToolStripMenuItem_Click(object sender, EventArgs e)
		{
			Close();
		}

	// Attrubutes
		/// <summary>
		/// The current selected rendering panel.
		/// </summary>
		RenderPanelControl currentRenderControl;

		/// <summary>
		/// A list of all rendering panels.
		/// </summary>
		List<RenderPanelControl> renderControls;

		EntityWindow entityWindow;
		PropertyWindow propertyWindow;
	}
}
