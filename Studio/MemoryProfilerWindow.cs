﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Net.Sockets;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using Aga.Controls.Tree;

namespace Studio
{
	public partial class MemoryProfilerWindow : DockContent
	{
		public MemoryProfilerWindow()
		{
			InitializeComponent();

			// Use a background worker to pull report data from the server
			// Reference: http://msdn.microsoft.com/en-ca/library/waw3xexc.aspx
			backgroundWorker1.DoWork += new DoWorkEventHandler(backgroundWorker1_DoWork);
			backgroundWorker1.RunWorkerCompleted += new RunWorkerCompletedEventHandler(backgroundWorker1_RunWorkerCompleted);

			mStringList = new List<string>();

			treeViewAdv1.Model = new TreeModel();
			mMemoryProfilerServer = new Binding.MemoryProfilerServer();
			mMemoryProfilerServer.listern(5000);

			// All setup finished, we can start the timer.
			timer1.Enabled = true;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="connectionString">Format: "hostname:port"</param>
		private void connect(string connectionString)
		{
			try
			{
				if (mClient != null)
				{
					disconnect();
					return;
				}

				string[] token = connectionString.Split(':');

				mClient = new System.Net.Sockets.TcpClient(token[0], Int32.Parse(token[1]));
				mStreamReader = new StreamReader(mClient.GetStream());

				textBox1.Enabled = false;
				button1.Text = "Disconnect";
				button2.Enabled = true;
			}
			catch (Exception ex)
			{
				MessageBox.Show(ex.ToString());
				mClient = null;
			}
		}

		private void disconnect()
		{
			mDisconnectPending = true;
			backgroundWorker1.CancelAsync();
		}

		private void afterDisconnect()
		{
			mDisconnectPending = false;
			TreeModel model = treeViewAdv1.Model as TreeModel;
			mClient.Close();
			mClient = null;
			textBox1.Enabled = true;
			button1.Text = "Connect";

			if (mPasued)
				button2_Click(this, new EventArgs());
			button2.Enabled = false;

			model.Nodes.Clear();
			mRootNode = null;
		}

		private TcpClient mClient;
		private bool mDisconnectPending = false;
		private bool mPasued = false;
		private StreamReader mStreamReader;

		/// <summary>
		/// This string should only modified by worker thread
		/// </summary>
		private string mReportString;
		private List<string> mStringList;
		private MemCallstackNode mRootNode;

		private Binding.MemoryProfilerServer mMemoryProfilerServer;

		/// <summary>
		/// If I can call RunWorkerAsync() in RunWorkerCompleted(), I do not need this timer
		/// </summary>
		private void timer1_Tick(object sender, EventArgs e)
		{
			mMemoryProfilerServer.flush();

			if (backgroundWorker1.IsBusy)
				return;

			if (mClient != null && mClient.Connected && !mDisconnectPending)
				backgroundWorker1.RunWorkerAsync();

			if (mDisconnectPending)
				afterDisconnect();
		}

		private void backgroundWorker1_DoWork(object sender, DoWorkEventArgs e)
		{
			try
			{
				// Split on ";".
				// If you want to support function name has ";", a more robust regular expression is needed.
				Regex regex = new Regex(@";");

				mStringList.Clear();
				mReportString = "";
				string lastLine = "";

				if (mRootNode == null)
					mRootNode = new MemCallstackNode();

				MemCallstackNode node = null;
				MemCallstackNode previousNode = null;	// Back up the node of last iteration, for detecting dead nodes.

				// Parse the incomming string message and build a corresponding callstack tree.
				while (true)
				{
					if (backgroundWorker1.CancellationPending)
						break;

					string s = mStreamReader.ReadLine();
					// We use double new line as a terminate indicator
					if (s == lastLine && s == "")
						break;

					if (mPasued)
						continue;

					if (s.Length > 0)
					{
						string[] tokens = regex.Split(s);
						int level = Int32.Parse(tokens[0]);

						string id = tokens[1];

						if (node == null)
							node = mRootNode;
						else
						{
							previousNode = node;

							// Up the callstack
							for (int j = node.Level - level; j > -1; )
							{
								// NOTE: node.Level - node.Parent.Level may not simply equals to 1
								j -= (node.Level - node.Parent.Level);

								node = node.Parent as MemCallstackNode;
							}

							// Search for existing node with the same name
							MemCallstackNode searchNode = null;
							foreach (MemCallstackNode n in node.Nodes)
							{
								if (n.Id == id)
								{
									searchNode = n;
									break;
								}
							}

							// Create a new node if none has found
							if (searchNode == null)
							{
								searchNode = new MemCallstackNode();
								searchNode.Parent = node;
							}

							node = searchNode;
						}

						// Remove any "dead" node
						if (previousNode != null)
						{
							while (previousNode.MyNextNode != null && previousNode.MyNextNode != node)
								previousNode.MyNextNode.Parent = null;
						}

						node.MyPreviousNode = previousNode;
						node.Level = level;
						node.Id = tokens[1];
						node.Name = tokens[2];
						node.TCount = Int32.TryParse(tokens[3], out node.TCount) ? node.TCount : -1;
						node.SCount = Int32.TryParse(tokens[4], out node.SCount) ? node.SCount : -1;
						node.TkBytes = Double.Parse(tokens[5]);
						node.SkBytes = Double.Parse(tokens[6]);
						node.SCountPerFrame = Double.TryParse(tokens[7], out node.SCountPerFrame) ? node.SCountPerFrame : -1;
						node.CallPerFrame = Double.TryParse(tokens[8], out node.CallPerFrame) ? node.CallPerFrame : -1;
					}

					lastLine = s;
					mStringList.Add(s);
					mReportString += s;
				}	// while(true)

				// Remove any "dead" node
				{
					while (node != null && node.MyNextNode != null)
						node.MyNextNode.Parent = null;
				}
			}
			catch (Exception ex)
			{
				MessageBox.Show(ex.ToString());
				disconnect();
			}
		}

		private void backgroundWorker1_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
		{
			TreeModel model = treeViewAdv1.Model as TreeModel;
			if (mRootNode != null)
				model.Nodes.Add(mRootNode);
			if (mDisconnectPending)
				afterDisconnect();
			treeViewAdv1.Refresh();
		}

		private void button1_Click(object sender, EventArgs e)
		{
			connect(textBox1.Text);
		}

		private void button2_Click(object sender, EventArgs e)
		{
			mPasued = !mPasued;
			button2.Text = mPasued ? "Resume" : "Pause";
		}
	}

