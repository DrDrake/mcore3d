﻿using System;
using System.Windows.Forms;
using Aga.Controls.Tree;
using Binding;
using System.IO;

namespace Studio
{
	public partial class ProjectWindow : DockContent
	{
		public static ProjectWindow Singleton;

		public ProjectWindow(MainForm mainForm)
		{
			InitializeComponent();
			Singleton = this;
//			mContext = new Context(treeViewAdv);
//			mContext.Project = new Project();
			mMainForm = mainForm;

			ToolStripManager.Merge(this.MainMenuStrip, MainForm.Singleton.MainMenuStrip);
			UpdateRecentProjectList();
		}

	// Operations
		/// <summary>
		/// Make a new project, the main window should responsible for cleanning up
		/// all the scene windows before calling this.
		/// </summary>
		public void NewProject()
		{
			mContext = new Context(treeViewAdv);
			mContext.Project = new Project();
		}

		public void SaveProject(string path)
		{
			Project.SerializeToXML(path, Project);
		}

		/// <summary>
		/// Open an existing project, the main window should responsible for cleanning up
		/// all the scene windows before calling this.
		/// </summary>
		public bool LoadProject(string path)
		{
			Context backupContext = mContext;

			try
			{
				mContext = new Context(treeViewAdv);
				mContext.ProjectPath = path;
				mContext.Project = Project.DeserializeFromXML(path);
				return true;
			}
			catch(Exception ex)
			{
				// In case of any error, restore to the previous context
				mContext = backupContext;
				if(mContext != null)
					treeViewAdv.Model = mContext.TreeModel;

				MessageBox.Show("Load project failed!\n" + ex.Message);
				return false;
			}
		}

		private void NewProject(string path)
		{
			// Close all scene window and destroy the original resource manager
			mMainForm.CloseAllDocuments();

			if(Project != null)
				Project.ResourceManager.destroy();

			mContext = new Context(treeViewAdv);
			mContext.ProjectPath = path;
			mContext.Project = new Project();

			// Inform the user-preference
			mMainForm.UserPreference.OpenProject(path);
			UpdateRecentProjectList();

			SaveProject(path);
		}

		private void OpenProject(string path)
		{
			// Only destroy the old manager if the loading success
			ResourceManager oldMgr = Project == null ? null : Project.ResourceManager;

			// LoadProject() should handled all exception.
			if (LoadProject(path))
			{
				// Close all scene window
				mMainForm.CloseAllDocuments();

				if(oldMgr != null)
					oldMgr.destroy();
			}

			// Inform the user-preference
			mMainForm.UserPreference.OpenProject(path);
			UpdateRecentProjectList();
		}

		private void UpdateRecentProjectList()
		{
			recentProjectsToolStripMenuItem.DropDownItems.Clear();
			foreach (string path in mMainForm.UserPreference.RecentProjects)
			{
				ToolStripMenuItem i = new ToolStripMenuItem();
				i.Click += new EventHandler(openRecentProject);
				i.Text = path;
				recentProjectsToolStripMenuItem.DropDownItems.Add(i);
			}
		}

	// Attributes
		public Node SceneNode
		{
			get { return mContext.SceneNode; }
		}

		public Node MediaPathNode
		{
			get { return mContext.MediaPathNode; }
		}

		/// <summary>
		/// Where this project is saved.
		/// All other paths are relative to this path, for instance the media paths
		/// and the scene scrips path.
		/// </summary>
		public string ProjectPath
		{
			get { return mContext.ProjectPath; }
			set { mContext.ProjectPath = value; }
		}

		public string ProjectDirectory
		{
			get { return mContext.ProjectDirectory; }
		}

		public Project Project
		{
			get { return mContext == null ? null : mContext.Project; }
		}

		private Context mContext;
		private MainForm mMainForm;

	// Events
		private void ProjectWindow_Load(object sender, EventArgs e)
		{
		}

