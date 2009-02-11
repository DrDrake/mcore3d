using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using WeifenLuo.WinFormsUI.Docking;
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
			entityRoot = entity;

			// Fill up the tree view
			treeView.Nodes.Add(entity.treeViewNode.Nodes[0]);
			treeView.Nodes.Add(entity.treeViewNode.Nodes[1]);

			entity.treeViewNode.Nodes[1].Expand();
		}

	// Attributes
		/// <summary>
		/// The Entity system that this EntityWindow associated with.
		/// </summary>
		Entity entityRoot;

		PropertyWindow propertyWindow;
	}
}
