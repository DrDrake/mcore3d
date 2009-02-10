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

		/// <summary>
		/// Select the Entity tree to be displayed
		/// </summary>
		/// <param name="entity"></param>
		public void selectEntityRoot(Entity entity)
		{
			entityRoot = entity;

			// Fill up the tree view
			TreeNodeCollection currentNode = treeView.Nodes;
			Entity currentEntity = entity;

			do
			{
				bool noChildMove = false;
				System.Console.WriteLine(currentEntity.name);
				treeView.Nodes.Add(currentEntity.name);
				// NOTE: This while loop is copied from MCD::EntityPreorderIterator
				while (currentEntity != null)
				{
					if (currentEntity.firstChild != null && !noChildMove)
					{
						currentEntity = currentEntity.firstChild;
//						if (currentEntity != null)
//							currentNode.Add(currentEntity.name);
						break;
					}
					else if (currentEntity.nextSlibing != null)
					{
						currentEntity = currentEntity.nextSlibing;
//						if(currentEntity != null)
//							currentNode[0].Parent.Nodes.Add(currentEntity.name);
						break;
					}
					else
					{
						currentEntity = currentEntity.parent;
						noChildMove = true;

						if (currentEntity == entity)
							currentEntity = null;
					}
				}
			} while (currentEntity != null);

/*			TreeNode node;
			node = treeView.Nodes.Add("Fruits");
			node.Nodes.Add("Apple");
			node.Nodes.Add("Peach");*/
		}

		Entity entityRoot;
	}
}
