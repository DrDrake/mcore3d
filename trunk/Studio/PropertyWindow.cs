namespace Studio
{
	public partial class PropertyWindow : DockContent
	{
		public PropertyWindow()
		{
			InitializeComponent();
		}

		private void timer1_Tick(object sender, System.EventArgs e)
		{
			if (!propertyGrid1.ContainsFocus)
				propertyGrid1.Refresh();
		}
	}
}
