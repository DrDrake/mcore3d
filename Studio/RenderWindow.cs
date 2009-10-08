using System;
using System.Windows.Forms;
using Binding;

namespace Studio
{
	public partial class RenderWindow : DockContent
	{
		public RenderWindow()
		{
			InitializeComponent();
		}

		public RenderPanelControl Init(IntPtr sharedGlContext, Scene scene)
		{
			mScene = scene;

			mRenderControl = new RenderPanelControl(ProjectWindow.Singleton.Project.ResourceManager, sharedGlContext);
			mRenderControl.propertyGrid = MainForm.Singleton.propertyWindow.propertyGrid1;
			this.Tag = mRenderControl;
			mRenderControl.Tag = this;

			mRenderControl.Dock = DockStyle.Fill;
			this.panel1.Controls.Add(mRenderControl);
			this.TabText = scene.Text;

			// Broadcasting the key event from render panel to 
			mRenderControl.KeyPress += new KeyPressEventHandler(RenderWindow_KeyPress);

			return mRenderControl;
		}

		public void RefreshButtonState()
		{
			if (mRenderControl.gizmoMode == RenderPanelControl.GizmoMode.Rotate)
				toolStripButtonRotate.PerformClick();
			if (mRenderControl.gizmoMode == RenderPanelControl.GizmoMode.Scale)
				toolStripButtonScale.PerformClick();
			if (mRenderControl.gizmoMode == RenderPanelControl.GizmoMode.Translate)
				toolStripButtonTranslate.PerformClick();
		}

		private void ClearGizmoButtonsState()
		{
			toolStripButtonRotate.Checked = false;
			toolStripButtonScale.Checked = false;
			toolStripButtonTranslate.Checked = false;
		}

		public Scene Scene
		{
			get { return mScene; }
		}
		private Scene mScene;

		private RenderPanelControl mRenderControl;

		private void toolStripButtonMove_Click(object sender, EventArgs e)
		{

		}

		private void toolStripButtonTranslate_Click(object sender, EventArgs e)
		{
			ClearGizmoButtonsState();
			toolStripButtonTranslate.Checked = true;
			if (mRenderControl != null)
				mRenderControl.gizmoMode = RenderPanelControl.GizmoMode.Translate;
		}

		private void toolStripButtonRotate_Click(object sender, EventArgs e)
		{
			ClearGizmoButtonsState();
			toolStripButtonRotate.Checked = true;
			if (mRenderControl != null)
				mRenderControl.gizmoMode = RenderPanelControl.GizmoMode.Rotate;
		}

		private void toolStripButtonScale_Click(object sender, EventArgs e)
		{
			ClearGizmoButtonsState();
			toolStripButtonScale.Checked = true;
			if (mRenderControl != null)
				mRenderControl.gizmoMode = RenderPanelControl.GizmoMode.Scale;
		}

		private void RenderWindow_KeyPress(object sender, KeyPressEventArgs e)
		{
			switch (e.KeyChar)
			{
				case '1':
					toolStripButtonTranslate.PerformClick();
					break;
				case '2':
					toolStripButtonRotate.PerformClick();
					break;
				case '3':
					toolStripButtonScale.PerformClick();
					break;
			}
		}

		private void RenderWindow_Enter(object sender, EventArgs e)
		{
			// Forward the enter event to the render control
			if(mRenderControl != null)
				mRenderControl.Focus();
		}
	}
}
