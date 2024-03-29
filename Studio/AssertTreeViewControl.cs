﻿using System.Windows.Forms;
using Binding;
using System.Collections.Specialized;
using System.ComponentModel;

namespace Studio
{
	public partial class AssertTreeViewControl : UserControl
	{
		public AssertTreeViewControl()
		{
			InitializeComponent();

			SkipList = new StringCollection();
			SkipList.Add(".svn");
		}

		public void refresh(string dir)
		{
			treeView1.Nodes.Clear();
			TreeNode n = new TreeNode("\\");
			treeView1.Nodes.Add(n);
			populateTree(dir, n.Nodes, 2);
			n.Expand();
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

			// Remove the first slash, if any
			if (dir.Length > 0 && dir[0] == '/')
				dir = dir.Remove(0, 1);

			nodes.Clear();

			// Put the directory.GetDirectories() in a try block in case any error.
			try
			{
				// Loop through each subdirectory
				foreach (string d in FsCollection.getDirectories(dir))
				{
					if (SkipList.Contains(d))
						continue;

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
/*				foreach (string f in FsCollection.getFiles(dir))
				{
					// Create a new node
					TreeNode t = new TreeNode(f);
					t.ImageIndex = 1;
					t.SelectedImageIndex = 1;
					// Add it to the "master"
					nodes.Add(t);
				}*/
			}
			catch
			{
			}
		}

		private void treeView1_BeforeExpand(object sender, TreeViewCancelEventArgs e)
		{
			Cursor = Cursors.WaitCursor;
			treeView1.BeginUpdate();

			populateTree(e.Node.FullPath, e.Node.Nodes, 2);

			treeView1.EndUpdate();
			Cursor = Cursors.Arrow;
		}

		public FileSystemCollection FsCollection;

		/// <summary>
		/// List of folder to be excluded.
		/// </summary>
		public StringCollection SkipList;

		public event TreeViewEventHandler AfterSelect;

		public TreeNode SelectedNode
		{
			get { return treeView1.SelectedNode; }
			set { treeView1.SelectedNode = value; }
		}

		private void treeView1_AfterSelect(object sender, TreeViewEventArgs e)
		{
			if(AfterSelect != null)
				AfterSelect(sender, e);
		}
	}
}
