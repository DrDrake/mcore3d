
namespace Studio
{
	public partial class CodeWindow : DockContent
	{
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
