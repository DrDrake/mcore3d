using System;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace Studio
{
	public class TreeViewRearrange<T> where T : TreeView
	{
		/// <summary>
		/// Defines the relative position to a node.
		/// </summary>
		public enum Position { Up, Middle, Down, NotSet };

		#region Dragging event
		/// <summary>
		/// Indicate which node is being dragging, and over which node to drop to.
		/// </summary>
		public class DraggingArgument : CancelEventArgs
		{
			public TreeNode SourceNode, TargetNode;
			public Position Position;

			public DraggingArgument()
			{
				Color = Color.Gray;
			}

			/// <summary>
			/// Set the drag-over color of the node
			/// </summary>
			public Color Color;
		}
		public delegate void DraggingEventHandler(object sender, DraggingArgument arg);
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

		/// <summary>
		/// The tree view that this class is managing.
		/// </summary>
		public T TreeView
		{
			get { return mTreeView; }
			set
			{
				// Cleanp
				mBackupState = new BackupState();
				if (mTreeView != null)
				{
					mTreeView.DragLeave -= new System.EventHandler(this.treeView_DragLeave);
					mTreeView.DragDrop -= new System.Windows.Forms.DragEventHandler(this.treeView1_DragDrop);
					mTreeView.ItemDrag -= new System.Windows.Forms.ItemDragEventHandler(this.treeView1_ItemDrag);
					mTreeView.DragOver -= new System.Windows.Forms.DragEventHandler(this.treeView1_DragOver);
				}

				// Assign new value
				if ((mTreeView = value) != null)
				{
					mTreeView.DragLeave += new System.EventHandler(this.treeView_DragLeave);
					mTreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.treeView1_DragDrop);
					mTreeView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.treeView1_ItemDrag);
					mTreeView.DragOver += new System.Windows.Forms.DragEventHandler(this.treeView1_DragOver);
				}
			}
		}
		private T mTreeView;

		/// <summary>
		/// The time delay of expanding a node during drag over.
		/// </summary>
		public int ExpandNodeDelay
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
			mTreeView.DoDragDrop(e.Item.ToString(), DragDropEffects.Move | DragDropEffects.Copy);
		}

		private void treeView1_DragOver(object sender, DragEventArgs e)
		{
			// Determine the node we are dragging over
			Point p = mTreeView.PointToClient(new Point(e.X, e.Y));
			TreeNode n = mTreeView.GetNodeAt(p);

			if ((mBackupState.TargetNode = n) == null)
				return;

			{	// Handle auto scroll
				// Reference: http://www.syncfusion.com/faq/windowsforms/faq_c91c.aspx
				int delta = mTreeView.Height - p.Y;
				if ((delta < mTreeView.Height / 2) && (delta > 0))
				{
					if (n.NextVisibleNode != null)
						n.NextVisibleNode.EnsureVisible();
				}
				if ((delta > mTreeView.Height / 2) && (delta < mTreeView.Height))
				{
					if (n.PrevVisibleNode != null)
						n.PrevVisibleNode.EnsureVisible();
				}
			}

			// Invoke the callback to see the user allow dropping or not
			e.Effect = DragDropEffects.Move;
			DraggingArgument arg = new DraggingArgument();
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
					mTreeView.Refresh();
					DrawLeafPlaceholders(n, n.Bounds.Top, arg.Color);
				}
			}
			else if (yOffset > n.Bounds.Height * 3 / 4)	// Drag over bottom of a node
			{
				if (mBackupState.SetPositionIfNot(Position.Down))
				{
					mTreeView.Refresh();
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
			mTreeView.Refresh();
		}

		private void treeView_DragLeave(object sender, EventArgs e)
		{
			mBackupState.TargetNode = null;
			mTreeView.Refresh();
		}

		private void DrawLeafPlaceholders(TreeNode NodeOver, int verticalPos, Color color)
		{
			// NOTE: The graphics should be created every time, cannot cache it
			Graphics g = mTreeView.CreateGraphics();

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
	}
}
