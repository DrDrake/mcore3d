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
			renderPanel.propertyGrid = propertyWindow.propertyGrid1;
			renderPanel.entitySelectionChanged += new EntitySelectionChangedHandler(onEntitySelectionChanged);
			content.Tag = renderPanel;
			renderControls.Add(renderPanel);
			renderPanel.Enter += new EventHandler(sceneSelectionChanged);
			content.FormClosing += new FormClosingEventHandler(sceneClosing);
			renderPanel.Dock = DockStyle.Fill;
			content.Controls.Add(renderPanel);
			content.TabText = "Scene " + renderControls.Count;

			// Broadcasting the key event from render panel to entity window
			content.KeyUp += new KeyEventHandler(entityWindow.treeView_KeyUp);
			renderPanel.KeyUp += new KeyEventHandler(entityWindow.treeView_KeyUp);

			// Forward the key event from render panel to main window
			content.KeyPress += new KeyPressEventHandler(MainForm_KeyPress);
			renderPanel.KeyPress += new KeyPressEventHandler(MainForm_KeyPress);

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

			if (currentRenderControl != null)
			{
				currentRenderControl.enableAutoUpdate(true);
				entityWindow.selectEntityRoot(currentRenderControl.rootEntity);
				onEntitySelectionChanged(this, currentRenderControl.selectedEntity);
			}
			else
			{
				entityWindow.selectEntityRoot(null);
				onEntitySelectionChanged(this, null);
			}

			enableGizmoButtons(true);
			updateGizmoButtonsState();
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

			if (renderControls.Count == 0)
			{
				clearGizmoButtonsState();
				enableGizmoButtons(false);
			}
		}

		private void exitToolStripMenuItem_Click(object sender, EventArgs e)
		{
			Close();
		}

		private void onEntitySelectionChanged(object sender, Entity entity)
		{
			// Inform other window about the changed selecion
			entityWindow.selectedEntity = entity;
			propertyWindow.propertyGrid1.SelectedObject = entity;
			if (currentRenderControl != null)
				currentRenderControl.selectedEntity = entity;
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

		private DeserializeDockContent mDeserializeDockContent;

		/// <summary>
		/// Redirect Tool bar button click to the menu item.
		/// Reference: http://www.java2s.com/Tutorial/CSharp/0460__GUI-Windows-Forms/ToolBarLinkedWithMenu.htm
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void toolStripMain_ItemClicked(object sender, ToolStripItemClickedEventArgs e)
		{
			ToolStripButton btn = (ToolStripButton)e.ClickedItem;

			if (btn.Tag != null)
			{
				ToolStripDropDownItem item = (ToolStripDropDownItem)btn.Tag;
				item.PerformClick();
			}
		}

		#region "Gizmo buttons"
		private void enableGizmoButtons(bool flag)
		{
			toolStripButtonRotate.Enabled = flag;
			toolStripButtonScale.Enabled = flag;
			toolStripButtonTranslate.Enabled = flag;
		}

		private void clearGizmoButtonsState()
		{
			toolStripButtonRotate.Checked = false;
			toolStripButtonScale.Checked = false;
			toolStripButtonTranslate.Checked = false;
		}

		/// <summary>
		/// Update the Gizmo buttons' state according to the active gizmo in the render panel
		/// </summary>
		private void updateGizmoButtonsState()
		{
			if (currentRenderControl == null)
				return;
			if(currentRenderControl.gizmoMode == RenderPanelControl.GizmoMode.Rotate)
				toolStripButtonRotate.PerformClick();
			if (currentRenderControl.gizmoMode == RenderPanelControl.GizmoMode.Scale)
				toolStripButtonScale.PerformClick();
			if (currentRenderControl.gizmoMode == RenderPanelControl.GizmoMode.Translate)
				toolStripButtonTranslate.PerformClick();
		}

		private void toolStripButtonRotate_Click(object sender, EventArgs e)
		{
			clearGizmoButtonsState();
			toolStripButtonRotate.Checked = true;
			if (currentRenderControl != null)
				currentRenderControl.gizmoMode = RenderPanelControl.GizmoMode.Rotate;
		}

		private void toolStripButtonScale_Click(object sender, EventArgs e)
		{
			clearGizmoButtonsState();
			toolStripButtonScale.Checked = true;
			if (currentRenderControl != null)
				currentRenderControl.gizmoMode = RenderPanelControl.GizmoMode.Scale;
		}

		private void toolStripButtonTranslate_Click(object sender, EventArgs e)
		{
			clearGizmoButtonsState();
			toolStripButtonTranslate.Checked = true;
			if (currentRenderControl != null)
				currentRenderControl.gizmoMode = RenderPanelControl.GizmoMode.Translate;
		}
		#endregion

		private void MainForm_KeyPress(object sender, KeyPressEventArgs e)
		{
			if (e.KeyChar == 'q')
				toolStripButtonTranslate.PerformClick();
			if (e.KeyChar == 'w')
				toolStripButtonRotate.PerformClick();
			if (e.KeyChar == 'e')
				toolStripButtonScale.PerformClick();
		}
	}
}
