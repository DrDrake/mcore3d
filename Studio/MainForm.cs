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
			mDeserializeDockContent = new DeserializeDockContent(GetDockingFromPersistString);
			Singleton = this;
		}

	// Properties
		public static MainForm Singleton;

	// Operations
		public void CloseAllDocuments()
		{
			List<IDockContent> dockContent = new List<IDockContent>();
			dockContent.AddRange(dockPanel.Documents);

			foreach (IDockContent _d in dockContent)
			{
				DockContent d = _d as DockContent;
				if (d == null)
					continue;

				d.Close();
				if (d.Tag is RenderPanelControl)
					(d.Tag as RenderPanelControl).destroy();
			}
		}

		public void OpenScene(String[] scripts)
		{
		}

		private void UpdateToolBars()
		{
			if (currentRenderControl == null)
				return;

			ComponentResourceManager resources = new ComponentResourceManager(typeof(MainForm));
			toolStripButtonPlay.Image = ((System.Drawing.Image)(
				resources.GetObject(currentRenderControl.playing ? "toolStripButtonStop.Image" : "toolStripButtonPlay.Image"))
			);
		}

		private IDockContent GetDockingFromPersistString(string persistString)
		{
			if (persistString == typeof(ProjectWindow).ToString())
				return projectWindow;
			else if (persistString == typeof(AssertWindow).ToString())
				return assertWindow;
			else if (persistString == typeof(EntityWindow).ToString())
				return entityWindow;
			else if (persistString == typeof(LogWindow).ToString())
				return logWindow;
			else if (persistString == typeof(PropertyWindow).ToString())
				return propertyWindow;
			else if (persistString == typeof(MemoryProfilerWindow).ToString())
				return memoryProfilerWindow;

			// For example, the docking state of renderWindow will not be laoded
			return null;
		}

	// Attributes
		/// <summary>
		/// The current selected rendering panel.
		/// </summary>
		RenderPanelControl currentRenderControl;

		/// <summary>
		/// A list of all rendering panels.
		/// </summary>
		public List<RenderPanelControl> renderControls;

		public ProjectWindow projectWindow;
		public EntityWindow entityWindow;
		public PropertyWindow propertyWindow;
		AssertWindow assertWindow;
		LogWindow logWindow;
		MemoryProfilerWindow memoryProfilerWindow;
		public AssertBrowsingForm assertBrowsingForm;

		public UserPreference UserPreference;
		private DeserializeDockContent mDeserializeDockContent;
		
	// Events
		private void MainForm_Load(object sender, EventArgs e)
		{
			UserPreference = UserPreference.DeserializeFromXML("userPreference.xml");

			// NOTE: Starting up the form is quite time-consumming, especially for the CodeWindow
			// See http://msdn.microsoft.com/en-us/library/cc656914.aspx
			// and http://msdn.microsoft.com/en-us/magazine/cc163655.aspx
			// to improve startup time.
			projectWindow = new ProjectWindow(this);
			assertWindow = new AssertWindow();
			entityWindow = new EntityWindow();
			logWindow = new LogWindow();
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
				propertyWindow.Show(dockPanel);
				memoryProfilerWindow.Show(dockPanel);
			}

			dockPanel.ResumeLayout(true, true);
		}

		public void OpenScene(Scene scene)
		{
			RenderWindow renderWindow = new RenderWindow();
			renderWindow.Show(dockPanel, DockState.Document);

			IntPtr sharedGlContext = new IntPtr(0);

			if (renderControls.Count > 0)
				sharedGlContext = renderControls[0].glContext;

			RenderPanelControl renderPanel = renderWindow.Init(sharedGlContext, scene);
			renderControls.Add(renderPanel);
			renderPanel.Enter += new EventHandler(sceneSelectionChanged);
			renderPanel.entitySelectionChanged += new EntitySelectionChangedHandler(onEntitySelectionChanged);
			renderWindow.FormClosing += new FormClosingEventHandler(sceneClosing);

			// Selected the newly created scene
			sceneSelectionChanged(renderPanel, new EventArgs());

			toolStripDebug.Enabled = true;

			// Execute all scene setup scripts
			foreach (SceneScript s in scene.SceneScripts)
				renderPanel.executeScript(s.Path);
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
				toolStripDebug.Enabled = false;
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
			if(projectWindow.Project != null)
				projectWindow.Project.ResourceManager.destroy();
		}

		private void MainForm_FormClosed(object sender, FormClosedEventArgs e)
		{
			dockPanel.SaveAsXml("layout.xml");
			UserPreference.SerializeToXML("userPreference.xml", UserPreference);
		}

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

		private void toolStripButtonPlay_Click(object sender, EventArgs e)
		{
			RenderWindow renderWindow = currentRenderControl.Tag as RenderWindow;

			if (currentRenderControl.playing)
				currentRenderControl.stop();
			else
				currentRenderControl.play(renderWindow.Scene.StarupScript.Path);

			if(currentRenderControl != null)
				entityWindow.selectEntityRoot(currentRenderControl.rootEntity);
			UpdateToolBars();
		}

		private void assetBrowserToolStripMenuItem_Click(object sender, EventArgs e)
		{
			assertBrowsingForm.Show();
		}
	}
}