		private void treeViewAdv1_MouseDown(object sender, MouseEventArgs e)
		{
			treeViewAdv.ContextMenuStrip = null;

			if (treeViewAdv.SelectedNode == null || treeViewAdv.SelectedNode.Tag == null)
				return;

			if (treeViewAdv.SelectedNode.Tag as Node == SceneNode)
				treeViewAdv.ContextMenuStrip = sceneMenuStrip;
			if (treeViewAdv.SelectedNode.Tag as Node == MediaPathNode)
				treeViewAdv.ContextMenuStrip = mediaPathMenuStrip;
		}

		private void addSceneToolStripMenuItem_Click(object sender, System.EventArgs e)
		{
			Scene s = new Scene();
			s.Text = "New scene";
			Project.Scenes.Add(s);
			treeViewAdv.FindNodeByTag(SceneNode).Expand(true);
		}

		private void addPathToolStripMenuItem_Click(object sender, System.EventArgs e)
		{
			FolderBrowserDialog d = new FolderBrowserDialog();
			d.SelectedPath = ProjectDirectory;

			if(d.ShowDialog() != DialogResult.OK)
				return;

			MediaPath path = new MediaPath();
			path.Text = d.SelectedPath;
			Project.MediaPaths.Add(path);
			treeViewAdv.FindNodeByTag(MediaPathNode).Expand(true);
		}

		private void newProjectToolStripMenuItem_Click(object sender, EventArgs e)
		{
			SaveFileDialog d = new SaveFileDialog();
			d.Title = "Select a location for the new project";
			d.Filter = "Xml files (*.xml)|*.xml|All files (*.*)|*.*";
			d.RestoreDirectory = true; 

			if (d.ShowDialog(this) != DialogResult.OK)
				return;

			NewProject(d.FileName);
		}

		private void openProjectToolStripMenuItem_Click(object sender, EventArgs e)
		{
			OpenFileDialog d = new OpenFileDialog();
//			d.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.Personal);
			d.Filter = "Xml files (*.xml)|*.xml|All files (*.*)|*.*";
			d.RestoreDirectory = true;

			if (d.ShowDialog(this) != DialogResult.OK)
				return;

			OpenProject(d.FileName);
		}

		private void openRecentProject(object sender, EventArgs e)
		{
			ToolStripMenuItem m = (ToolStripMenuItem)sender;
			OpenProject(m.Text);
		}

		private void saveProjectToolStripMenuItem_Click(object sender, EventArgs e)
		{
			SaveProject(ProjectPath);
		}

		private void saveProjectAsToolStripMenuItem_Click(object sender, EventArgs e)
		{
			SaveFileDialog d = new SaveFileDialog();
			d.InitialDirectory = ProjectDirectory;
			d.Filter = "Xml files (*.xml)|*.xml|All files (*.*)|*.*";
			d.RestoreDirectory = true;

			if (d.ShowDialog(this) != DialogResult.OK)
				return;

			SaveProject(d.FileName);
		}
	}

	/// <summary>
	/// A context for exception handling when loading new project.
	/// </summary>
	class Context
	{
		public Context(TreeViewAdv treeView)
		{
//			ProjectPath = System.IO.Directory.GetCurrentDirectory();

			TreeModel model = new TreeModel();
			TreeModel = new SortedTreeModel(model);
			treeView.Model = TreeModel;

			SceneNode = new FixedNode("Scenes");
			model.Nodes.Add(SceneNode);

			MediaPathNode = new FixedNode("Media paths");
			model.Nodes.Add(MediaPathNode);

			treeView.FindNodeByTag(SceneNode).AutoExpandOnStructureChanged = true;
			treeView.FindNodeByTag(MediaPathNode).AutoExpandOnStructureChanged = true;
		}

		public SortedTreeModel TreeModel;
		public Node SceneNode;
		public Node MediaPathNode;
		public string ProjectPath;
		public string ProjectDirectory
		{
			get { return Path.GetDirectoryName(ProjectPath) + "\\"; }
		}
		public Project Project;
	}

	/// <summary>
	/// Some default nodes that are fixed on the tree view.
	/// </summary>
	class FixedNode : Node
	{
		public FixedNode(string text)
			: base(text)
		{
		}

		public new string Text
		{
			get { return base.Text; }
		}
	}
}
