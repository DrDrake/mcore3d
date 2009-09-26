using System.Windows.Forms;
using Binding;
using System.Collections.Specialized;

namespace Studio
{
	public partial class AssertTreeViewControl : UserControl
	{
		public AssertTreeViewControl(FileSystemCollection fsCollection)
		{
			InitializeComponent();
			mFsCollection = fsCollection;
		}

		public void refresh(string dir)
		{
			treeView1.Nodes.Clear();
			populateTree(dir, treeView1.Nodes, 2);
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

			// Put the directory.GetDirectories() in a try block in case any error.
			try
			{
				// Loop through each subdirectory
				foreach (string d in mFsCollection.getDirectories(dir))
				{
					// Creates a new node
					TreeNode t = new TreeNode(d);
					// Populate the new node recursively
					populateTree(dir + "/" + d, t.Nodes, depth - 1);
					nodes.Add(t);	// Add the node to the "master" node
				}
			}
			catch
			{
			}

			try
			{
				// Lastly, loop through each file in the directory, and add these as nodes
				foreach (string f in mFsCollection.getFiles(dir))
				{
					// Create a new node
					TreeNode t = new TreeNode(f);
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

		private void treeView1_BeforeExpand(object sender, TreeViewCancelEventArgs e)
		{
			treeView1.BeginUpdate();

			populateTree(e.Node.FullPath, e.Node.Nodes, 2);

			treeView1.EndUpdate();
		}

		private FileSystemCollection mFsCollection;
	}
}