	class MemCallstackNode : Aga.Controls.Tree.Node
	{
		public int Level;
		public string Id;
		public string Name;
		public int TCount;
		public int SCount;
		public double TkBytes;
		public double SkBytes;
		public double SCountPerFrame;
		public double CallPerFrame;

		/// <summary>
		/// Rounded TkBytes to the nearest integer, and display as "<1" if TkBytes is less than 1 but not zero
		/// </summary>
		public string TkBytesAsString
		{
			get
			{
				return (TkBytes < 1 && TkBytes > 0) ? "< 1" : Math.Round(TkBytes).ToString();
			}
		}

		/// <summary>
		/// Rounded SkBytes to the nearest integer, and display as "<1" if SkBytes is less than 1 but not zero
		/// </summary>
		public string SkBytesAsString
		{
			get
			{
				return (SkBytes < 1 && SkBytes > 0) ? "< 1" : Math.Round(SkBytes).ToString();
			}
		}

		public new MemCallstackNode Parent
		{
			get
			{
				return base.Parent as MemCallstackNode;
			}

			set
			{
				if (value == null)	// Detaching this node from it's current parent
				{
					if (mMyPrevious != null)
						mMyPrevious.mMyNext = mMyNext;
					if (mMyNext != null)
						mMyNext.mMyPrevious = mMyPrevious;
				}
				base.Parent = value;
			}
		}

		public MemCallstackNode MyPreviousNode
		{
			get
			{
				return mMyPrevious;
			}

			set
			{
				if (value != null)
					value.mMyNext = this;
				mMyPrevious = value;
			}
		}

		public MemCallstackNode MyNextNode
		{
			get
			{
				return mMyNext;
			}
		}

		// These two nodes mantains the linear structure of the comming TCP string protocol.
		private MemCallstackNode mMyPrevious;
		private MemCallstackNode mMyNext;
	}
}
