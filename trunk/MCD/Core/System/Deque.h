// A warning free version of STL deque header
#include "Platform.h"

#ifdef MCD_VC
#	pragma warning(push)
#	pragma warning(disable: 6011)
#endif
#include <deque>
#include <stack>
#ifdef MCD_VC
#	pragma warning(pop)
#endif
