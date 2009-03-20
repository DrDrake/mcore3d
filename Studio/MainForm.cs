using System;
using System.Collections.Generic;
using System.Windows.Forms;
using Binding;
using WeifenLuo.WinFormsUI.Docking;

namespace Studio
{
	public partial class MainForm: Form
	{
		public MainForm()
		{
			InitializeComponent();
			renderControls = new List<RenderPanelControl>();
			mDeserializeDockContent = new DeserializeDockContent(getDockingFromPersistString);
		}

	// Operations

	// Events
		private void MainForm_Load(object sender, EventArgs e)
		{
			assertWindow = new AssertWindow();
			entityWindow = new EntityWindow();
			logWindow = new LogWindow();
			propertyWindow = new PropertyWindow();

			// The Global object can be accessed though Global::instance
			new Global(logWindow.textBox);

			assertWindow.refresh("media");
			entityWindow.entitySelectionChanged += new EntitySelectionChangedHandler(onEntitySelectionChanged);

			// Restore the docking layout
			dockPanel.SuspendLayout(true);
			try
			{
				dockPanel.LoadFromXml("layout.xml", mDeserializeDockContent);
			}
			catch (Exception)
			{	// Use a default docking if the xml failed to load
				assertWindow.Show(dockPanel);
				entityWindow.Show(dockPanel);
				logWindow.Show(dockPanel);
				propertyWindow.Show(dockPanel);
			}

			dockPanel.ResumeLayout(true, true);
		}

		private void newToolStripMenuItem_Click(object sender, EventArgs e)
		{
			DockContent content = new DockContent();
			content.Show(dockPanel, DockState.Document);
			RenderPanelControl renderPanel = new RenderPanelControl();
			renderPanel.entitySelectionChanged += new EntitySelectionChangedHandler(onEntitySelectionChanged);
			content.Tag = renderPanel;
			renderControls.Add(renderPanel);
			renderPanel.Enter += new EventHandler(sceneSelectionChanged);
			content.FormClosing += new FormClosingEventHandler(sceneClosing);
			renderPanel.Dock = DockStyle.Fill;
			content.Controls.Add(renderPanel);
			content.TabText = "Scene " + renderControls.Count;

			// Selected the newly created scene
			sceneSelectionChanged(renderPanel, new EventArgs());
		}

		/// <summary>
		/// Invoked when a particular rendering panel is selected, but only do meaningfull
		/// things if the scene selection is changed.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void sceneSelectionChanged(object sender, EventArgs e)
		{
			RenderPanelControl backup = currentRenderControl;
			currentRenderControl = (RenderPanelControl)sender;

			if (currentRenderControl == backup)
				return;

			// Disable all but only the selected control will continue auto updating.
			// This way, we are REALLY sure that only one scene is keep rendering.
			foreach (RenderPanelControl r in renderControls)
				r.enableAutoUpdate(false);

			onEntitySelectionChanged(this, null);

			if(currentRenderControl != null)
			{
				currentRenderControl.enableAutoUpdate(true);
				entityWindow.selectEntityRoot(currentRenderControl.rootEntity);
			}
			else
				entityWindow.selectEntityRoot(null);

			entityWindow.treeView.SelectedNodes.Clear();
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
				// Deselect the currently selected scene
				sceneSelectionChanged(null, new EventArgs());

				DockContent d = (DockContent)sender;
				RenderPanelControl c = (RenderPanelControl)d.Tag;
				c.destroy();
				renderControls.Remove(c);
			}
		}

		private void exitToolStripMenuItem_Click(object sender, EventArgs e)
		{
			Close();
		}

		private void onEntitySelectionChanged(object sender, Entity entity)
		{
			// Use sender == null as an indicator to break the infinity recursion
			if (sender == null)
				return;

			currentSelectedEntity = entity;
			propertyWindow.propertyGrid1.SelectedObject = entity;

			// Broadcast the event to other windows
			entityWindow.entitySelectionChanged(null, entity);
			if(currentRenderControl != null)
				currentRenderControl.entitySelectionChanged(null, entity);
		}

		private void MainForm_FormClosed(object sender, FormClosedEventArgs e)
		{
			dockPanel.SaveAsXml("layout.xml");
		}

		private IDockContent getDockingFromPersistString(string persistString)
		{
			if (persistString == typeof(AssertWindow).ToString())
				return assertWindow;
			else if (persistString == typeof(EntityWindow).ToString())
				return entityWindow;
			else if (persistString == typeof(LogWindow).ToString())
				return logWindow;
			else if (persistString == typeof(PropertyWindow).ToString())
				return propertyWindow;

			// For example, the docking state of renderWindow will not be laoded
			return null;
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
		AssertWindow assertWindow;
		LogWindow logWindow;

		Entity currentSelectedEntity;

		private DeserializeDockContent mDeserializeDockContent;
	}
}
