namespace MCD {

template<typename Real>
Real Math<Real>::sinZeroHalfPI(Real x)
{
	// From ID software, "Slerping Clock Cycles"
	MCD_ASSUME(x >= 0 && x <= cPiOver2());
	const Real s = x * x;
	Real t = Real(-2.39e-08);
	t *= s;
	t += Real(2.7526e-06);
	t *= s;
	t += Real(-1.98409e-04);
	t *= s;
	t += Real(8.3333315e-03);
	t *= s;
	t += Real(-1.666666664e-01);
	t *= s;
	t += Real(1.0);
	t *= x;
	return t;
}

template<typename Real>
Real Math<Real>::aTanPositive(Real y, Real x)
{
	// From ID software, "Slerping Clock Cycles"
	MCD_ASSUME(y >= 0);
	MCD_ASSUME(x >= 0);

	Real a, d, t;
	if(y > x) {
		a = -x / y;
		d = cPiOver2();
	} else {
		if(x == 0)
			return 0;
		a = y / x;
		d = Real(0.0);
	}

	const Real s = a * a;
	t = Real(0.0028662257);
	t *= s;
	t += Real(-0.0161657367);
	t *= s;
	t += Real(0.0429096138);
	t *= s;
	t += Real(-0.0752896400);
	t *= s;
	t += Real(0.1065626393);
	t *= s;
	t += Real(-0.1420889944);
	t *= s;
	t += Real(0.1999355085);
	t *= s;
	t += Real(-0.3333314528);
	t *= s;
	t += Real(1.0);
	t *= a;
	t += d;
	return t;
}

}	// namespace MCD
