using Aga.Controls.Tree;

namespace Studio
{
	public class TreeNodeBase<T> : Node where T : Node
	{
		/// <summary>
		/// The container which containing this node
		/// </summary>
		public TreeNodeList<T> ParentContainer;

		/// <summary>
		/// Get the tree view that containing this node
		/// </summary>
		public TreeViewAdv TreeView
		{
			get
			{
				return ParentContainer.TreeView;
			}
		}

		/// <summary>
		/// Get the visual tree node from this model node.
		/// </summary>
		public TreeNodeAdv TreeNode
		{
			get
			{
				return TreeView.FindNodeByTag(this);
			}
		}
	}

	public class TreeNodeList<T> : System.Collections.Generic.List<T> where T : Node
	{
		public TreeNodeList(TreeViewAdv treeView, Node node)
		{
			TreeView = treeView;
			Node = node;
		}

		/// <summary>
		/// The change handler when something is added or remove form the list
		/// </summary>
		/// <param name="obj">The object being add or remove</param>
		/// <returns>True to allow the change to take place</returns>
		public delegate bool ChangeHandler(T obj);
		public event ChangeHandler OnAdd;
		public event ChangeHandler OnRemove;

		/// <summary>
		/// Intercept List.Add() to fire event
		/// </summary>
		public virtual new void Add(T obj)
		{
			if (OnAdd != null)
				OnAdd(obj);
			(obj as TreeNodeBase<T>).ParentContainer = this;
			Node.Nodes.Add(obj);
			base.Add(obj);
		}

		public virtual new void Remove(T obj)
		{
			if (OnRemove != null)
				OnRemove(obj);
			(obj as TreeNodeBase<T>).ParentContainer = null;
			obj.Parent = null;
			base.Remove(obj);
		}

		public virtual new void Clear()
		{
			while (base.Count > 0)
				this.Remove(base[0]);
		}

		public TreeViewAdv TreeView;
		public Node Node;
	}	// TreeNodeList<T>
}
