using System.IO;

namespace Studio
{
	public class MediaPath : TreeNodeBase<MediaPath>
	{
		public new string Text
		{
			get { return base.Text; }
			set
			{
				// Check whether the path is valid.
				string fullPath = ProjectWindow.Singleton.ProjectPath + value;

				// TODO: Log to log window any change the text color of the node
				// if the path is invalid instead of throwing exception.
				if (!Directory.Exists(fullPath) && !File.Exists(fullPath))
					throw new IOException("The path does not exist");
				base.Text = value;
			}
		}
	}

	public class Scene : TreeNodeBase<Scene>
	{
		/// <summary>
		/// The script to be run when the scene is loaded into the editor.
		/// </summary>
		public string SceneScriptPath;

		/// <summary>
		/// The script to be run when the play button is pressed.
		/// </summary>
		public string InitScriptPath;
	}

	public class Project
	{
		public Project()
		{
			MediaPaths = new TreeNodeList<MediaPath>(ProjectWindow.Singleton.treeViewAdv, ProjectWindow.Singleton.MediaPathNode);
			Scenes = new TreeNodeList<Scene>(ProjectWindow.Singleton.treeViewAdv, ProjectWindow.Singleton.SceneNode);
		}

		public string Name;
		public TreeNodeList<MediaPath> MediaPaths;
		public TreeNodeList<Scene> Scenes;
	}
}
