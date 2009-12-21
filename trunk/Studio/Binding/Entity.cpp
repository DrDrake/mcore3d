#include "stdafx.h"
#include "Entity.h"
#include "Utility.h"
#undef nullptr
#include <gcroot.h>

using namespace System::Globalization;
using namespace System::Windows::Forms;

namespace Binding {

// Reference: http://www.codeguru.com/columns/vb/article.php/c6529
// Reference: http://msdn.microsoft.com/en-us/library/aa302326.aspx
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
		str += float(Decimal::Round(Decimal(vec[0]), 5)); str += ", ";
		str += float(Decimal::Round(Decimal(vec[1]), 5)); str += ", ";
		str += float(Decimal::Round(Decimal(vec[2]), 5));
		return str;
	}

	virtual Object^ ConvertFrom(ITypeDescriptorContext^ context, CultureInfo^ culture, Object^ value) override
	{
		if(value->GetType() != String::typeid)
			return TypeConverter::ConvertFrom(context, culture, value);
		array<String^>^ ar = ((String^)value)->Split(L',');
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

void Entity::markParentDirty()
{
	if(parent)
		parent->isChildrenDirty = true;
}

Entity::Entity(MCD::Entity* entity)
{
	mImpl = entity;
	mImpl->userData.setPtr(new gcroot<Entity^>(this));	// MCD::Entity::userData will reference back the C# Entity
	mIsValid = new MCD::EntityPtr(mImpl);
	isChildrenDirty = false;
}

Entity::Entity(IntPtr entity)
{
	mImpl = reinterpret_cast<MCD::Entity*>(entity.ToPointer());
	mImpl->userData.setPtr(new gcroot<Entity^>(this));	// MCD::Entity::userData will reference back the C# Entity
	mIsValid = new MCD::EntityPtr(mImpl);
	isChildrenDirty = false;
}

Entity::~Entity()
{
	this->!Entity();
}

Entity::!Entity()
{
	delete mIsValid;
}

bool Entity::isValid()
{
	return ptr() != nullptr;
}

MCD::Entity* Entity::ptr()
{
	MCD::Entity* p = mIsValid->get();
	if(!p && mParent)
		mParent->isChildrenDirty = true;
	return p;
}

MCD::Entity* Entity::_rParent()
{
	MCD::Entity* p = ptr();
	return p ? p->parent() : nullptr;
}

MCD::Entity* Entity::_rFirstChild()
{
	MCD::Entity* p = ptr();
	return p ? p->firstChild() : nullptr;
}

MCD::Entity* Entity::_rNextSibling()
{
	MCD::Entity* p = ptr();
	return p ? p->nextSibling() : nullptr;
}

MCD::Entity* Entity::_cParent() {
	return mParent ? mParent->mImpl : nullptr;
}

MCD::Entity* Entity::_cFirstChild() {
	return mFirstChild ? mFirstChild->mImpl : nullptr;
}

MCD::Entity* Entity::_cNextSibling() {
	return mNextSibling ? mNextSibling->mImpl : nullptr;
}

void Entity::asChildOf(Entity^ parent_)
{
	if(parent_) {
		mImpl->asChildOf(parent_->mImpl);
		markParentDirty();
	}
}

void Entity::insertBefore(Entity^ sibling_)
{
	mImpl->insertBefore(sibling_->mImpl);
	markParentDirty();
}

void Entity::insertAfter(Entity^ sibling_)
{
	mImpl->insertAfter(sibling_->mImpl);
	markParentDirty();
}

void Entity::unlink()
{
	if(isValid()) {
		markParentDirty();
		mImpl->unlink();
	}
}

void Entity::destroyThis()
{
	unlink();
	delete mImpl;
	mImpl = nullptr;
}

MCD::Entity* Entity::getRawEntityPtr()
{
	return mIsValid ? mIsValid->get() : nullptr;
}

Entity^ Entity::getEntityFromRawPtr(MCD::Entity* entity)
{
	if(!entity) return nullptr;

	gcroot<Entity^>* p = entity->userData.getPtr<gcroot<Entity^> >();
	if(!p) return nullptr;

	return p->operator->();
}

bool Entity::enabled::get()
{
	return mImpl->enabled;
}

void Entity::enabled::set(bool value)
{
	mImpl->enabled = value;
}

String^ Entity::name::get()
{
	if(!ptr()) return nullptr;
	return gcnew String(mImpl->name.c_str());
}

void Entity::name::set(String^ value)
{
	mImpl->name = Utility::toWString(value);
}

Entity^ Entity::parent::get()
{
	MCD::Entity* r = _rParent();
	MCD::Entity* c = _cParent();

	if(r != c || (mParent && !c)) {
		if(mParent)
			mParent->isChildrenDirty = true;
		mParent = (r ? gcnew Entity(r) : nullptr);
	}

	return mParent;
}

Entity^ Entity::firstChild::get()
{
	MCD::Entity* r = _rFirstChild();
	MCD::Entity* c = _cFirstChild();

	if(r != c || (mFirstChild && !c)) {
		isChildrenDirty = true;
		if(mFirstChild = (r ? gcnew Entity(r) : nullptr))
			mFirstChild->mParent = this;
	}

	return mFirstChild;
}

Entity^ Entity::nextSibling::get()
{
	MCD::Entity* r = _rNextSibling();
	MCD::Entity* c = _cNextSibling();

	if(r != c || (mNextSibling && !c)) {
		if(mParent)
			mParent->isChildrenDirty = true;
		if(mNextSibling = (r ? gcnew Entity(r) : nullptr))
			mNextSibling->mParent = mParent;
	}

	return mNextSibling;
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
	// Get the scale first
	array<float>^ currentScale = this->scale;

	MCD::Mat33f mat = mImpl->localTransform.mat33();
	// Undo the effect of scaling on the 3x3 matrix
	for(size_t i=0; i<3; ++i) for(size_t j=0; j<3; ++j)
		mat[i][j] /= currentScale[j];

	array<float>^ a = gcnew array<float>(3);
	MCD::Vec3f angles;
	mat.getRotationXYZ(angles.x, angles.y, angles.z);

	a[0] = MCD::Mathf::toDegree(angles[0]);
	a[1] = MCD::Mathf::toDegree(angles[1]);
	a[2] = MCD::Mathf::toDegree(angles[2]);

	return a;
}

void Entity::rotation::set(array<float>^ value)
{
	// Get the scale first
	array<float>^ currentScale = this->scale;

	MCD::Mat33f mat = MCD::Mat33f::makeXYZRotation(
		MCD::Mathf::toRadian(value[0]),
		MCD::Mathf::toRadian(value[1]),
		MCD::Mathf::toRadian(value[2])
	);

	// Apply the scaling back to the 3x3 matrix
	for(size_t i=0; i<3; ++i) for(size_t j=0; j<3; ++j)
		mat[i][j] *= currentScale[j];

	mImpl->localTransform.setMat33(mat);
}

// Get only the scaling part of the transformation matrix
// Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=491578
array<float>^ Entity::scale::get()
{
	array<float>^ a = gcnew array<float>(3);
	const MCD::Vec3f s = mImpl->localTransform.scale();

	a[0] = s.x;
	a[1] = s.y;
	a[2] = s.z;

	return a;
}

void Entity::scale::set(array<float>^ value)
{
	if(value[0] <= 0 || value[1] <= 0 || value[2] <= 0)
		throw gcnew System::Exception("Values should be greater than zero");

	mImpl->localTransform.setScale(MCD::Vec3f(value[0], value[1], value[2]));
}

EntityPreorderIterator::EntityPreorderIterator(Entity^ e)
	: mCurrent(e), mStart(e)
{
}

bool EntityPreorderIterator::ended()
{
	return mCurrent == nullptr;
}

Entity^ EntityPreorderIterator::next()
{
	// After an upward movement is preformed, we will not visit the child again
	bool noChildMove = false;

	while(mCurrent)
	{
		Entity ^fc, ^ns;
		if(!noChildMove && (fc = mCurrent->firstChild))
			return mCurrent = fc;
		else if(ns = mCurrent->nextSibling)
			return mCurrent = ns;
		else
		{
			mCurrent = mCurrent->parent;
			noChildMove = true;

			if(mCurrent == mStart)
				mCurrent = nullptr;
		}
	}

	return mCurrent;
}

}	// namespace Binding
