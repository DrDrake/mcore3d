using System;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;
using System.Diagnostics;

namespace Studio
{
	public partial class RearrangeableTreeView : UserControl
	{
		public RearrangeableTreeView()
		{
			InitializeComponent();
			mBackupState = new BackupState();
		}

		/// <summary>
		/// Defines the relative position to a node.
		/// </summary>
		public enum Position { Up, Middle, Down, NotSet };

		#region Draggin event
		/// <summary>
		/// Indicate which node is being dragging, and over which node to drop to.
		/// </summary>
		public class DragginArgument : CancelEventArgs
		{
			public TreeNode SourceNode, TargetNode;
			public Position Position;

			public DragginArgument()
			{
				Color = Color.Gray;
			}

			/// <summary>
			/// Set the drag-over color of the node
			/// </summary>
			public Color Color;
		}
		public delegate void DraggingEventHandler(object sender, DragginArgument arg);
		public event DraggingEventHandler Dragging;
		#endregion

		#region Drop event
		public class DropArgument
		{
			public TreeNode SourceNode, TargetNode;
			public Position Position;
		}
		public delegate void DropEventHandler(object sender, DropArgument arg);
		public event DropEventHandler Drop;
		#endregion

		int ExpandNodeDelay
		{
			get { return mBackupState.expandNodeDelay; }
			set { mBackupState.expandNodeDelay = value; }
		}

		private class BackupState
		{
			DropArgument dropArgument;
			Color oldColor;
			Timer expandNodeTimer;
			public int expandNodeDelay;

			public BackupState()
			{
				dropArgument = new DropArgument();
				expandNodeTimer = new Timer();
				expandNodeTimer.Tick += new EventHandler(expandNodeTimer_Tick);
				expandNodeDelay = 500;
			}

			void expandNodeTimer_Tick(object sender, EventArgs e)
			{
				if(TargetNode != null)
					TargetNode.Expand();
			}

			public TreeNode SourceNode
			{
				get { return dropArgument.SourceNode; }
				set
				{
					dropArgument.SourceNode = value;
					TargetNode = null;
					dropArgument.Position = Position.NotSet;
				}
			}

			public TreeNode TargetNode
			{
				get { return dropArgument.TargetNode; }
				set
				{	// Lazy update
					if (dropArgument.TargetNode == value)
						return;

					expandNodeTimer.Stop();

					if(dropArgument.TargetNode != null)
						dropArgument.TargetNode.BackColor = oldColor;
					if (value != null)
						oldColor = value.BackColor;

					dropArgument.TargetNode = value;
				}
			}

			public Position Position
			{
				get { return dropArgument.Position; }
			}

			public bool SetPositionIfNot(Position pos)
			{
				if (dropArgument.Position != pos)
				{
					// Whenever the user drag right over a node, start a timer count down
					// for node expansion.
					if (pos == Position.Middle)
					{
						expandNodeTimer.Interval = expandNodeDelay;
						expandNodeTimer.Start();
					}

					if (dropArgument.TargetNode != null)
						dropArgument.TargetNode.BackColor = oldColor;
					dropArgument.Position = pos;
					return true;
				}
				else
					return false;
			}
		}
		BackupState mBackupState;

		private void treeView1_ItemDrag(object sender, ItemDragEventArgs e)
		{
			mBackupState.SourceNode = ((CodersLab.Windows.Controls.NodesCollection)e.Item)[0];
			DoDragDrop(e.Item.ToString(), DragDropEffects.Move | DragDropEffects.Copy);
		}

