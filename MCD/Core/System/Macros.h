#ifndef __MCD_CORE_SYSTEM_MACROS__
#define __MCD_CORE_SYSTEM_MACROS__

// This file contains some useful macros

//! Act as a comma
#define MCD_MACRO_COMMA ,

//! Join 2 macros together, see BOOST_JOIN for more explanation
#define MCD_MACRO_JOIN( X, Y) _MCD_MACRO_DO_JOIN_( X, Y )
#define _MCD_MACRO_DO_JOIN_( X, Y ) _MCD_MACRO_DO_JOIN2_(X,Y)
#define _MCD_MACRO_DO_JOIN2_( X, Y ) X##Y

/*! To get back the host object from a member variable.
	Use this macro to declare the \em GetOuter and \em GetOuterSafe functions
	to access the outer object.

	For example, you want to access the ClientInfo from it's member variable mActClient and mActServer:
	\code
	struct ClientInfo {
		struct ActivityClient : public LinkListBase::NodeBase {
			MCD_DECLAR_GET_OUTER_OBJ(ClientInfo, mActClient);
		} mActClient;

		struct ActivityServer : public LinkListBase::NodeBase {
			MCD_DECLAR_GET_OUTER_OBJ(ClientInfo, mActServer);
		} mActServer;
	};

	ClientInfo info;
	ClientInfo& infoRef = info.mActServer.getOuter();
	ClientInfo* infoPtr = info.mActServer.getOuterSafe();
	\endcode

	\note The same restriction of \em offsetof macro apply on \em MCD_DECLAR_GET_OUTER_OBJ
 */
#define MCD_DECLAR_GET_OUTER_OBJ(OuterClass, ThisVar) \
	OuterClass& getOuter() { return *(OuterClass*) \
	((intptr_t(this) + 1) - intptr_t(&((OuterClass*)1)->ThisVar)); } \
	const OuterClass& getOuter() const { return *(OuterClass*) \
	((intptr_t(this) + 1) - intptr_t(&((OuterClass*)1)->ThisVar)); } \
	OuterClass* getOuterSafe() { return this ? &getOuter() : nullptr; } \
	const OuterClass* getOuterSafe() const { return this ? &getOuter() : nullptr; }

#endif	// __MCD_CORE_SYSTEM_MACROS__
