using CodersLab.Windows.Controls;
using System;
using System.Drawing;
using System.Windows.Forms;
using global::Binding;

namespace Studio
{
	/// <summary>
	/// Showing the Entity tree with a multi-selectable tree-view control
	/// Download URL of the multi-selectable tree-view:
	/// http://sourceforge.net/projects/mstreeview/
	/// Some reference on using tree-view:
	/// http://www.java2s.com/Code/CSharp/GUI-Windows-Form/TreeViewExample.htm
	/// </summary>
	public partial class EntityWindow : DockContent
	{
		public EntityWindow()
		{
			InitializeComponent();

			// Instantiate delegate with anonymous method
			entitySelectionChanged = delegate(object sender, Entity entity)
			{
				if (entity == selectedEntity)
					return;

				treeView.SelectedNodes.Clear();

				if(entity != null)
					treeView.SelectedNodes.Add(entity.treeViewNode);
			};
		}

	// Operations
		/// <summary>
		/// Select the Entity tree to be displayed
		/// </summary>
		/// <param name="entity"></param>
		public void selectEntityRoot(Entity entity)
		{
			entityRoot = entity;

			treeView.Nodes.Clear();

			if (entity != null)
			{
				// Fill up the tree view
				treeView.Nodes.Add(entity.treeViewNode.Nodes[0]);
				treeView.Nodes.Add(entity.treeViewNode.Nodes[1]);
                treeView.Nodes.Add(entity.treeViewNode.Nodes[2]);

				entity.treeViewNode.Nodes[2].Expand();
			}
		}

	// Attributes
		/// <summary>
		/// The Entity system that this EntityWindow associated with.
		/// </summary>
		Entity entityRoot;

		public Entity selectedEntity
		{
			get
			{
				if (treeView.SelectedNodes.Count == 0)
					return null;
				return (Entity)treeView.SelectedNodes[0].Tag;
			}
		}

		/// <summary>
		/// Occur when the selection in the entity tree view is changed.
		/// </summary>
		public EntitySelectionChangedHandler entitySelectionChanged;

	// Events
		private void treeView_SelectionsChanged(object sender, EventArgs e)
		{
			entitySelectionChanged(this, selectedEntity);

			// Ensure the tree view node is visible
			// TODO: Enable as an option
			if(selectedEntity != null)
				selectedEntity.treeViewNode.EnsureVisible();
		}

		/// Referende: http://www.c-sharpcorner.com/UploadFile/mgold/DoingDragandDropUsingCSharp11302005020610AM/DoingDragandDropUsingCSharp.aspx
		#region Drag drop related
		private TreeNode mSourceNode;
		private void treeView_ItemDrag(object sender, System.Windows.Forms.ItemDragEventArgs e)
		{
			NodesCollection nodes = (NodesCollection)e.Item;
			mSourceNode = nodes[0];
			DoDragDrop(e.Item.ToString(), DragDropEffects.Move | DragDropEffects.Copy);
		}

		private void treeView_DragEnter(object sender, DragEventArgs e)
		{
			if (e.Data.GetDataPresent(DataFormats.Text))
				e.Effect = DragDropEffects.Move;
			else
				e.Effect = DragDropEffects.None;
		}

		private TreeNode mDragOldNode;
		private Color mDragOldColor;
		private void treeView_DragOver(object sender, DragEventArgs e)
		{
			// Determine the node we are dragging over
			Point p = treeView.PointToClient(new Point(e.X, e.Y));
			TreeNode n = treeView.GetNodeAt(p);
//			treeView.HotTracking = true;

			Object o = e.Data.GetData("System.Windows.Forms.TreeNode");
			if (o != null)
				o = o;
			TreeNode NodeMoving = (TreeNode)e.Data.GetData("System.Windows.Forms.TreeNode");

			if (n != null && n != mDragOldNode)
			{
				n.EnsureVisible();
				if(mDragOldNode != null)
					mDragOldNode.BackColor = mDragOldColor;
				mDragOldNode = n;
				mDragOldColor = n.BackColor;

				if (!treeView.SelectedNodes.Contains(n))
//					n.Checked = true;
					n.BackColor = Color.Gray;

//				e.Effect = DragDropEffects.Scroll;
			}
		}
		#endregion

		private void treeView_DragDrop(object sender, DragEventArgs e)
		{
			// Determine the node we are drop at
			Point p = treeView.PointToClient(new Point(e.X, e.Y));
			TreeNode n = treeView.GetNodeAt(p);
			n.BackColor = mDragOldColor;
			mDragOldNode = null;
		}
	}
}
