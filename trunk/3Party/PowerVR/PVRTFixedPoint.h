/******************************************************************************

 @File         PVRTFixedPoint.h

 @Title        

 @Copyright    Copyright (C) 2005 - 2008 by Imagination Technologies Limited.

 @Platform     Independant

 @Description  Set of macros and functions to make fixed-point easier to program.

******************************************************************************/
#ifndef _PVRTFIXEDPOINT_H_
#define _PVRTFIXEDPOINT_H_

#define VERTTYPE float

/* Floating-point operations */
#define VERTTYPEMUL(a,b)			( (VERTTYPE)((a)*(b)) )
#define VERTTYPEDIV(a,b)			( (VERTTYPE)((a)/(b)) )
#define VERTTYPEABS(a)				( (VERTTYPE)(fabs(a)) )
#define PVRTABS(a)					((a) <= 0 ? -(a) : (a))

#define f2vt(x)						(x)
#define vt2f(x)						(x)

#define PVRT_PI_OVER_TWO			PVRT_PI_OVER_TWOf
#define PVRT_PI						PVRT_PIf
#define PVRT_TWO_PI					PVRT_TWO_PIf
#define PVRT_ONE					PVRT_ONEf

/* If trig tables are forced ON in non-fixed-point builds then convert fixed-point trig tables results to float */
#define	PVRTCOS(x)					((float)cos(x))
#define	PVRTSIN(x)					((float)sin(x))
#define	PVRTTAN(x)					((float)tan(x))
#define	PVRTACOS(x)					((float)acos(x))

#define	PVRTFCOS(x)					((float)cos(x))
#define	PVRTFSIN(x)					((float)sin(x))
#define	PVRTFTAN(x)					((float)tan(x))
#define	PVRTFACOS(x)				((float)acos(x))

/* Useful values */
#define PVRT_PI_OVER_TWOf	(3.1415926535f / 2.0f)
#define PVRT_PIf			(3.1415926535f)
#define PVRT_TWO_PIf		(3.1415926535f * 2.0f)
#define PVRT_ONEf			(1.0f)

#endif /* _PVRTFIXEDPOINT_H_ */

/*****************************************************************************
 End of file (PVRTFixedPoint.h)
*****************************************************************************/
