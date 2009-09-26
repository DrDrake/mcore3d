using System;
using System.IO;
using System.Xml;
using System.Xml.Serialization;
using Aga.Controls.Tree;

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
		public MediaPaths()
			: base(ProjectWindow.Singleton.treeViewAdv, ProjectWindow.Singleton.MediaPathNode)
		{
			mFileSystem = ProjectWindow.Singleton.Project.FileSystem;
		}

		public MediaPaths(TreeViewAdv treeView, Node node, Binding.FileSystemCollection fileSystem)
			: base(treeView, node)
		{
			mFileSystem = fileSystem;

			// NOTE: We at least add the resource path that the Studio needs.
			// TODO: Should we embed all the Studio specific resource to the executable?
			mFileSystem.addFileSystem("./");
			mFileSystem.addFileSystem("Media");
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

	/// <summary>
	/// A wrapper class to get around various problem in C# serialization
	/// (in particular, the lack of ignore base class option)
	/// See http://rboxman.spaces.live.com/blog/cns!F0B1967823B96201!235.entry?wa=wsignin1.0&sa=597236082
	/// and http://www.codeproject.com/KB/XML/xmlserializerforunknown.aspx
	/// </summary>
	public class MediaPathXmlWrapper
	{
		#region Basic wrapping
		public MediaPathXmlWrapper() : this(new MediaPath()) { }

		public MediaPathXmlWrapper(MediaPath val)
		{
			this.mVal = val;
		}

		public static implicit operator MediaPath(MediaPathXmlWrapper wrapper)
		{
			return wrapper != null ? wrapper.mVal : null;
		}

		public static implicit operator MediaPathXmlWrapper(MediaPath val)
		{
			return val != null ? new MediaPathXmlWrapper(val) : null;
		}

		[XmlIgnore]
		protected MediaPath mVal;
		#endregion

		[XmlAttribute]
		public string Path
		{
			get { return mVal.Text; }
			set { mVal.Text = value; }
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

	public class SceneXmlWrapper
	{
		#region Basic wrapping
		public SceneXmlWrapper() : this(new Scene()) { }

		public SceneXmlWrapper(Scene val)
		{
			this.mVal = val;
		}

		public static implicit operator Scene(SceneXmlWrapper wrapper)
		{
			return wrapper != null ? wrapper.mVal : null;
		}

		public static implicit operator SceneXmlWrapper(Scene val)
		{
			return val != null ? new SceneXmlWrapper(val) : null;
		}

		[XmlIgnore]
		protected Scene mVal;
		#endregion

		[XmlAttribute]
		public string Name
		{
			get { return mVal.Text; }
			set { mVal.Text = value; }
		}

		[XmlAttribute]
		public string SceneScriptPath
		{
			get { return mVal.SceneScriptPath; }
			set { mVal.SceneScriptPath = value; }
		}

		[XmlAttribute]
		public string InitScriptPath
		{
			get { return mVal.InitScriptPath; }
			set { mVal.InitScriptPath = value; }
		}
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

		[XmlAttribute]
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

		[XmlArrayItem(ElementName = "MediaPath", Type = typeof(MediaPathXmlWrapper))]
		public MediaPaths MediaPaths;

		[XmlArrayItem(ElementName = "Scene", Type = typeof(SceneXmlWrapper))]
		public TreeNodeList<Scene> Scenes;

		/// <summary>
		/// Serialize the Project class to an Xml file
		/// <see cref="http://www.switchonthecode.com/tutorials/csharp-tutorial-xml-serialization"/>
		/// </summary>
		/// <param name="xmlPath"></param>
		/// <param name="preference"></param>
		public static void SerializeToXML(string xmlPath, Project project)
		{
			XmlSerializer serializer = new XmlSerializer(typeof(Project));
			TextWriter textWriter = new StreamWriter(xmlPath);
			serializer.Serialize(textWriter, project);
			textWriter.Close();
		}

		public static Project DeserializeFromXML(string xmlPath)
		{
			Project ret = null;

			XmlSerializer deserializer = new XmlSerializer(typeof(Project));
			TextReader textReader = new StreamReader(xmlPath);

			Project tmp = (Project)deserializer.Deserialize(textReader);
			textReader.Close();

			if (tmp != null)
				ret = tmp;

			return ret;
		}
	}
}
