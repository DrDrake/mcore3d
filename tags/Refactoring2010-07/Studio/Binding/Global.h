#pragma once

namespace Binding {

/*!	A place for everyone to access
 */
public ref class Global
{
public:
	Global(System::Windows::Forms::TextBox^ textBox);

	static Global^ instance;

protected:
	~Global();
	!Global();
};

}
