using System;
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

				entity.treeViewNode.Nodes[1].Expand();
			}
		}

	// Attributes
		/// <summary>
		/// The Entity system that this EntityWindow associated with.
		/// </summary>
		Entity entityRoot;

		public PropertyWindow propertyWindow;

		public Entity selectedEntity
		{
			get
			{
				if (treeView.SelectedNodes.Count == 0)
					return null;
				return (Entity)treeView.SelectedNodes[0].Tag;
			}
		}

	// Events
		private void treeView_SelectionsChanged(object sender, EventArgs e)
		{
			propertyWindow.propertyGrid1.SelectedObject = selectedEntity;
		}
	}
}
