
namespace Studio
{
	public partial class CodeWindow : DockContent
	{
		/// <summary>
		/// Using the ScintillaNet control for text editing.
		/// Because of some problme in ScintillaNet, I've made a cutomized version,
		/// see /mocre3d/doc folder for more information.
		/// See: http://scintillanet.codeplex.com/WorkItem/View.aspx?WorkItemId=24671
		/// See: http://scintillanet.codeplex.com/WorkItem/View.aspx?WorkItemId=24757
		/// </summary>
		public CodeWindow()
		{
			InitializeComponent();

			// See http://scintillanet.codeplex.com/Thread/View.aspx?ThreadId=56517
			ScintillaNet.Marker marker = scintilla1.Markers[2];
			marker.BackColor = System.Drawing.Color.AliceBlue; // the ultraedit style
			marker.Symbol = ScintillaNet.MarkerSymbol.RoundRectangle; // the ultraedit style
			this.scintilla1.Lines[3].AddMarker(marker);
			this.scintilla1.Lines[10].AddMarker(marker);
		}
	}
}
