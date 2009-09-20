using Aga.Controls.Tree;
using System.Windows.Forms;

namespace Studio
{
	public partial class ProjectWindow : DockContent
	{
		public static ProjectWindow Singleton;

		public ProjectWindow()
		{
			InitializeComponent();
			Singleton = this;
			ProjectPath = System.IO.Directory.GetCurrentDirectory();

			TreeModel model = new TreeModel();
			treeViewAdv.Model = new SortedTreeModel(model);

			SceneNode = new FixedNode("Scenes");
			model.Nodes.Add(SceneNode);

			MediaPathNode = new FixedNode("Media paths");
			model.Nodes.Add(MediaPathNode);

			mProject = new Project();
		}

		public Node SceneNode;
		public Node MediaPathNode;

		/// <summary>
		/// Where this project is saved.
		/// All other paths are relative to this path, for instance the media paths
		/// and the scene scrips path.
		/// </summary>
		public string ProjectPath;

		public Project Project
		{
			get { return mProject; }
		}
		private Project mProject;

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

		}

		private void addPathToolStripMenuItem_Click(object sender, System.EventArgs e)
		{
			FolderBrowserDialog d = new FolderBrowserDialog();
			d.SelectedPath = ProjectPath;

			if(d.ShowDialog() != DialogResult.OK)
				return;

			MediaPath path = new MediaPath();
			path.Text = d.SelectedPath;
			mProject.MediaPaths.Add(path);
			treeViewAdv.FindNodeByTag(MediaPathNode).Expand(true);
		}
	}

	class FixedNode : Node
	{
		public FixedNode(string text)
			: base(text)
		{
		}

		public new string Text
		{
			get
			{
				return base.Text;
			}
		}
	}
}
