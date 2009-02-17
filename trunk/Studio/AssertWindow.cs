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
			populateTree(dir, treeView.Nodes, 2);
		}

		/// <summary>
		/// A recursive method to populate a TreeView with directories, subdirectories, etc
		/// </summary>
		/// <param name="dir">The path of the directory</param>
		/// <param name="nodes">The "master" node's collection, to populate</param>
		/// <param name="depth">The recursive level</param>
		///	<seealso cref="http://www.dreamincode.net/code/snippet2591.htm"/>
		void populateTree(string dir, TreeNodeCollection nodes, int depth)
		{
			if (depth <= 0)
				return;

			nodes.Clear();

			// Get the information of the directory
			DirectoryInfo directory = new DirectoryInfo(dir);

			// Put the directory.GetDirectories() in a try block in case any error.
			try
			{
				// Loop through each subdirectory
				foreach (DirectoryInfo d in directory.GetDirectories())
				{
					// Creates a new node
					TreeNode t = new TreeNode(d.Name);
					// Populate the new node recursively
					populateTree(d.FullName, t.Nodes, depth - 1);
					nodes.Add(t);	// Add the node to the "master" node
				}
			}
			catch
			{
			}

			try
			{
				// Lastly, loop through each file in the directory, and add these as nodes
				foreach (FileInfo f in directory.GetFiles())
				{
					// Create a new node
					TreeNode t = new TreeNode(f.Name);
					t.ImageIndex = 1;
					t.SelectedImageIndex = 1;
					// Add it to the "master"
					nodes.Add(t);
				}
			}
			catch
			{
			}
		}

		/// <summary>
		/// Only add more nodes when necessary.
		/// </summary>
		private void treeView_BeforeExpand(object sender, TreeViewCancelEventArgs e)
		{
			treeView.BeginUpdate();

			populateTree("Media/" + e.Node.FullPath, e.Node.Nodes, 2);

			treeView.EndUpdate();
		}
	}
}
