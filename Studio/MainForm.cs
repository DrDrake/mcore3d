using System;
using System.Collections.Generic;
using System.ComponentModel;
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
			Singleton = this;
		}

	// Properties
		MainForm Singleton;

	// Operations
		void UpdateToolBars()
		{
			if (currentRenderControl == null)
				return;

			toolStripGizmo.Enabled = !currentRenderControl.playing;

			ComponentResourceManager resources = new ComponentResourceManager(typeof(MainForm));
			toolStripButtonPlay.Image = ((System.Drawing.Image)(
				resources.GetObject(currentRenderControl.playing ? "toolStripButtonStop.Image" : "toolStripButtonPlay.Image"))
			);
		}

	// Events
		private void MainForm_Load(object sender, EventArgs e)
		{
			// NOTE: Starting up the form is quite time-consumming, especially for the CodeWindow
			// See http://msdn.microsoft.com/en-us/library/cc656914.aspx
			// and http://msdn.microsoft.com/en-us/magazine/cc163655.aspx
			// to improve startup time.
			projectWindow = new ProjectWindow();
			assertWindow = new AssertWindow();
			entityWindow = new EntityWindow();
			logWindow = new LogWindow();
			codeWindow = new CodeWindow();
			propertyWindow = new PropertyWindow();
			memoryProfilerWindow = new MemoryProfilerWindow();

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
				projectWindow.Show(dockPanel);
				assertWindow.Show(dockPanel);
				entityWindow.Show(dockPanel);
				logWindow.Show(dockPanel);
				codeWindow.Show(dockPanel);
				propertyWindow.Show(dockPanel);
				memoryProfilerWindow.Show(dockPanel);
			}

			dockPanel.ResumeLayout(true, true);
		}

		private void newToolStripMenuItem_Click(object sender, EventArgs e)
		{
			DockContent content = new DockContent();
			content.Show(dockPanel, DockState.Document);

			IntPtr sharedGlContext = new IntPtr(0);

			if (renderControls.Count > 0)
				sharedGlContext = renderControls[0].glContext;

			RenderPanelControl renderPanel = new RenderPanelControl(ProjectWindow.Singleton.Project.ResourceManager, sharedGlContext);
			renderPanel.propertyGrid = propertyWindow.propertyGrid1;
			renderPanel.entitySelectionChanged += new EntitySelectionChangedHandler(onEntitySelectionChanged);
			content.Tag = renderPanel;
			renderPanel.Tag = content;
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

			toolStripGizmo.Enabled = true;
			toolStripDebug.Enabled = true;
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

			updateGizmoButtonsState();
			UpdateToolBars();
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
				toolStripGizmo.Enabled = false;
				toolStripDebug.Enabled = false;
				clearGizmoButtonsState();
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

		private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
		{
			// Ensure proper destroy sequence.
			foreach (RenderPanelControl r in renderControls)
				r.destroy();
			projectWindow.Project.ResourceManager.destroy();
		}

		private void MainForm_FormClosed(object sender, FormClosedEventArgs e)
		{
			dockPanel.SaveAsXml("layout.xml");
		}

		private IDockContent getDockingFromPersistString(string persistString)
		{
			if (persistString == typeof(ProjectWindow).ToString())
				return projectWindow;
			else if (persistString == typeof(AssertWindow).ToString())
				return assertWindow;
			else if (persistString == typeof(EntityWindow).ToString())
				return entityWindow;
			else if (persistString == typeof(LogWindow).ToString())
				return logWindow;
			else if (persistString == typeof(CodeWindow).ToString())
				return codeWindow;
			else if (persistString == typeof(PropertyWindow).ToString())
				return propertyWindow;
			else if (persistString == typeof(MemoryProfilerWindow).ToString())
				return memoryProfilerWindow;

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

		ProjectWindow projectWindow;
		EntityWindow entityWindow;
		PropertyWindow propertyWindow;
		AssertWindow assertWindow;
		LogWindow logWindow;
		CodeWindow codeWindow;
		MemoryProfilerWindow memoryProfilerWindow;

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
			switch (e.KeyChar)
			{
				case '1':
					toolStripButtonTranslate.PerformClick();
					break;
				case '2':
					toolStripButtonRotate.PerformClick();
					break;
				case '3':
					toolStripButtonScale.PerformClick();
					break;
			}
		}

		private void toolStripButtonMove_Click(object sender, EventArgs e)
		{
		}

		private void toolStripGizmo_ItemClicked(object sender, ToolStripItemClickedEventArgs e)
		{

		}

		private void toolStripButtonPlay_Click(object sender, EventArgs e)
		{
			currentRenderControl.playing = !currentRenderControl.playing;
			if(currentRenderControl != null)
				entityWindow.selectEntityRoot(currentRenderControl.rootEntity);
			UpdateToolBars();
		}

		private void saveToolStripMenuItem_Click(object sender, EventArgs e)
		{
			SaveFileDialog d = new SaveFileDialog();
			d.Filter = "Xml files (*.xml)|*.xml|All files (*.*)|*.*";
			d.AutoUpgradeEnabled = false;

			if (d.ShowDialog() != DialogResult.OK)
				return;

			projectWindow.SaveProject(d.FileName);
		}

		private void openToolStripMenuItem_Click(object sender, EventArgs e)
		{
			OpenFileDialog d = new OpenFileDialog();
			d.AutoUpgradeEnabled = false;
			d.Filter = "Xml files (*.xml)|*.xml|All files (*.*)|*.*";
			if (d.ShowDialog() != DialogResult.OK)
				return;

			// Only destroy the old manager if the loading success
			ResourceManager oldMgr = projectWindow.Project.ResourceManager;

			if (projectWindow.LoadProject(d.FileName))
			{
				// Close all scene window
				while (renderControls.Count != 0)
				{
					RenderPanelControl r = renderControls[0];
					(r.Tag as DockContent).Close();
					r.destroy();
				}
				oldMgr.destroy();
			}
		}
	}
}
