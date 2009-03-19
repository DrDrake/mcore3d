﻿using CodersLab.Windows.Controls;
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

				treeView.TreeView.SelectedNodes.Clear();

				if(entity != null)
					treeView.TreeView.SelectedNodes.Add(entity.treeViewNode);
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

			treeView.TreeView.Nodes.Clear();

			if (entity != null)
			{
				// Fill up the tree view
				treeView.TreeView.Nodes.Add(entity.treeViewNode.Nodes[0]);
				treeView.TreeView.Nodes.Add(entity.treeViewNode.Nodes[1]);
				treeView.TreeView.Nodes.Add(entity.treeViewNode.Nodes[2]);

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
				if (treeView.TreeView.SelectedNodes.Count == 0)
					return null;
				return (Entity)treeView.TreeView.SelectedNodes[0].Tag;
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
	}
}
