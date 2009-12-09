#pragma once

#include "Entity.h"

namespace Binding {

class RenderPanelControlImpl;
ref class ResourceManager;

public delegate void EntitySelectionChangedHandler(Object^ sender, Entity^ entity);

/// <summary>
/// Summary for RenderPanelControl
/// </summary>
///
/// WARNING: If you change the name of this class, you will need to change the
///          'Resource File Name' property for the managed resource compiler tool
///          associated with all .resx files this class depends on.  Otherwise,
///          the designers will not be able to interact properly with localized
///          resources associated with this form.
public ref class RenderPanelControl : public System::Windows::Forms::UserControl
{
public:
	/// Give sharedGlContext a valid OpenGl context so that all resources created
	/// by sharedGlContext can also be used in this RenderPanelControl. Give it
	/// null if no sharing is needed.
	RenderPanelControl(ResourceManager^ resourceManager, IntPtr sharedGlContext);

	property IntPtr glContext {
		IntPtr get();
	}

	/// Destroy the rendering context
	void destroy();

	/// Update the rendering.
	void update();

	/// Enable the internal timer for automatic rendering update.
	void enableAutoUpdate(bool flag);

	property ::Binding::Entity^ rootEntity {
		::Binding::Entity^ get();
	}

	property ::Binding::Entity^ selectedEntity {
		::Binding::Entity^ get();
		void set(::Binding::Entity^);
	}

	/// Occur when the selection (by picking) in the scene is changed.
	EntitySelectionChangedHandler^ entitySelectionChanged;

	/// When en entity is updated, refresh it's corresponding property grid.
	/// TODO: Revise and find a better solution that free the grid from RenderPanelControl.
	System::Windows::Forms::PropertyGrid^ propertyGrid;

	enum class GizmoMode
	{
		Translate, Rotate, Scale, None
	};

	property GizmoMode gizmoMode {
		GizmoMode get();
		void set(GizmoMode);
	}

	bool executeScriptFile(System::String^ scriptFilePath);

	void play(System::String^ scriptFilePath);

	void stop();

	/// Is in playing (debug) mode or not.
	property bool playing {
		bool get();
	}

	void printSerailize();

protected:
	/// <summary>
	/// Clean up any resources being used.
	/// </summary>
	~RenderPanelControl();

	// Overriding the Finalize()
	// ref: http://www.geocities.com/Jeff_Louie/deterministic_destructors.htm
	!RenderPanelControl();	// Finalize

private:
	IntPtr mSharedGlContext;
	RenderPanelControlImpl* mImpl;
	::Binding::Entity^ mRootEntity;
	::Binding::ResourceManager^ mResourceManager;

private:
	/// <summary>
	/// Required designer variable.
	/// </summary>
	System::Windows::Forms::Timer^ timer;
	System::ComponentModel::IContainer^ components;

#pragma region Windows Form Designer generated code
	/// <summary>
	/// Required method for Designer support - do not modify
	/// the contents of this method with the code editor.
	/// </summary>
	void InitializeComponent(void)
	{
		this->components = (gcnew System::ComponentModel::Container());
		this->timer = (gcnew System::Windows::Forms::Timer(this->components));
		this->SuspendLayout();
		// 
		// timer
		// 
		this->timer->Enabled = true;
		this->timer->Interval = 10;
		this->timer->Tick += gcnew System::EventHandler(this, &RenderPanelControl::timer_Tick);
		// 
		// RenderPanelControl
		// 
		this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
		this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
		this->BackColor = System::Drawing::Color::Gray;
		this->Name = L"RenderPanelControl";
		this->Size = System::Drawing::Size(150, 138);
		this->Load += gcnew System::EventHandler(this, &RenderPanelControl::RenderPanelControl_Load);
		this->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &RenderPanelControl::RenderPanelControl_Paint);
		this->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &RenderPanelControl::RenderPanelControl_KeyUp);
		this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &RenderPanelControl::RenderPanelControl_KeyDown);
		this->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &RenderPanelControl::RenderPanelControl_MouseDown);
		this->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &RenderPanelControl::RenderPanelControl_MouseUp);
		this->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &RenderPanelControl::RenderPanelControl_MouseMove);
		this->ResumeLayout(false);

	}
#pragma endregion

private: System::Void timer_Tick(System::Object^ sender, System::EventArgs^ e);
private: System::Void RenderPanelControl_Load(System::Object^ sender, System::EventArgs^ e);
private: System::Void RenderPanelControl_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e);
private: System::Void RenderPanelControl_KeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e);
private: System::Void RenderPanelControl_KeyUp(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e);
private: System::Void RenderPanelControl_MouseDown(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e);
private: System::Void RenderPanelControl_MouseUp(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e);
private: System::Void RenderPanelControl_MouseMove(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e);
};

}	// namespace Binding
