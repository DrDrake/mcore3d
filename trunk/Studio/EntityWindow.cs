using System;
using System.Drawing;
using System.Windows.Forms;
using Aga.Controls.Tree;
using global::Binding;

namespace Studio
{
	public partial class EntityWindow : DockContent
	{
		public EntityWindow()
		{
			InitializeComponent();
		}

	// Operations
		/// <summary>
		/// Select the Entity tree to be displayed
		/// </summary>
		/// <param name="entity"></param>
		public void selectEntityRoot(Entity entity)
		{
			if (entity == null)
				return;

			entityRoot = entity;
			treeView.Model = new EntityTreeModel(entity);
			treeView.FindNodeByTag(entity).Expand();
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
			set
			{
				if (value == selectedEntity)
					return;

				if (value != null)
					treeView.SelectedNode = treeView.FindNodeByTag(value);
			}
		}

		/// <summary>
		/// To notify external object that the entity selection is changed.
		/// </summary>
		public EntitySelectionChangedHandler entitySelectionChanged;

	// Events
		private void treeView_SelectionChanged(object sender, EventArgs e)
		{
			entitySelectionChanged(this, selectedEntity);
		}

		private void treeView_KeyUp(object sender, System.Windows.Forms.KeyEventArgs e)
		{
			// Press delete key will delete the selected node from the scene graph
			if (e.KeyCode == Keys.Delete && treeView.SelectedNodes.Count > 0)
			{
				entitySelectionChanged(this, null);

				foreach (TreeNodeAdv n in treeView.SelectedNodes)
				{
					Entity en = (n.Tag as Entity);
					en.destroyThis();
				}
			}
		}

		private void treeView_ItemDrag(object sender, ItemDragEventArgs e)
		{
			treeView.DoDragDropSelectedNodes(DragDropEffects.Move | DragDropEffects.Copy);
		}

		private void treeView_DragOver(object sender, DragEventArgs e)
		{
			if ((e.KeyState & 8) == 8)	// Ctrl key pressed
				e.Effect = DragDropEffects.Copy;
			else
				e.Effect = DragDropEffects.Move;
		}

		/// <summary>
		/// Reference on how to do drap drop:
		/// TreeViewAdv example's SimpleExample.cs
		/// </summary>
		private void treeView_DragDrop(object sender, DragEventArgs e)
		{
			treeView.BeginUpdate();

			TreeNodeAdv[] nodes = (TreeNodeAdv[])e.Data.GetData(typeof(TreeNodeAdv[]));
			Entity dropEntity = treeView.DropPosition.Node.Tag as Entity;

			if (treeView.DropPosition.Position == NodePosition.Inside)
			{
				foreach (TreeNodeAdv n in nodes)
				{
					Entity en = (n.Tag as Entity);

					if ((e.KeyState & 8) == 8)	// Ctrl key pressed
						en = en.clone();

					en.asChildOf(dropEntity);
				}
				treeView.DropPosition.Node.IsExpanded = true;
			}
			else
			{
				foreach (TreeNodeAdv n in nodes)
				{
					System.Diagnostics.Debug.Assert(dropEntity != null);
					Entity en = (n.Tag as Entity);

					if ((e.KeyState & 8) == 8)	// Ctrl key pressed
						en = en.clone();

					if (treeView.DropPosition.Position == NodePosition.Before)
					{
						en.insertAfter(dropEntity);
					}
					else if (treeView.DropPosition.Position == NodePosition.After)
					{
						en.insertBefore(dropEntity);
						dropEntity = en;
					}
				}
			}

			treeView.EndUpdate();
		}
	}
}
