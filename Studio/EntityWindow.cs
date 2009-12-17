using System;
using System.Windows.Forms;
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
			if (e.KeyCode == Keys.Delete && selectedEntity != null)
			{
				Entity entity = selectedEntity;
				entitySelectionChanged(this, null);
				entity.destroyThis();
			}
		}
	}
}
