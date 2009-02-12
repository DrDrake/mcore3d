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

			DockContent content = new DockContent();
			content.Show(dockPanel, DockState.Document);
			RenderPanelControl renderPanel1 = new RenderPanelControl();
			renderControls.Add(renderPanel1);
			renderPanel1.Enter += new EventHandler(renderControlActivated);
			renderPanel1.Dock = DockStyle.Fill;
			content.Controls.Add(renderPanel1);
			content.TabText = "Scene 1";

			content = new DockContent();
			content.Show(dockPanel, DockState.Document);
			RenderPanelControl renderPanel2 = new RenderPanelControl();
			renderControls.Add(renderPanel2);
			renderPanel2.Enter += new EventHandler(renderControlActivated);
			renderPanel2.Dock = DockStyle.Fill;
			content.Controls.Add(renderPanel2);
			content.TabText = "Scene 2";

			propertyWindow.propertyGrid1.SelectedObject = renderPanel1.rootEntity;

			entityWindow.selectEntityRoot(renderPanel1.rootEntity);
			dockPanel.ResumeLayout(true, true);
		}

		/// <summary>
		/// Invoked when a particular rendering panel is selected
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void renderControlActivated(object sender, EventArgs e)
		{
			RenderPanelControl backup = currentRenderControl;
			// Disable all but only the selected control will continue auto updating.
			foreach (RenderPanelControl r in renderControls)
				r.enableAutoUpdate(false);
			currentRenderControl = (RenderPanelControl)sender;
			currentRenderControl.enableAutoUpdate(true);

			// Refresh the Entity explorer and the property window as well,
			// if the current render panel is switched to another
			if (currentRenderControl != backup)
			{
				entityWindow.selectEntityRoot(currentRenderControl.rootEntity);
				entityWindow.treeView.SelectedNodes.Clear();
				propertyWindow.propertyGrid1.SelectedObject = entityWindow.selectedEntity;
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
