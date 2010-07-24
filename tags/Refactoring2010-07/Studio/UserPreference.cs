using System;
using System.Collections.Generic;
using System.IO;
using System.Xml.Serialization;

namespace Studio
{
	public class UserPreference
	{
		public UserPreference()
		{
			RecentProjects = new List<string>();
		}

		public void OpenProject(string filePath)
		{
			RecentProjects.Remove(filePath);
			RecentProjects.Insert(0, filePath);
			if (RecentProjects.Count > 5)
				RecentProjects.RemoveAt(RecentProjects.Count - 1);
		}

		public static void SerializeToXml(string xmlPath, UserPreference preference)
		{
			XmlSerializer serializer = new XmlSerializer(typeof(UserPreference));
			TextWriter textWriter = new StreamWriter(xmlPath);
			serializer.Serialize(textWriter, preference);
			textWriter.Close();
		}

		public static UserPreference DeserializeFromXml(string xmlPath)
		{
			UserPreference ret = new UserPreference();
			try
			{
				XmlSerializer deserializer = new XmlSerializer(typeof(UserPreference));
				TextReader textReader = new StreamReader(xmlPath);
				UserPreference tmp = (UserPreference)deserializer.Deserialize(textReader);
				if (tmp != null)
					ret = tmp;
				textReader.Close();
			}
			catch (Exception e)
			{
				System.Console.WriteLine(e.Message);
			}

			return ret;
		}

		public List<string> RecentProjects;
	}
}
