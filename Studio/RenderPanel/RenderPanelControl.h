#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace RenderPanel {

class RenderPanelControlImpl;

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

protected:
	/// <summary>
	/// Clean up any resources being used.
	/// </summary>
	~RenderPanelControl();

	// Overriding the Finalize()
	// ref: http://www.geocities.com/Jeff_Louie/deterministic_destructors.htm
	!RenderPanelControl();	// Finalize

	property ::Binding::Entity^ rootEntity {
		::Binding::Entity^ get();
	}

private: RenderPanelControlImpl* mImpl;
private: System::Windows::Forms::Timer^  timer1;

private: System::ComponentModel::IContainer^  components;

private:
	/// <summary>
	/// Required designer variable.
	/// </summary>


#pragma region Windows Form Designer generated code
	/// <summary>
	/// Required method for Designer support - do not modify
	/// the contents of this method with the code editor.
	/// </summary>
	void InitializeComponent(void)
	{
		this->components = (gcnew System::ComponentModel::Container());
		this->timer1 = (gcnew System::Windows::Forms::Timer(this->components));
		this->SuspendLayout();
		// 
		// timer1
		// 
		this->timer1->Enabled = true;
		this->timer1->Interval = 10;
		this->timer1->Tick += gcnew System::EventHandler(this, &RenderPanelControl::timer1_Tick);
		// 
		// RenderPanelControl
		// 
		this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
		this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
		this->BackColor = System::Drawing::Color::Gray;
		this->Name = L"RenderPanelControl";
		this->Load += gcnew System::EventHandler(this, &RenderPanelControl::RenderPanelControl_Load);
		this->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &RenderPanelControl::RenderPanelControl_Paint);
		this->Leave += gcnew System::EventHandler(this, &RenderPanelControl::RenderPanelControl_Leave);
		this->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &RenderPanelControl::RenderPanelControl_KeyUp);
		this->Enter += gcnew System::EventHandler(this, &RenderPanelControl::RenderPanelControl_Enter);
		this->SizeChanged += gcnew System::EventHandler(this, &RenderPanelControl::RenderPanelControl_SizeChanged);
		this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &RenderPanelControl::RenderPanelControl_KeyDown);
		this->ResumeLayout(false);

	}
#pragma endregion

private: System::Void RenderPanelControl_Load(System::Object^ sender, System::EventArgs^ e);
private: System::Void timer1_Tick(System::Object^ sender, System::EventArgs^ e);
private: System::Void RenderPanelControl_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e);
private: System::Void RenderPanelControl_Enter(System::Object^ sender, System::EventArgs^ e);
private: System::Void RenderPanelControl_Leave(System::Object^ sender, System::EventArgs^ e);
private: System::Void RenderPanelControl_SizeChanged(System::Object^ sender, System::EventArgs^ e);
private: System::Void RenderPanelControl_KeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e);
private: System::Void RenderPanelControl_KeyUp(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e);
};

}	// namespace RenderPanel
