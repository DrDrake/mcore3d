namespace Studio
{
	/// <summary>
	/// A base class providing interface for a document.
	/// </summary>
	public class Document : Studio.DockContent
	{
		public Document()
		{
		}

		public Document(string path)
		{
			mPath = path;
		}

		public virtual bool SaveDocument()
		{
			return false;
		}

		public virtual bool LoadDocument()
		{
			return false;
		}

		public string Path
		{
			get { return mPath; }
		}
		private string mPath;
	}
}
