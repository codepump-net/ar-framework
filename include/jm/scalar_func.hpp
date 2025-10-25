//
//  jm_scalar_func.hpp
//  SystemCalib
//
//  Created by Hyun Joon Shin on 2023/08/24.
//

#ifndef jm_scalar_func_h
#define jm_scalar_func_h

#include "vec.hpp"

namespace jm {

const scalar_t PI = 3.14159265358979f;

template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto degrees		( T d )	{ return d*180/PI; }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto radians		( T d )	{ return d*PI/180; }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto sin			( T a ) { return ::sinf(float(a)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto cos			( T a )	{ return ::cosf(float(a)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto tan			( T a ) { return ::tanf(float(a)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto sinh			( T d )	{ return ::sinhf(float(d)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto cosh			( T d )	{ return ::coshf(float(d)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto tanh			( T d )	{ return ::tanhf(float(d)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto asin			( T d )	{ return ::asinf(float(d)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto acos			( T d )	{ return ::acosf(float(d)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto atan			( T d )	{ return ::atanf(float(d)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto asinh			( T d )	{ return ::asinhf(float(d)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto acosh			( T d )	{ return ::acoshf(float(d)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto atanh			( T d )	{ return ::atanhf(float(d)); }

template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto exp			( T d )	{ return ::expf(float(d)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto log			( T d )	{ return ::logf(float(d)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto exp2			( T d )	{ return ::exp2f(float(d)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto log2			( T d )	{ return ::log2f(float(d)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto sqrt			( T d )	{ return ::sqrtf(float(d)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto abs			( T d )	{ return ::abs(float(d)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto floor			( T d )	{ return ::floorf(float(d)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto trunc			( T d )	{ return ::truncf(float(d)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto round			( T d )	{ return ::roundf(float(d)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto ceil			( T d )	{ return ::ceilf(float(d)); }

template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto fract			( T d )	{ return d-floor(d); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto inversesqrt	( T d )	{ return 1.f/sqrt(d); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>	auto sign			( T d )	{ return d>0?T(1):(d<0?T(-1):T(0)); }
template<typename T,typename = std::enable_if_t<!is_vector_v<T>>> 	auto roundEven		( T x ) { return floor(x*2) == ceil(x*2) ? ( int(round(x))%2==0?ceil(x):floor(x) ) : round(x); }

template<> inline		auto sin<double>	( double a ) { return ::sin(a); }
template<> inline		auto cos<double>	( double a ) { return ::cos(a); }
template<> inline		auto tan<double>	( double a ) { return ::tan(a); }
template<> inline		auto sinh<double>	( double d ) { return ::sinh(d); }
template<> inline		auto cosh<double>	( double d ) { return ::cosh(d); }
template<> inline		auto tanh<double>	( double d ) { return ::tanh(d); }
template<> inline		auto asin<double>	( double d ) { return ::asin(d); }
template<> inline		auto acos<double>	( double d ) { return ::acos(d); }
template<> inline		auto atan<double>	( double d ) { return ::atan(d); }
template<> inline		auto asinh<double>	( double d ) { return ::asinh(d); }
template<> inline		auto acosh<double>	( double d ) { return ::acosh(d); }
template<> inline		auto atanh<double>	( double d ) { return ::atanh(d); }

template<> inline		auto exp<double>	( double a ) { return ::exp(a); }
template<> inline		auto log<double>	( double a ) { return ::log(a); }
template<> inline		auto exp2<double>	( double a ) { return ::exp2(a); }
template<> inline		auto log2<double>	( double a ) { return ::log2(a); }
template<> inline		auto sqrt<double>	( double a ) { return ::sqrt(a); }
template<> inline		auto abs<double>	( double a ) { return ::abs(a); }
template<> inline		auto floor<double>	( double a ) { return ::floor(a); }
template<> inline		auto trunc<double>	( double a ) { return ::trunc(a); }
template<> inline		auto round<double>	( double a ) { return ::round(a); }
template<> inline		auto ceil<double>	( double a ) { return ::ceil(a); }

template<typename T0, typename T1, typename=std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
bool lessThan( T0 a, T1 b ) { return a <  b; }		
template<typename T0, typename T1, typename=std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
bool lessThanEqual( T0 a, T1 b ) { return a <= b; }	
template<typename T0, typename T1, typename=std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
bool greaterThan( T0 a, T1 b ) { return a >  b; }	
template<typename T0, typename T1, typename=std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
bool greaterThanEqua( T0 a, T1 b ) { return a >= b; }
template<typename T0, typename T1, typename=std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
bool equal( T0 a, T1 b ) { return a == b; }			
template<typename T0, typename T1, typename=std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
bool notEqual( T0 a, T1 b ) { return a != b; }		



template<typename T0, typename T1,typename = std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
auto atan	( T0 a, T1 b ) {
	return atan2f(float(a),float(b));
}

template<typename T0, typename T1,typename = std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
auto pow	( T0 a, T1 b ) {
	return powf(float(a),float(b));
}

template<typename T0, typename T1,typename = std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
auto mod	( T0 a, T1 b ) {
	return  a-b*floor(a/b);
}

template<typename T0, typename T1,typename = std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
auto modf	( T0 x, T1& y) {
	y = fract(x); return floor(x);
}

template<typename T0, typename T1,typename = std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
auto min	( T0 a, T1 b ) {
	using scalar = typename std::common_type_t<T0,T1>;
	return a<b?scalar(a):scalar(b);
}

template<typename T0, typename T1,typename = std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
auto max	( T0 a, T1 b ) {
	using scalar = typename std::common_type_t<T0,T1>;
	return a>b?scalar(a):scalar(b);
}

template<typename T0, typename T1,typename = std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
auto clamp		( float  d, T0 e1, T1 e2 ) {
	using scalar = typename std::common_type_t< T0, T1, float>;
	return d>e2?scalar(e2):(d<e1?scalar(e1):d);
}

template<typename T0, typename T1,typename = std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
auto clamp		( double d, T0 e1, T1 e2 ) {
	using scalar = double;
	return d>e2?scalar(e2):(d<e1?scalar(e1):d);
}

template<typename T0, typename T1,typename = std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
auto	mix	(T0  d1, T1 d2, float t ) {
	using scalar = typename std::common_type_t<T0,T1>;
	return scalar ((1-t)*scalar(d1)+t*scalar(d2));
}

template<typename T0, typename T1,typename = std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
auto	mix	(T0  d1, T1 d2, double t ) {
	using scalar = double;
	return scalar ((1-t)*scalar(d1)+t*scalar(d2));
}

template<typename T0, typename T1,typename = std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
auto	mix	(T0  d1, T1 d2, bool t ) {
	using scalar = typename std::common_type_t<T0,T1>;
	return t?scalar(d2):scalar(d1);
}

template<typename T0, typename T1,typename = std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
auto step		( T0 a, T1 b ) {
	using scalar = typename std::common_type_t<T0,T1>;
	return scalar(b)<scalar(a)?scalar(0):scalar(1);
}

template<typename T0, typename T1,typename = std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
auto	smoothstep	(T0 xx1, T1 xx2, float yy ) {
	using scalar = typename std::common_type_t<T0, T1, float>;
	scalar t = clamp(scalar(yy-xx1)/scalar(xx2-xx1),scalar(0),scalar(1));
	return t*t*(scalar(3)-scalar(2)*t);
}

template<typename T0, typename T1,typename = std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
auto	smoothstep	(T0 xx1, T1 xx2, double yy ) {
	using scalar = double;
	scalar t = clamp(scalar(yy-xx1)/scalar(xx2-xx1),scalar(0),scalar(1));
	return t*t*(scalar(3)-scalar(2)*t);
}

template<typename T,typename = std::enable_if_t<!is_vector_v<T>>>
auto  frexp( T xx, int& expr) {
	using scalar = typename std::common_type_t<float,T>;
	if( xx == 0 ) {
		expr = 0;
		return scalar(0);
	}
	scalar e = log2(abs(scalar(xx)));
	expr = (int)ceil( e );
	if( expr == (int)floor(e) ) expr++;
	return scalar(xx)/exp2( scalar(expr) );
}

inline float  ldexp(float xx, int expr) { return xx * ::exp2f(float(expr)); }
inline double  ldexp(double xx, int expr) { return xx * ::exp2(expr); }

template<typename T0, typename T1,typename = std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
auto fma( float a, T0 b, T1 c) {
	using scalar = typename std::common_type_t<float, T0, T1>;
	return scalar(a)*scalar(b)+scalar(c);
}

template<typename T0, typename T1,typename = std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
double fma( double a, T0 b, T1 c) {
	using scalar = double;
	return scalar(a)*scalar(b)+scalar(c);
}

template<typename T0, typename T1,typename = std::enable_if_t<!is_vector_v<T0> && !is_vector_v<T1>>>
auto fma( int a, T0 b, T1 c) {
	using scalar = typename std::common_type_t<float, T0, T1>;
	return scalar(a)*scalar(b)+scalar(c);
}

inline int		floatBitsToInt		( float d ) { int v = *((int*)&d); return v; }
inline uint		floatBitsToUint		( float d ) { uint v = *((uint*)&d); return v; }
inline float	intBitsToFloat		( int   d ) { float v = *((float*)&d); return v; }
inline float	uintBitsToFloat		( uint  d ) { float v = *((float*)&d); return v; }

}

#endif /* jm_scalar_func_h */
