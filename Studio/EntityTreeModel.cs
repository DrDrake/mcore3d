using System;
using System.Collections.Generic;
using Aga.Controls.Tree;
using Binding;

#pragma warning disable 67	// Event never used

namespace Studio
{
	/// <summary>
	/// See TreeViewAdv's FolderBrowserModel.cs on how to make a ITreeModel
	/// </summary>
	class EntityTreeModel : ITreeModel
	{
		private Entity _root;
		private System.Windows.Forms.Timer _timer;
		private EntityPreorderIterator _updateIterator;	// For preforming incremental Entity tree update

		public EntityTreeModel(Entity rootEntity)
		{
			_root = rootEntity;
			_timer = new System.Windows.Forms.Timer();
			_timer.Enabled = true;
			_timer.Interval = 200;
			_timer.Tick += new EventHandler(_timer_Tick);
		}

		TreePath ToTreePath(Entity e)
		{
			List<Entity> items = new List<Entity>();
			while(e != null && e != _root)
			{
				items.Add(e);
				e = e.parent;
			}
			items.Add(_root);
			items.Reverse();

			return new TreePath(items.ToArray());
		}

		/// <summary>
		/// Periodically poll for stuctural changes in the Entity tree.
		/// </summary>
		void _timer_Tick(object sender, EventArgs e)
		{
			if (StructureChanged == null)
				return;

			if (_updateIterator == null || _updateIterator.ended())
				_updateIterator = new EntityPreorderIterator(_root);

			for (int i = 0; i < _timer.Interval/2 && !_updateIterator.ended(); ++i, _updateIterator.next())
			{
				if (!_updateIterator.current.isChildrenDirty)
					continue;
				StructureChanged(this, new TreePathEventArgs(ToTreePath(_updateIterator.current)));
				_updateIterator.current.isChildrenDirty = false;
			}
		}

		#region ITreeModel Members

		System.Collections.IEnumerable ITreeModel.GetChildren(TreePath treePath)
		{
			Stack<Entity> items = null;

			if (treePath.IsEmpty())
			{
				items = new Stack<Entity>();
				items.Push(_root);
			}
			else
			{
				Entity parent = treePath.LastNode as Entity;
				if (parent != null)
				{
					items = new Stack<Entity>();
					Entity e = parent.firstChild;

					while (e != null)
					{
						items.Push(e);
						e = e.nextSibling;
					}
				}
			}

			return items;
		}

		bool ITreeModel.IsLeaf(TreePath treePath)
		{
			Entity e = treePath.LastNode as Entity;
			return !(e != null && e.firstChild != null);
		}

		public event EventHandler<TreeModelEventArgs> NodesChanged;
		public event EventHandler<TreeModelEventArgs> NodesInserted;
		public event EventHandler<TreeModelEventArgs> NodesRemoved;
		public event EventHandler<TreePathEventArgs> StructureChanged;

		#endregion
	}
}
