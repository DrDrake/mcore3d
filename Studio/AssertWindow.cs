using System.IO;
using System.Windows.Forms;

namespace Studio
{
	public partial class AssertWindow : DockContent
	{
		public AssertWindow()
		{
			InitializeComponent();
		}

		public void refresh(string dir)
		{
			treeView.Nodes.Clear();
			populateTree(dir, treeView.Nodes);
		}

		/// <summary>
		/// A recursive method to populate a TreeView with directories, subdirectories, etc
		/// </summary>
		/// <param name="dir">The path of the directory</param>
		/// <param name="nodes">The "master" node's collection, to populate</param>
		///	<seealso cref="http://www.dreamincode.net/code/snippet2591.htm"/>
		void populateTree(string dir, TreeNodeCollection nodes)
		{
			// Get the information of the directory
			DirectoryInfo directory = new DirectoryInfo(dir);

			// Loop through each subdirectory
			foreach (DirectoryInfo d in directory.GetDirectories())
			{
				// Creates a new node
				TreeNode t = new TreeNode(d.Name);
				// Populate the new node recursively
				populateTree(d.FullName, t.Nodes);
				nodes.Add(t);	// Add the node to the "master" node
			}

			// Lastly, loop through each file in the directory, and add these as nodes
			foreach (FileInfo f in directory.GetFiles())
			{
				// Create a new node
				TreeNode t = new TreeNode(f.Name);
				// Add it to the "master"
				nodes.Add(t);
			}
		}
	}
}