		private void treeView1_DragOver(object sender, DragEventArgs e)
		{
			// Determine the node we are dragging over
			Point p = TreeView.PointToClient(new Point(e.X, e.Y));
			TreeNode n = TreeView.GetNodeAt(p);

			if ((mBackupState.TargetNode = n) == null)
				return;

			{	// Handle auto scroll
				// Reference: http://www.syncfusion.com/faq/windowsforms/faq_c91c.aspx
				int delta = TreeView.Height - p.Y;
				if ((delta < TreeView.Height / 2) && (delta > 0))
				{
					if (n.NextVisibleNode != null)
						n.NextVisibleNode.EnsureVisible();
				}
				if ((delta > TreeView.Height / 2) && (delta < TreeView.Height))
				{
					if (n.PrevVisibleNode != null)
						n.PrevVisibleNode.EnsureVisible();
				}
			}

			// Invoke the callback to see the user allow dropping or not
			e.Effect = DragDropEffects.Move;
			DragginArgument arg = new DragginArgument();
			if (Dragging != null && mBackupState.SourceNode != null)
			{
				arg.SourceNode = mBackupState.SourceNode;
				arg.TargetNode = mBackupState.TargetNode;
				arg.Position = mBackupState.Position;
				Dragging(this, arg);
				if (arg.Cancel)
					e.Effect = DragDropEffects.None;
			}

			int yOffset = p.Y - n.Bounds.Top;
			if (yOffset < n.Bounds.Height / 4)	// Drag over top of a node
			{
				if (mBackupState.SetPositionIfNot(Position.Up))
				{
					TreeView.Refresh();
					DrawLeafPlaceholders(n, n.Bounds.Top, arg.Color);
				}
			}
			else if (yOffset > n.Bounds.Height * 3 / 4)	// Drag over bottom of a node
			{
				if (mBackupState.SetPositionIfNot(Position.Down))
				{
					TreeView.Refresh();
					DrawLeafPlaceholders(n, n.Bounds.Bottom, arg.Color);
				}
			}
			else if (mBackupState.SetPositionIfNot(Position.Middle))	// Darg right over a node
			{
				n.EnsureVisible();
				n.BackColor = arg.Color;
			}
		}

		private void treeView1_DragDrop(object sender, DragEventArgs e)
		{
			DropArgument arg = new DropArgument();
			arg.SourceNode = mBackupState.SourceNode;
			arg.TargetNode = mBackupState.TargetNode;
			arg.Position = mBackupState.Position;
			if(Drop != null)
				Drop(this, arg);
			mBackupState.SourceNode = null;
			TreeView.Refresh();
		}

		private void treeView_MouseDown(object sender, MouseEventArgs e)
		{
			TreeView.SelectedNodes.Clear();
			TreeView.SelectedNodes.Add(TreeView.GetNodeAt(e.X, e.Y));
		}

		private void treeView_DragLeave(object sender, EventArgs e)
		{
			mBackupState.TargetNode = null;
			TreeView.Refresh();
		}

		private void DrawLeafPlaceholders(TreeNode NodeOver, int verticalPos, Color color)
		{
			// NOTE: The graphics should be created every time, cannot cache it
			Graphics g = TreeView.CreateGraphics();

			int LeftPos = NodeOver.Bounds.Left - 10;
			int RightPos = NodeOver.Bounds.Right + 10;

			Point[] LeftTriangle = new Point[3] {
				new Point(LeftPos, verticalPos - 5),
				new Point(LeftPos, verticalPos + 4),
				new Point(LeftPos + 4, verticalPos - 1)
			};

			Point[] RightTriangle = new Point[3] {
				new Point(RightPos, verticalPos - 5),
				new Point(RightPos, verticalPos + 4),
				new Point(RightPos - 4, verticalPos - 1)
			};

			g.FillPolygon(new SolidBrush(color), LeftTriangle);
			g.FillPolygon(new SolidBrush(color), RightTriangle);
			g.DrawLine(new Pen(color, 2), new Point(LeftPos, verticalPos), new Point(RightPos, verticalPos));
		}

		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary> 
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.TreeView = new CodersLab.Windows.Controls.TreeView();
			this.SuspendLayout();
			// 
			// TreeView
			// 
			this.TreeView.AllowDrop = true;
			this.TreeView.Dock = System.Windows.Forms.DockStyle.Fill;
			this.TreeView.Location = new System.Drawing.Point(0, 0);
			this.TreeView.Name = "TreeView";
			this.TreeView.ShowNodeToolTips = true;
			this.TreeView.Size = new System.Drawing.Size(150, 150);
			this.TreeView.TabIndex = 0;
			this.TreeView.DragLeave += new System.EventHandler(this.treeView_DragLeave);
			this.TreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.treeView1_DragDrop);
			this.TreeView.MouseDown += new System.Windows.Forms.MouseEventHandler(this.treeView_MouseDown);
			this.TreeView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.treeView1_ItemDrag);
			this.TreeView.DragOver += new System.Windows.Forms.DragEventHandler(this.treeView1_DragOver);
			// 
			// MyTreeView
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.TreeView);
			this.Name = "MyTreeView";
			this.ResumeLayout(false);

		}

		#endregion

		public CodersLab.Windows.Controls.TreeView TreeView;
	}
}
