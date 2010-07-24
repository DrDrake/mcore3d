using System;
using System.Collections.Specialized;
using System.IO;

namespace Studio
{
	public class Utility
	{
		/// <summary>
		/// Creates a relative path from one file or folder to another.
		/// </summary>
		/// <param name="fromDirectory">
		/// Contains the directory that defines the start of the relative path.
		/// </param>
		/// <param name="toPath">
		/// Contains the path that defines the endpoint of the relative path.
		/// </param>
		/// <returns>
		/// The relative path from the start directory to the end path.
		/// Null if any of the input is null.
		/// <remarks>Reference: http://weblogs.asp.net/pwelter34/archive/2006/02/08/create-a-relative-path-code-snippet.aspx</remarks>
		/// </returns>
		public static string RelativePathTo(string fromDirectory, string toPath, char outputSeparator)
		{
			if (fromDirectory == null || toPath == null)
				return null;

			bool isRooted = Path.IsPathRooted(fromDirectory) && Path.IsPathRooted(toPath);

			if (isRooted)
			{
				bool isDifferentRoot = string.Compare(
					Path.GetPathRoot(fromDirectory),
					Path.GetPathRoot(toPath), true
				) != 0;

				if (isDifferentRoot)
					return toPath;
			}

			StringCollection relativePath = new StringCollection();
			string[] separators = new string[2] {"\\", "/"};
			string[] fromDirectories = fromDirectory.Split(separators, StringSplitOptions.RemoveEmptyEntries);
			string[] toDirectories = toPath.Split(separators, StringSplitOptions.RemoveEmptyEntries);
			int length = Math.Min(fromDirectories.Length, toDirectories.Length);
			int lastCommonRoot = -1;

			// find common root
			for (int x = 0; x < length; ++x)
			{
				if (string.Compare(fromDirectories[x], toDirectories[x], true) != 0)
					break;

				lastCommonRoot = x;
			}
			if (lastCommonRoot == -1)
				return toPath;

			// Add relative folders in from path
			for (int x = lastCommonRoot + 1; x < fromDirectories.Length; x++)
				if (fromDirectories[x].Length > 0)
					relativePath.Add("..");

			// Add to folders to path
			for (int x = lastCommonRoot + 1; x < toDirectories.Length; ++x)
				relativePath.Add(toDirectories[x]);

			// Create relative path
			string[] relativeParts = new string[relativePath.Count];
			relativePath.CopyTo(relativeParts, 0);

			string newPath = string.Join(outputSeparator.ToString(), relativeParts);

			if (newPath.EndsWith("."))
				newPath += outputSeparator;

			return newPath;
		}
	}
}
