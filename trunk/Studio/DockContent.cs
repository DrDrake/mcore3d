namespace Studio
{
	public partial class DockContent : WeifenLuo.WinFormsUI.Docking.DockContent
	{
		public DockContent()
		{
			InitializeComponent();
		}

		private void DockContent_Enter(object sender, System.EventArgs e)
		{
			// Propagate the focusing event to child controls
			foreach (System.Windows.Forms.Control c in Controls)
				c.Focus();
		}
	}
}
