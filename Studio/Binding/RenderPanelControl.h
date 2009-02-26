#pragma once

#include "Entity.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace Binding {

class RenderPanelControlImpl;

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
	RenderPanelControl(void);

	/// Destroy the rendering context
	void destroy();

	/// Update the rendering.
	void update();

	/// Enable the internal timer for automatic rendering update.
	void enableAutoUpdate(bool flag);

	property ::Binding::Entity^ rootEntity {
		::Binding::Entity^ get();
	}

	/// Occur when the selection (by picking) in the scene is changed.
	EntitySelectionChangedHandler^ entitySelectionChanged;

protected:
	/// <summary>
	/// Clean up any resources being used.
	/// </summary>
	~RenderPanelControl();

	// Overriding the Finalize()
	// ref: http://www.geocities.com/Jeff_Louie/deterministic_destructors.htm
	!RenderPanelControl();	// Finalize

	void onEntitySelectionChanged(Object^ sender, Entity^ entity);

private:
	RenderPanelControlImpl* mImpl;
	::Binding::Entity^ mRootEntity;

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
		this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
		this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
		this->BackColor = System::Drawing::Color::Gray;
		this->Name = L"RenderPanelControl";
		this->Load += gcnew System::EventHandler(this, &RenderPanelControl::RenderPanelControl_Load);
		this->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &RenderPanelControl::RenderPanelControl_Paint);
		this->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &RenderPanelControl::RenderPanelControl_KeyUp);
		this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &RenderPanelControl::RenderPanelControl_KeyDown);
		this->ResumeLayout(false);

	}
#pragma endregion

private: System::Void timer_Tick(System::Object^ sender, System::EventArgs^ e);
private: System::Void RenderPanelControl_Load(System::Object^ sender, System::EventArgs^ e);
private: System::Void RenderPanelControl_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e);
private: System::Void RenderPanelControl_KeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e);
private: System::Void RenderPanelControl_KeyUp(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e);
};

}	// namespace Binding
