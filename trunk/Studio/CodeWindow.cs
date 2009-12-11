using Binding;

namespace Studio
{
	public partial class CodeWindow : Document
	{
		/// <summary>
		/// Using the ScintillaNet control for text editing.
		/// Because of some problme in ScintillaNet, I've made a cutomized version,
		/// see /mocre3d/doc folder for more information.
		/// See: http://scintillanet.codeplex.com/WorkItem/View.aspx?WorkItemId=24671
		/// See: http://scintillanet.codeplex.com/WorkItem/View.aspx?WorkItemId=24757
		/// </summary>
		public CodeWindow(string path)
			: base(path)
		{
			InitializeComponent();

			// See http://scintillanet.codeplex.com/Thread/View.aspx?ThreadId=56517
			ScintillaNet.Marker marker = scintilla.Markers[2];
			marker.BackColor = System.Drawing.Color.AliceBlue; // the ultraedit style
			marker.Symbol = ScintillaNet.MarkerSymbol.RoundRectangle; // the ultraedit style
			this.scintilla.Lines[3].AddMarker(marker);
			this.scintilla.Lines[10].AddMarker(marker);
		}

		public override bool SaveDocument()
		{
			FileSystemCollection fs = ProjectWindow.Singleton.Project.ResourceManager.fileSystemCollection;
			if (fs.saveString(Path, scintilla.Text))
			{
				scintilla.Modified = false;
				if (TabText.EndsWith("*"))
					TabText.Remove(TabText.Length - 1);
			}
			return false;
		}

		private void scintilla_TextChanged(object sender, System.EventArgs e)
		{
			if(!TabText.EndsWith("*"))
				this.TabText += "*";
		}
	}
}
