using System;
using System.Windows.Forms;
using Binding;

namespace Studio
{
	public partial class AssertBrowsingForm : Form
	{
		public AssertBrowsingForm(FileSystemCollection fs)
		{
			InitializeComponent();
			mFsCollection = fs;
		}

		private FileSystemCollection mFsCollection;

		private void AssertBrowsingForm_Load(object sender, EventArgs e)
		{
			treeView1.FsCollection = mFsCollection;
			treeView1.refresh("");
		}

		private void treeView1_AfterSelect(object sender, TreeViewEventArgs e)
		{
			listView1.Items.Clear();

			string directoryName = treeView1.SelectedNode.FullPath;

			foreach (string fileName in mFsCollection.getFiles(directoryName))
			{
				listView1.Items.Add(fileName);
			}
		}
	}
}
