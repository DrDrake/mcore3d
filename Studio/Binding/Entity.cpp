#include "stdafx.h"
#include "Entity.h"
#include "Utility.h"
#undef nullptr

using namespace System::Globalization;
using namespace System::Windows::Forms;

namespace Binding {

public ref class Vector3Converter : public TypeConverter
{
public:
	virtual bool CanConvertFrom(ITypeDescriptorContext^ context, Type^ sourceType) override
	{
		if(sourceType == String::typeid)
			return true;
		return TypeConverter::CanConvertFrom(context, sourceType);
	}

	virtual bool CanConvertTo(ITypeDescriptorContext^ context, Type^ destinationType) override
	{
		if(destinationType == String::typeid)
			return true;
		return TypeConverter::CanConvertTo(context, destinationType);
	}

	virtual Object^ ConvertTo(ITypeDescriptorContext^ context, CultureInfo^ culture, Object^ value, Type^ destinationType) override
	{
		if(destinationType != String::typeid)
			return TypeConverter::ConvertFrom(context, culture, value);

		array<float>^ vec = (array<float>^)(value);
		String^ str = gcnew String("");
		str += vec[0]; str += ", ";
		str += vec[1]; str += ", ";
		str += vec[2];
		return str;
	}

	virtual Object^ ConvertFrom(ITypeDescriptorContext^ context, CultureInfo^ culture, Object^ value) override
	{
		if(value->GetType() != String::typeid)
			return TypeConverter::ConvertFrom(context, culture, value);
		array<String^>^ ar = ((String^)value)->Split(gcnew array<wchar_t>{L','});
		return gcnew array<float>{ System::Single::Parse(ar[0]), System::Single::Parse(ar[1]), System::Single::Parse(ar[2]) };
	}

/*	virtual bool GetPropertiesSupported(ITypeDescriptorContext^ context) override
	{
		return true;
	}

	virtual PropertyDescriptorCollection^ GetProperties(ITypeDescriptorContext^ context, Object^ value, array<Attribute^>^ attributes) override
	{
		return TypeDescriptor::GetProperties(value);
	}*/
};

Entity::Entity(MCD::Entity* entity)
{
	mImpl = entity;
	treeViewNode = gcnew TreeNode(this->name);
	treeViewNode->Tag = this;
}

Entity::Entity(IntPtr entity)
{
	mImpl = reinterpret_cast<MCD::Entity*>(entity.ToPointer());
	treeViewNode = gcnew TreeNode(this->name);

	// Loop all the nodes once, and the tree view nodes will be constructed correctly
	Entity^ current = this;
	do
	{
		// After an upward movement is preformed, we will not visit the child again
		bool noChildMove = false;
		while(current)
		{
			if(current->firstChild && !noChildMove)
			{
				current = current->firstChild;
				break;
			}
			else if(current->nextSlibing)
			{
				current = current->nextSlibing;
				break;
			}
			else
			{
				current = current->parent;
				noChildMove = true;

				if(current == this)
					current = nullptr;
			}
		}
	} while(current != nullptr);
}

void Entity::link(Entity^ parent)
{
	if(parent) {
		parent->treeViewNode->Nodes->Add(treeViewNode);
		mImpl->link(parent->mImpl);
	}
}

void Entity::unlink()
{
	mImpl->unlink();
	if(treeViewNode->Parent != nullptr)
		treeViewNode->Parent->Nodes->Remove(treeViewNode);
}

String^ Entity::name::get()
{
   return gcnew String(mImpl->name.c_str());
}

void Entity::name::set(String^ value)
{
	mImpl->name = Utility::toWString(value);
	treeViewNode->Text = value;
}

Entity^ Entity::parent::get()
{
	MCD::Entity* n = mImpl->parent();

	// Cache hit
	if(mParent != nullptr && mParent->mImpl == n)
		return mParent;

	return n ? gcnew Entity(n) : nullptr;
}

Entity^ Entity::firstChild::get()
{
	MCD::Entity* n = mImpl->firstChild();

	// Cache hit
	if(mFirstChild != nullptr && mFirstChild->mImpl == n)
		return mFirstChild;

	// Remove from the TreeView (if any)
	if(mFirstChild != nullptr)
		mFirstChild->treeViewNode->Remove();

	if(n) {
		mFirstChild = gcnew Entity(n);
		mFirstChild->mParent = this;
		treeViewNode->Nodes->Add(mFirstChild->treeViewNode);
	}
	else
		mFirstChild = nullptr;

	return mFirstChild;
}

Entity^ Entity::nextSlibing::get()
{
	MCD::Entity* n = mImpl->nextSlibing();

	// Cache hit
	if(mNextSlibing != nullptr && mNextSlibing->mImpl == n)
		return mNextSlibing;

	// Remove from the TreeView (if any)
	if(mNextSlibing != nullptr)
		mNextSlibing->treeViewNode->Remove();

	if(n) {
		mNextSlibing = gcnew Entity(n);
		mNextSlibing->mParent = mParent;
		parent->treeViewNode->Nodes->Add(mNextSlibing->treeViewNode);
	}
	else
		mNextSlibing = nullptr;

	return mNextSlibing;
}

array<float>^ Entity::translation::get()
{
	array<float>^ a = gcnew array<float>(3);
	MCD::Vec3f trans = mImpl->localTransform.translation();

	a[0] = trans[0];
	a[1] = trans[1];
	a[2] = trans[2];

	return a;
}

void Entity::translation::set(array<float>^ value)
{
	MCD::Vec3f trans(value[0], value[1], value[2]);
	mImpl->localTransform.setTranslation(trans);
}

array<float>^ Entity::rotation::get()
{
	array<float>^ a = gcnew array<float>(3);
	MCD::Vec3f angles;
	mImpl->localTransform.mat33().getRotationXYZ(angles.x, angles.y, angles.z);

	a[0] = MCD::Mathf::toDegree(angles[0]);
	a[1] = MCD::Mathf::toDegree(angles[1]);
	a[2] = MCD::Mathf::toDegree(angles[2]);

	return a;
}

void Entity::rotation::set(array<float>^ value)
{
	mImpl->localTransform.setMat33(MCD::Mat33f::rotateXYZ(
		MCD::Mathf::toRadian(value[0]),
		MCD::Mathf::toRadian(value[1]),
		MCD::Mathf::toRadian(value[2]))
	);
}

}
