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

		private void AssertBrowsingForm_Load(object sender, EventArgs e)
		{
			AssertTreeViewControl c = new AssertTreeViewControl(mFsCollection);
			c.Dock = DockStyle.Fill;
			this.splitContainer1.Panel1.Controls.Add(c);
			c.refresh("");
		}

		private FileSystemCollection mFsCollection;
	}
}
