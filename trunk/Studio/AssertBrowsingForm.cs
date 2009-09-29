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

		/// <summary>
		/// Just hide the form instead of closing it.
		/// </summary>
		private void AssertBrowsingForm_FormClosing(object sender, FormClosingEventArgs e)
		{
			Visible = false;
			e.Cancel = true;
		}

		private void treeView1_AfterSelect(object sender, TreeViewEventArgs e)
		{
			listView1.Items.Clear();

			string directoryName = treeView1.SelectedNode.FullPath;

			textBoxFolderPath.Text = directoryName;

			foreach (string fileName in mFsCollection.getFiles(directoryName))
			{
				listView1.Items.Add(fileName);
			}
		}

		private void button1_Click(object sender, EventArgs e)
		{

		}
	}
}
