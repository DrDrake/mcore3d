#ifndef __SGE_CORE_SYSTEM_MACROS__
#define __SGE_CORE_SYSTEM_MACROS__

// This file contains some useful macros

//! Act as a comma
#define SGE_MACRO_COMMA ,

//! Join 2 macros together, see BOOST_JOIN for more explanation
#define SGE_MACRO_JOIN( X, Y) _SGE_MACRO_DO_JOIN_( X, Y )
#define _SGE_MACRO_DO_JOIN_( X, Y ) _SGE_MACRO_DO_JOIN2_(X,Y)
#define _SGE_MACRO_DO_JOIN2_( X, Y ) X##Y

/*! To get back the host object from a member variable.
	Use this macro to declare the \em GetOuter and \em GetOuterSafe functions
	to access the outer object.

	For example, you want to access the ClientInfo from it's member variable mActClient and mActServer:
	\code
	struct ClientInfo {
		struct ActivityClient : public LinkListBase::NodeBase {
			SGE_DECLAR_GET_OUTER_OBJ(ClientInfo, mActClient);
		} mActClient;

		struct ActivityServer : public LinkListBase::NodeBase {
			SGE_DECLAR_GET_OUTER_OBJ(ClientInfo, mActServer);
		} mActServer;
	};

	ClientInfo info;
	ClientInfo& infoRef = info.mActServer.getOuter();
	ClientInfo* infoPtr = info.mActServer.getOuterSafe();
	\endcode

	\note The same restriction of \em offsetof macro apply on \em SGE_DECLAR_GET_OUTER_OBJ
 */
#define SGE_DECLAR_GET_OUTER_OBJ(OuterClass, ThisVar) \
	OuterClass& getOuter() { return *(OuterClass*) \
	((intptr_t(this) + 1) - intptr_t(&((OuterClass*)1)->ThisVar)); } \
	const OuterClass& getOuter() const { return *(OuterClass*) \
	((intptr_t(this) + 1) - intptr_t(&((OuterClass*)1)->ThisVar)); } \
	OuterClass* getOuterSafe() { return this ? &getOuter() : nullptr; } \
	const OuterClass* getOuterSafe() const { return this ? &getOuter() : nullptr; }

#endif	// __SGE_CORE_SYSTEM_MACROS__
