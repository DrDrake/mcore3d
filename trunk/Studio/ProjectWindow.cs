using System;
using System.Windows.Forms;
using Aga.Controls.Tree;

namespace Studio
{
	public partial class ProjectWindow : DockContent
	{
		public static ProjectWindow Singleton;

		public ProjectWindow()
		{
			InitializeComponent();
			Singleton = this;
			mContext = new Context(treeViewAdv);
			mContext.Project = new Project();
		}

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
				mContext.Project = Project.DeserializeFromXML(path);
				mContext.ProjectPath = path;
				treeViewAdv.FindNodeByTag(MediaPathNode).Expand(true);
				return true;
			}
			catch(Exception ex)
			{
				// In case of any error, restore to the previous context
				mContext = backupContext;
				treeViewAdv.Model = mContext.TreeModel;

				MessageBox.Show("Load project failed!\n" + ex.Message);
				return false;
			}
		}

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

		public Project Project
		{
			get { return mContext.Project; }
		}

		private Context mContext;

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
			d.SelectedPath = ProjectPath;

			if(d.ShowDialog() != DialogResult.OK)
				return;

			MediaPath path = new MediaPath();
			path.Text = d.SelectedPath;
			Project.MediaPaths.Add(path);
			treeViewAdv.FindNodeByTag(MediaPathNode).Expand(true);
		}
	}

	/// <summary>
	/// A context for exception handling when loading new project.
	/// </summary>
	class Context
	{
		public Context(TreeViewAdv treeView)
		{
			ProjectPath = System.IO.Directory.GetCurrentDirectory();

			TreeModel model = new TreeModel();
			TreeModel = new SortedTreeModel(model);
			treeView.Model = TreeModel;

			SceneNode = new FixedNode("Scenes");
			model.Nodes.Add(SceneNode);

			MediaPathNode = new FixedNode("Media paths");
			model.Nodes.Add(MediaPathNode);
		}

		public SortedTreeModel TreeModel;
		public Node SceneNode;
		public Node MediaPathNode;
		public string ProjectPath;
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
