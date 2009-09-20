using System.IO;
using Aga.Controls.Tree;
using System;

namespace Studio
{
	public class MediaPath : TreeNodeBase<MediaPath>
	{
		public MediaPath()
		{
		}

		public MediaPath(string path)
		{
			Text = path;
		}

		public new string Text
		{
			get { return base.Text; }

			// TODO: Should we uncache all resources in ResourceManager or simply warn the user?
			set
			{
				string projectPath = ProjectWindow.Singleton.ProjectPath;
				value = Utility.RelativePathTo(projectPath, value, '/');

				if (Path.IsPathRooted(value))
					throw new Exception("Media path cannot cross the root of the project file: " + projectPath);

				// Make it more visible to the user, instead of an empty string
				if (value == "")
					value = "./";

				// Check whether the path is valid.
				string fullPath = Path.Combine(projectPath, value);

				// TODO: Log to log window any change the text color of the node
				// if the path is invalid instead of throwing exception.
				if (!Directory.Exists(fullPath) && !File.Exists(fullPath))
					throw new IOException("The path does not exist");

				base.Text = value;
			}
		}
	}

	public class MediaPaths : TreeNodeList<MediaPath>
	{
		public MediaPaths(TreeViewAdv treeView, Node node, Binding.FileSystemCollection fileSystem)
			: base(treeView, node)
		{
			mFileSystem = fileSystem;

			// TODO: temp
			Add(new MediaPath("./"));
			Add(new MediaPath("Media"));
		}

		public virtual new void Add(MediaPath path)
		{
			foreach (MediaPath p in this)
			{
				if(p.Text == path.Text)
					throw new Exception("The path: \"" + path.Text + "\" already exist");
			}

			base.Add(path);
			mFileSystem.addFileSystem(path.Text);
		}

		public virtual new void Remove(MediaPath path)
		{
			// TODO: Should we uncache all resources in ResourceManager or simply warn the user?
			base.Remove(path);
			mFileSystem.removeFileSystem(path.Text);
		}

		private Binding.FileSystemCollection mFileSystem;
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
			mFileSystem = new Binding.FileSystemCollection();
			mResourceManager = new Binding.ResourceManager(mFileSystem);
			MediaPaths = new MediaPaths(ProjectWindow.Singleton.treeViewAdv, ProjectWindow.Singleton.MediaPathNode, FileSystem);
			Scenes = new TreeNodeList<Scene>(ProjectWindow.Singleton.treeViewAdv, ProjectWindow.Singleton.SceneNode);
		}

		public string Name;

		public Binding.FileSystemCollection FileSystem
		{
			get { return mFileSystem; }
		}
		private Binding.FileSystemCollection mFileSystem;

		public Binding.ResourceManager ResourceManager
		{
			get { return mResourceManager; }
		}
		private Binding.ResourceManager mResourceManager;

		public MediaPaths MediaPaths;
		public TreeNodeList<Scene> Scenes;
	}
}
