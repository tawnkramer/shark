////////////////////////////////////
// TMath.cpp
//
// Author : Tawn Kramer
// Nov 12, 2003

#include <math.h>
#include <string.h>
#include "tmath.h"

using namespace TMath;

const double	TMath::dPI = 3.1415926535897932384626433832795028841971;
const float		TMath::PI	= 3.1415926535897932f;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// class Vector2

Vector2::Vector2()
{
	x = y = 0.0f;
}

Vector2::Vector2(float x, float y)
{
	this->x = x;
	this->y = y;
}

Vector2::Vector2(const Vector2& v)
{
	this->x = v.x;
	this->y = v.y;
}

Vector2::Vector2(const Vector3& v)
{
	this->x = v.x;
	this->y = v.y;
}

Vector2::Vector2(const Vector4& v)
{
	this->x = v.x;
	this->y = v.y;
}

Vector2&	Vector2::operator=(const Vector2& v)
{
	x = v.x;
	y = v.y;
	return( *this );
}

Vector2&	Vector2::operator=(const Vector3& v)
{
	x = v.x;
	y = v.y;
	return( *this );
}

Vector2&	Vector2::operator=(const Vector4& v)
{
	x = v.x;
	y = v.y;
	return( *this );
}

Vector2	Vector2::operator+(const Vector2& v) const
{
	return Vector2(x + v.x, y + v.y);
}

Vector2	Vector2::operator-(const Vector2& v) const
{
	return Vector2(x - v.x, y - v.y);
}

Vector2	Vector2::operator*(const float scalar) const
{
	return Vector2(x * scalar, y * scalar);
}

Vector2	Vector2::operator/(const float scalar) const
{
	return Vector2(x / scalar, y / scalar);
}

Vector2&	Vector2::operator+=(const Vector2& v)
{
	x += v.x;
	y += v.y;
	return(*this);
}

Vector2&	Vector2::operator-=(const Vector2& v)
{
	x -= v.x;
	y -= v.y;
	return(*this);
}

Vector2&	Vector2::operator*=(const float scalar)
{
	x *= scalar;
	y *= scalar;
	return(*this);
}

Vector2&	Vector2::operator/=(const float scalar)
{
	x /= scalar;
	y /= scalar;
	return(*this);
}

bool		Vector2::operator==(const Vector2& v) const
{
	return( this->x == v.x && this->y == v.y );
}

float	Vector2::operator[](const int i) const
{
	return (i == 0 ? x : y);
}

float	Vector2::Dot(const Vector2& v)
{
	return (x * v.x + y * v.y);
}

float	Vector2::Normalize()
{
	float mag = this->Mag();
	
	if(mag == 0)
		return mag;

	x = x / mag;
	y = y / mag;

	return mag;
}

float	Vector2::Mag() const
{
	if( x == 0 && y == 0 )
		return 0;

	return (sqrtf((x * x) + (y * y)));
}

float	Vector2::Mag2() const
{
	return ((x * x) + (y * y));
}

float	Vector2::Dist(const Vector2& v) const
{
	Vector2 a(*this);

	return((a - v).Mag());
}

float	Vector2::RadAngleBetween(const Vector2& v) const
{
	Vector2 a(*this);
	Vector2 b(v);

	a.Normalize();
	b.Normalize();
	
	return (acosf(a.Dot(b)));
}

float	Vector2::DegAngleBetween(const Vector2& v) const
{
	return( RadToDeg(this->RadAngleBetween(v)));
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Vector3

Vector3::Vector3()
{
	x = y = z = 0.0f;
}

Vector3::Vector3(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector3::Vector3(const Vector2& v)
{
	this->x = v.x;
	this->y = v.y;
	this->z = 0.0f;
}

Vector3::Vector3(const Vector3& v)
{
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
}

Vector3::Vector3(const Vector4& v)
{
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
}

Vector3&	Vector3::operator=(const Vector2& v)
{
	x = v.x;
	y = v.y;
	z = 0.0f;
	return( *this );
}

Vector3&	Vector3::operator=(const Vector3& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	return( *this );
}

Vector3&	Vector3::operator=(const Vector4& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	return( *this );
}

Vector3	Vector3::operator+(const Vector3& v) const
{
	return Vector3(x + v.x, y + v.y, z + v.z);
}

Vector3	Vector3::operator-(const Vector3& v) const
{
	return Vector3(x - v.x, y - v.y, z - v.z);
}

Vector3	Vector3::operator*(const Vector3& v) const
{
	return Vector3(x * v.x, y * v.y, z * v.z);
}

Vector3	Vector3::operator*(const float scalar) const
{
	return Vector3(x * scalar, y * scalar, z * scalar);
}

Vector3	Vector3::operator/(const float scalar) const
{
	return Vector3(x / scalar, y / scalar, z / scalar);
}

Vector3&	Vector3::operator+=(const Vector3& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return(*this);
}

Vector3&	Vector3::operator-=(const Vector3& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return(*this);
}

Vector3&	Vector3::operator*=(const float scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return(*this);
}

Vector3&	Vector3::operator/=(const float scalar)
{
	float inv = 1.0f / scalar;
	x *= inv;
	y *= inv;
	z *= inv;
	return(*this);
}

bool		Vector3::operator==(const Vector3& v) const
{
	return( this->x == v.x && this->y == v.y && this->z == v.z );
}

bool		Vector3::operator!=(const Vector3& v) const
{
	return( this->x != v.x || this->y != v.y || this->z != v.z );
}

float	Vector3::operator[](const int i) const
{
	if ( i == 0 )
		return x;
	
	if ( i == 1)
		return y;

	return z;
}

float&	Vector3::operator[](const int i)
{
	if ( i == 0 )
		return x;

	if ( i == 1)
		return y;

	return z;
}

float	Vector3::Dot(const Vector3& v) const
{
	return (x * v.x + y * v.y + z * v.z);
}

Vector3	Vector3::Cross(const Vector3& v) const
{
    Vector3 ret(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	return ret;
}


float	Vector3::Normalize()
{
	float mag = this->Mag();
	
	if(mag == 0)
		return mag;

	float recip = 1.0f / mag;

	x *= recip;
	y *= recip;
	z *= recip;

	return mag;
}

float	Vector3::Mag() const
{
	if(x == 0 && y == 0 && z == 0)
		return 0;

	return (sqrtf((x * x) + (y * y) + (z * z)));
}

float	Vector3::Mag2() const
{
	return ((x * x) + (y * y) + (z * z));
}

float	Vector3::Dist(const Vector3& v) const
{
	Vector3 a(*this);

	return((a - v).Mag());
}

float	Vector3::RadAngleBetween(const Vector3& v) const
{
	Vector3 a(*this);
	Vector3 b(v);

	a.Normalize();
	b.Normalize();
	
	return (acosf(a.Dot(b)));
}

float	Vector3::DegAngleBetween(const Vector3& v) const
{
	return( RadToDeg(this->RadAngleBetween(v)));
}

Quaternion Vector3::DeltaRotBetween(const Vector3& target) const
{
	Vector3 thisNormalized = *this, targetNormalized = target;
	thisNormalized.Normalize();
	targetNormalized.Normalize();
	
	float diff = (thisNormalized - targetNormalized).Mag2();
	if(diff < 0.000001f)
		//it's critical that this is not too large. even 0.001f is large enough to cause
		//visible "chunking" as these axis approach each other.
	{
		//these are parallel. return unit quat.
		return Quaternion();
	}

	Vector3 N = thisNormalized.Cross(targetNormalized);
	N.Normalize();
	float S = thisNormalized.Dot(targetNormalized);
	float W = sqrtf((1.0f + S) * 0.5f);
	float B = sqrtf((1.0f - S) * 0.5f);
	N *= B;
	return Quaternion(N.x, N.y, N.z, W);
}

Vector3	Vector3::Lerp(const Vector3& v, float a) const
{
	float inv = 1.0f - a;
	return Vector3(x * inv + v.x * a, y * inv + v.y * a, z * inv + v.z * a);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Vector4

Vector4::Vector4() : x(0.0f) , y (0.0f) , z(0.0f), w(1.0f)
{
}

Vector4::Vector4(float x, float y, float z, float w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

Vector4::Vector4(const Vector2& v)
{
	this->x = v.x;
	this->y = v.y;
	this->z = 0.0f;
	this->w = 0.0f;
}

Vector4::Vector4(const Vector3& v)
{
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
	this->w = 0.0f;
}

Vector4::Vector4(const Vector4& v)
{
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
	this->w = v.w;
}

Vector4	Vector4::operator+(const Vector4& v) const
{
	return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
}

Vector4	Vector4::operator-(const Vector4& v) const
{
	return Vector4(x - v.x, y - v.y, z - v.z, w - v.w);
}

Vector4	Vector4::operator*(const Vector4& v) const
{
	return Vector4(x * v.x, y * v.y, z * v.z, w * v.w);
}

Vector4	Vector4::operator*(const float scalar) const
{
	return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
}

Vector4	Vector4::operator/(const float scalar) const
{
	return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
}

Vector4&	Vector4::operator+=(const Vector4& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return(*this);
}

Vector4&	Vector4::operator-=(const Vector4& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return(*this);
}

Vector4&	Vector4::operator*=(const float scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;
	return(*this);
}

Vector4&	Vector4::operator/=(const float scalar)
{
	x /= scalar;
	y /= scalar;
	z /= scalar;
	w /= scalar;
	return(*this);
}

bool		Vector4::operator==(const Vector4& v) const
{
	return( this->x == v.x && this->y == v.y && this->z == v.z && this->w == v.w);
}

float	Vector4::operator[](const int i) const
{
	if ( i == 0 )
		return x;
	
	if ( i == 1)
		return y;

	if ( i == 2)
		return z;

	return w;
}

void	Vector4::Set(float x, float y, float z, float w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

float	Vector4::Dot(const Vector4& v) const
{
	return (x * v.x + y * v.y + z * v.z + w * v.w);
}

float	Vector4::Normalize()
{
	float mag = this->Mag();
	
	if(mag == 0)
		return mag;

	float recip = 1.0f / mag;

	x *= recip;
	y *= recip;
	z *= recip;
	w *= recip;

	return mag;
}

float	Vector4::Mag() const
{
	if(x == 0 && y == 0 && z == 0 && w == 0)
		return 0;

	return (sqrtf((x * x) + (y * y) + (z * z) + (w * w)));
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//class Quaternion

Quaternion::Quaternion()
{
	x = y = z = 0.0f;
	w = 1.0f;
}

Quaternion::Quaternion(float x, float y, float z, float w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

void Quaternion::ToAxisAngle(Vector3& axis, float& angle) const
{
	float halfAngle = acosf(w);
	float sinHalfAngle = sinf(halfAngle);
	if (sinHalfAngle)
	{
		axis.x = x / sinHalfAngle;
		axis.y = y / sinHalfAngle;
		axis.z = z / sinHalfAngle;
	}
	angle = 2.0f * halfAngle;
}

void Quaternion::FromAxisAngle(const Vector3& axis, const float& angle)
{
	float sina = sinf(angle * 0.5f);
	float cosa = cosf(angle * 0.5f);
	w = cosa;
	x = sina*axis.x;
	y = sina*axis.y;
	z = sina*axis.z;
}

Quaternion  Quaternion::Inverse() const
{
	Quaternion r;
	float norm = x*x + y*y + z*z + w*w;
	float invNorm = 1.0f / norm;
	r.x = -x * invNorm;
	r.y = -y * invNorm;
	r.z = -z * invNorm;
	r.w = w * invNorm;

	return r;
}

Quaternion Quaternion::operator*(const Quaternion& quat) const
{
	return Quaternion(
		x * quat.w + w * quat.x - z * quat.y + y * quat.z,
		y * quat.w + z * quat.x + w * quat.y - x * quat.z,
		z * quat.w - y * quat.x + x * quat.y + w * quat.z,
		w * quat.w - x * quat.x - y * quat.y - z * quat.z
		);
}

Vector3 Quaternion::operator*(const Vector3 &vec) const
{
	Quaternion v(vec.x, vec.y, vec.z, 0);
	Quaternion vq_ = v * Conjugate();
	Quaternion qresult = *this * vq_;

	return Vector3(qresult.x, qresult.y, qresult.z);
}

/*
Vector3	Quaternion::operator*(const Vector3& vec) const
{
	Matrix33 m;
	m.FromQuat(*this);
	return (m * vec);
}*/

Quaternion	Quaternion::operator*(const float scalar) const
{
	return Quaternion(x * scalar, y * scalar, z * scalar, w * scalar);
}

void Quaternion::operator*=(const Quaternion& quat)
{
	*this = *this * quat;
}

Quaternion Quaternion::operator-(const Quaternion& quat) const
{
	return (*this * quat.Inverse());
}

Quaternion Quaternion::Lerp(const Quaternion& to, float percent) const
{
	return Quaternion( x + (to.x - x) * percent, y + (to.y - y) * percent, z + (to.z - z) * percent, w + (to.w - w) * percent);
}

Quaternion Quaternion::Slerp(const Quaternion& to, float a) const
{
	//special cases requiring no calculation
	if(a == 0.0f)
		return *this;
	if(a == 1.0f)
		return to;

	Quaternion q;
	float u, v, ang, s;
	float dot = this->Dot(to);
	ang = dot < -1 ? PI : dot > 1 ? 0 : acosf(dot); /* acos gives NaN for dot slightly out of range */
	s = sinf(ang);
	
	if( s == 0) return 
		ang < PI/2 ? *this : to;

	u = sinf((1-a)*ang)/s;
	v = sinf(a*ang)/s;

	q.x = u * x + v * to.x;
	q.y = u * y + v * to.y;
	q.z = u * z + v * to.z;
	q.w = u * w + v * to.w;
	return q;
}

//Interpolate the quat between Q01 and Q02 and use the previous Q00 and next Q03 to maintain continuity
Quaternion	Quaternion::CatmullRom(const Quaternion &Q00, const Quaternion &Q01, const Quaternion &Q02, const Quaternion &Q03, float percent) const
{
	Quaternion Q10, Q11, Q12, Q20, Q21, result;
	Q10 = Q00.Lerp(Q01, percent + 1.0f);
	Q11 = Q01.Lerp(Q02, percent);
	Q12 = Q02.Lerp(Q03, percent - 1.0f);
	Q20 = Q10.Lerp(Q11, percent * 0.5f + 0.5f);
	Q21 = Q11.Lerp(Q12, percent * 0.5f);
	result = Q20.Lerp(Q21, percent);
	return result;
}

void Quaternion::FromMatrix(const Matrix44& mat)
{
	float qw2=0.25f*(mat.a.x+mat.b.y+mat.c.z+1.0f);
	float qx2=qw2-0.5f*(mat.b.y+mat.c.z);
	float qy2=qw2-0.5f*(mat.c.z+mat.a.x);
	float qz2=qw2-0.5f*(mat.a.x+mat.b.y);

	// Decide maximum magnitude component
	int i=(qw2>qx2)?
		((qw2>qy2)?((qw2>qz2)?0:3) : ((qy2>qz2)?2:3)) :
	((qx2>qy2)?((qx2>qz2)?1:3) : ((qy2>qz2)?2:3));
	float tmp;

	// Compute signed Quat components using numerically stable method
	switch (i)
	{
	case 0:
		w=sqrtf(qw2); tmp=0.25f/w;
		x=(mat.b.z-mat.c.y)*tmp;
		y=(mat.c.x-mat.a.z)*tmp;
		z=(mat.a.y-mat.b.x)*tmp;
		break;
	case 1:
		x=sqrtf(qx2); tmp=0.25f/x;
		w=(mat.b.z-mat.c.y)*tmp;
		y=(mat.b.x+mat.a.y)*tmp;
		z=(mat.a.z+mat.c.x)*tmp;
		break;
	case 2:
		y=sqrtf(qy2); tmp=0.25f/y;
		w=(mat.c.x-mat.a.z)*tmp;
		x=(mat.b.x+mat.a.y)*tmp;
		z=(mat.c.y+mat.b.z)*tmp;
		break;
	case 3:
		z=sqrtf(qz2); tmp=0.25f/z;
		w=(mat.a.y-mat.b.x)*tmp;
		x=(mat.c.x+mat.a.z)*tmp;
		y=(mat.c.y+mat.b.z)*tmp;
		break;
	}

	// Always keep all components positive
	// (note that scalar*Quat is equivalent to Quat, so q==-q)
	if (i&&w<0.0f)
	{
		w=-w; x=-x; y=-y; z=-z;
	}

	// Normalize it to be safe
	tmp=1.0f/sqrtf(w*w+x*x+y*y+z*z);
	w*=tmp;
	x*=tmp;
	y*=tmp;
	z*=tmp;
}

void Quaternion::AsRotX(float angle) 
{
	float halfAngle = angle * 0.5f;
	x = sinf(halfAngle);
	w = cosf(halfAngle);
	y = z = 0.0f;
}

void Quaternion::AsRotY(float angle) 
{
	float halfAngle = angle * 0.5f;
	y = sinf(halfAngle);
	w = cosf(halfAngle);
	x = z = 0.0f;
}

void Quaternion::AsRotZ(float angle) 
{
	float halfAngle = angle * 0.5f;
	z = sinf(halfAngle);
	w = cosf(halfAngle);
	x = y = 0.0f;
}

//So far seems to have no singularity and return very stable and correct values. 
float Quaternion::GetYAxisRot() const
{
	//this is a portion of the conversion from Quat to Matrix. just the c vector
	Vector3 c;
	float wy, xx, yy, xz, x2, y2, z2;
	x2 = x + x;
	y2 = y + y;
	z2 = z + z;
	xx = x * x2;
	xz = x * z2;
	yy = y * y2;
	wy = w * y2;

	c.x = xz + wy;
	c.y = 0.0f;
	c.z = 1.0f - (xx + yy);

	//normalize. this may be overkill, but safe.
	const float cx2cz2 = c.x * c.x + c.z * c.z;
	if(cx2cz2 > 0.0f)
	{
		float factor = 1.0f / sqrt(cx2cz2);
		c.x *= factor;
		c.z *= factor;
	}
	else
	{
		//pointing straight down y!
		return 0.0f;
	}

	//use the c vector to determine the angle
	return atan2f(c.x, c.z);
}

Vector3 Quaternion::GetZeroRollOrient() const
{
	Vector3 orient;
	Vector3 zeroPoint(0.0f, 1.0f, 0.0f);
	orient = (*this) * zeroPoint;
	return orient;
}

Quaternion Quaternion::Conjugate() const
{
	return Quaternion(-x, -y, -z, w);
}

//for any arbitrary orientation, decompose the portion of rotation about an
//given axis
float Quaternion::GetRotAboutAxis(const Vector3& axis) const
{
	Vector3 thisAxis;
	float thisAngle;
	ToAxisAngle(thisAxis, thisAngle);
	float dif = thisAxis.RadAngleBetween(axis);
	float percentAroundAxis = cosf(dif);
	return percentAroundAxis * thisAngle;
}

//create a quat with the rotation from the xaxis to this orientation
void Quaternion::AsRotationFromXAxis(const Vector3 &normalizedVector) 
{
	if (normalizedVector.x < -0.99999f)
	{
		x = z = w = 0.0f;
		y = 1.0f;
		return;
	}

	x = 0;
	y = -normalizedVector.z * 0.5f;
	z = normalizedVector.y * 0.5f;
	w = (normalizedVector.x + 1.0f) * 0.5f;
	Normalize();
}

Vector3 Quaternion::RotateTheXAxis() const
{
	Vector3 result;
	result.x = w*w + x*x - y*y - z*z;
	result.y = 2*(w*z + x*y);
	result.z = 2*(-w*y + x*z);
	return result;
}

//constrain an oriention to within a limiting polygon and having the maximum twist
Quaternion Quaternion::LimitRotation(Limiting_Polygon *polygon,	float max_axial_angle) const
{
	Vector3 xprime = RotateTheXAxis();

	Quaternion simple;
	simple.AsRotationFromXAxis(xprime);
	Quaternion twist = *this * simple.Conjugate();

	if(polygon)
	{
		polygon->Constrian(xprime);
		xprime.Normalize();
		simple.AsRotationFromXAxis(xprime);
	}
	
	Vector3 axis;
	float angle;
	twist.ToAxisAngle(axis, angle);
	
	if (angle > max_axial_angle) 
		angle = max_axial_angle;

	if (angle < -max_axial_angle) 
		angle = -max_axial_angle;

	twist.FromAxisAngle(axis, angle);

	return (twist * simple);
}

//return the quaternion limited to a cone about the x axis with a given angle
Quaternion Quaternion::LimitRotation(float coneAngle) const
{
	Vector3 x(1, 0, 0);
	Vector3 xprime = Rotate(x);
	float ct = cosf(coneAngle);

	if (xprime.Dot(x) >= ct) 
		return *this;

	Vector3 axis;
	float angle;
	ToAxisAngle(axis, angle);
	
	if (angle < 0) 
		angle += 2 * PI;

	if (angle > PI) 
	{
		axis *= -1.0f;
		angle = -angle + 2 * PI;
	}

	Quaternion result;
	result.FromAxisAngle(axis, coneAngle);
	return result;
}

//start angle is the inner radius, end angle is the outer radius
Quaternion	Quaternion::LimitRotation(float degAngleStartLimit, float degAngleEndLimit, const Quaternion& prev) const
{
	Vector3 x(1, 0, 0);
	Vector3 xprime = Rotate(x);

	float delta = x.RadAngleBetween(xprime);

	if(delta <= degAngleStartLimit)
		return *this;

	float alpha = (delta - degAngleStartLimit) / (degAngleEndLimit - degAngleStartLimit);

	if(alpha > 1.0f)
		alpha = 1.0f;

	if(alpha <= 0.0f)
		return *this;

	Quaternion ret = this->Slerp(prev, alpha);

	return ret;
}

Vector3 Quaternion::Rotate(const Vector3 &vec) const
{
	Quaternion v(vec.x, vec.y, vec.z, 0);
	Quaternion vq_ = v * Conjugate();
	Quaternion qresult = *this * vq_;

	return Vector3(qresult.x, qresult.y, qresult.z);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// class Matrix22

Matrix22::Matrix22()
{
	x0 = 1.0f; x1 = 0.0f;
	y0 = 0.0f; y1 = 1.0f;
}

Matrix22::Matrix22(	const float row0_col0,
					const float row0_col1, 
					const float row1_col0, 
					const float row1_col1)
{
	x0 = row0_col0;
	x1 = row0_col1;
	y0 = row1_col0;
	y1 = row1_col1;
}

float Matrix22::Determinant() const
{
	return (x0 * y1 - x1 * y0); 
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// class Matrix33

Matrix33::Matrix33()
{
	this->Identity();
}

Matrix33::Matrix33(const Matrix33RotFlag rotFlag, const float rot)
{

}

Matrix33::Matrix33(const Matrix43& mtx43)
{
	x0 = mtx43.x0;
	x1 = mtx43.x1;
	x2 = mtx43.x2;

	y0 = mtx43.y0;
	y1 = mtx43.y1;
	y2 = mtx43.y2;

	z0 = mtx43.z0;
	z1 = mtx43.z1;
	z2 = mtx43.z2;
}

Matrix33::Matrix33(const Matrix44& mtx44)
{
	x0 = mtx44.a.x;
	x1 = mtx44.a.y;
	x2 = mtx44.a.z;

	y0 = mtx44.b.x;
	y1 = mtx44.b.y;
	y2 = mtx44.b.z;

	z0 = mtx44.c.x;
	z1 = mtx44.c.y;
	z2 = mtx44.c.z;
}

void Matrix33::Identity()
{
	x0 = 1.0f; x1 = 0.0f; x2 = 0.0f;
	y0 = 0.0f; y1 = 1.0f; y2 = 0.0f;
	z0 = 0.0f; z1 = 0.0f; z2 = 1.0f;	
}

Matrix33& Matrix33::Transposed()
{
	swap(x1, y0); 
	swap(x2, z0);
	swap(y2, z1);

	return(*this);
}

Matrix33 Matrix33::Transpose() const
{
	Matrix33 ret;

	ret.x0 = x0;
	ret.x1 = y0;
	ret.x2 = z0;
	ret.y0 = x1;
	ret.y1 = y1;
	ret.y2 = z1;
	ret.z0 = x2;
	ret.z1 = y2;
	ret.z2 = z2;

	return ret;
}

float	Matrix33::Determinant() const
{
	return (	x0 * Matrix22(y1, y2, z1, z2).Determinant() -
				y0 * Matrix22(y0, z0, y2, z2).Determinant() +
				z0 * Matrix22(x1, x2, y1, y2).Determinant() );
}

Matrix33 Matrix33::Inverse() const
{
	Matrix33 ret;

	//Incomplete!

	return ret;
}

Matrix33	Matrix33::operator*(const Matrix33& m) const
{
	Matrix33 ret;

	//row0 * column 0
	ret.x0 = x0 * m.x0;
	ret.x1 = x1 * m.y0;
	ret.x2 = x2 * m.z0;
	//row1 * column 1
	ret.y0 = y0 * m.x1;
	ret.y1 = y1 * m.y1;
	ret.y2 = y2 * m.z1;
	//row2 * column 2
	ret.z0 = z0 * m.x2;
	ret.z1 = z1 * m.y2;
	ret.z2 = z2 * m.z2;	

	return ret;
}

void Matrix33::FromQuat(const Quaternion& q)
{
	// from Dlawson
	register float qwx, qwy, qwz, qxx, qyy, qyz, qxy, qxz, qzz, qx2, qy2, qz2;

	// calculate coefficients
	qx2 = q.x + q.x;     qy2 = q.y + q.y;     qz2 = q.z + q.z;
	qxx = q.x * qx2;      qxy = q.x * qy2;      qxz = q.x * qz2;
	qyy = q.y * qy2;      qyz = q.y * qz2;      qzz = q.z * qz2;
	qwx = q.w * qx2;      qwy = q.w * qy2;      qwz = q.w * qz2;

	x0 = 1.0f - (qyy + qzz);
	x1 = qxy + qwz;
	x2 = qxz - qwy;

	y0 = qxy - qwz;
	y1 = 1.0f - (qxx + qzz);
	y2 = qyz + qwx;

	z0 = qxz + qwy;
	z1 = qyz - qwx;
	z2 = 1.0f - (qxx + qyy);
}

Vector3		Matrix33::operator*(const Vector3 &v) const
{
	Vector3 res;

	res.x = v.x * x0 + v.y * y0 + v.z * z0;
	res.y = v.x * x1 + v.y * y1 + v.z * z1;
	res.z = v.x * x2 + v.y * y2 + v.z * z2;

	return res;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// class Matrix44

Matrix44 Matrix44::operator*(const Matrix44& m) const
{
	Matrix44 res;
	res.a.x = a.x*m.a.x + a.y*m.b.x + a.z*m.c.x + a.w*m.d.x;
	res.a.y = a.x*m.a.y + a.y*m.b.y + a.z*m.c.y + a.w*m.d.y;
	res.a.z = a.x*m.a.z + a.y*m.b.z + a.z*m.c.z + a.w*m.d.z;
	res.a.w = a.x*m.a.w + a.y*m.b.w + a.z*m.c.w + a.w*m.d.w;

	res.b.x = b.x*m.a.x + b.y*m.b.x + b.z*m.c.x + b.w*m.d.x;
	res.b.y = b.x*m.a.y + b.y*m.b.y + b.z*m.c.y + b.w*m.d.y;
	res.b.z = b.x*m.a.z + b.y*m.b.z + b.z*m.c.z + b.w*m.d.z;
	res.b.w = b.x*m.a.w + b.y*m.b.w + b.z*m.c.w + b.w*m.d.w;

	res.c.x = c.x*m.a.x + c.y*m.b.x + c.z*m.c.x + c.w*m.d.x;
	res.c.y = c.x*m.a.y + c.y*m.b.y + c.z*m.c.y + c.w*m.d.y;
	res.c.z = c.x*m.a.z + c.y*m.b.z + c.z*m.c.z + c.w*m.d.z;
	res.c.w = c.x*m.a.w + c.y*m.b.w + c.z*m.c.w + c.w*m.d.w;

	res.d.x = d.x*m.a.x + d.y*m.b.x + d.z*m.c.x + d.w*m.d.x;
	res.d.y = d.x*m.a.y + d.y*m.b.y + d.z*m.c.y + d.w*m.d.y;
	res.d.z = d.x*m.a.z + d.y*m.b.z + d.z*m.c.z + d.w*m.d.z;
	res.d.w = d.x*m.a.w + d.y*m.b.w + d.z*m.c.w + d.w*m.d.w;

	return res;
}

Vector3 Matrix44::operator*(const Vector3 &v) const
{
	register float lx=v.x, ly=v.y, lz=v.z;
	Vector3 r;
	r.x = lx*a.x + ly*b.x + lz*c.x + d.x;
	r.y = lx*a.y + ly*b.y + lz*c.y + d.y;
	r.z = lx*a.z + ly*b.z + lz*c.z + d.z;
	return r;
}

void Matrix44::Identity()
{
	a.Set (1.0f, 0.0f, 0.0f, 0.0f);
	b.Set (0.0f, 1.0f, 0.0f, 0.0f);
	c.Set (0.0f, 0.0f, 1.0f, 0.0f);
	d.Set (0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix44 Matrix44::Inverse() const // Gauss-Jordan elimination with partial pivoting
{
	Matrix44 a(*this);    // As a evolves from original mat into identity
	Matrix44 b;   // b evolves from identity into inverse(a)
	b.Identity();
/*	int i, j, i1;

	// Loop over cols of a from left to right, eliminating above and below diag
	for (j=0; j<4; j++) 
	{   // Find largest pivot in column j among rows j..3
		i1 = j;		    // Row with largest pivot candidate
		for (i=j+1; i<4; i++)
		{
			if (fabsf(a.v(i)[j]) > fabsf(a.v(i1)[j]))
			{
				i1 = i;
			}
		}

		// Swap rows i1 and j in a and b to put pivot on diagonal
		swap(a.v(i1), a.v(j));
		swap(b.v(i1), b.v(j));

		// Scale row j to have a unit diagonal
		//  if (a.v(j).n(j)==0.)
		//	TRACE("Mat44::inverse: singular matrix; can't invert\n");
		b.v(j) /= a.v(j)[j];
		a.v(j) /= a.v(j)[j];

		// Eliminate off-diagonal elems in col j of a, doing identical ops to b
		for (i=0; i<4; i++)
		{
			if (i!=j)
			{
				b.v(i) -= a.v(i)[j]*(const Vector4 &)b.v(j);
				a.v(i) -= a.v(i)[j]*(const Vector4 &)a.v(j);
			}
		}
	}
	*/
	return b;
}

Vector4& Matrix44::v(int index)
{
	if(index == 0)
		return a;

	if(index == 1)
		return b;

	if(index == 2)
		return c;

	return d;
}

void Matrix44::OpenGLMatrix(float* targetMtx)
{
	for (int j=0; j<4; j++)
	{
		for (int i=0; i<4; i++)
		{
			targetMtx[j+(i*4)] = v(j)[i];
		}
	}
}

void Matrix44::LookAt(const Vector3& vCamera, const Vector3& vTarget)
{
	LookAt(vCamera.x, vCamera.y, vCamera.z, vTarget.x, vTarget.y, vTarget.z, 0.0f, 1.0f, 0.0f);
}

void Matrix44::LookAt(const float eyex, const float eyey, const float eyez, 
				   const float centerx, const float centery, const float centerz, 
				   const float upx, float const upy, float const upz )
{
	Vector3 x, y, z;

	z.x = eyex - centerx;
	z.y = eyey - centery;
	z.z = eyez - centerz;
	z.Normalize();

	y.x = upx;
	y.y = upy;
	y.z = upz;

	x = y.Cross(z);
	x.Normalize();

	y = z.Cross(x);
	y.Normalize();

	a.Set(x.x,x.y,x.z,0.0f);
	b.Set(y.x,y.y,y.z,0.0f);
	c.Set(z.x,z.y,z.z,0.0f);

	Vector3 vO(-eyex,-eyey,-eyez);
	d.Set(x.Dot(vO),y.Dot(vO),z.Dot(vO), 1.0f);
}

void Matrix44::ContructViewMtx(float fFocalLength, float fAspect, float fNear, float fFar)
{
	float fTop = fNear;
	float fBottom = -fTop;
	float fRight = fTop*fAspect;
	float fLeft = -fRight;

	a.x =(2.0f*fNear)/(fRight-fLeft); 
	a.y = 0.0f;           
	a.z =(fRight+fLeft)/(fRight-fLeft);   
	a.w = 0.0f;

	b.x = 0.0f;
	b.y = (2.0f*fNear)/(fTop-fBottom); 
	b.z = (fTop+fBottom)/(fTop-fBottom);   
	b.w = 0.0f;

	c.x = 0.0f;           
	c.y = 0.0f;
	c.z = -(fFar+fNear)/(fFar-fNear); 
	c.w = (-2.0f*fFar*fNear)/(fFar-fNear);

	d.x = 0.0f;
	d.y = 0.0f;
	d.z = -1.0f;           
	d.w = 0.0f;
}

void Matrix44::FromQuat(const Quaternion& q)
{
	register float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	// calculate coefficients
	x2 = q.x + q.x;     y2 = q.y + q.y;     z2 = q.z + q.z;
	xx = q.x * x2;      xy = q.x * y2;      xz = q.x * z2;
	yy = q.y * y2;      yz = q.y * z2;      zz = q.z * z2;
	wx = q.w * x2;      wy = q.w * y2;      wz = q.w * z2;

	a.x = 1.0f - (yy + zz);
	a.y = xy + wz;
	a.z = xz - wy;
	a.w = 0.0f;

	b.x = xy - wz;
	b.y = 1.0f - (xx + zz);
	b.z = yz + wx;
	b.w = 0.0f;

	c.x = xz + wy;
	c.y = yz - wx;
	c.z = 1.0f - (xx + yy);
	c.w = 0.0f;

	d.x = 0.0f; 
	d.y = 0.0f;
	d.z = 0.0f;
	d.w = 1.0f;
}

void Matrix44::Normalize()
{
	a.Normalize();
	b.Normalize();
	c.Normalize();
}

void Matrix44::SetPos(const Vector3& p)
{
	d.x = p.x;
	d.y = p.y;
	d.z = p.z;
}

Vector4&	Matrix44::GetPos()
{
	return d;
}

void Matrix44::Scale(const float uniformScale)
{
	a *= uniformScale;
	b *= uniformScale;
	c *= uniformScale;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//class Limiting_Polygon
//an N sided polygon used to constrain operations
//this is a 2d polygon limited to the x = 1 plane

Limiting_Polygon::Limiting_Polygon()
{
	m_NumVertices = 0;
	m_pVertices = 0;
}

Limiting_Polygon::~Limiting_Polygon()
{
	if(m_pVertices)
		delete []m_pVertices;
}

void Limiting_Polygon::Init(int nVertices)
{
	if(m_pVertices)
		delete []m_pVertices;

	m_NumVertices = nVertices;
	m_pVertices = new Vector3[nVertices];
}

int Limiting_Polygon::GetNumVerts() 
{ 
	return m_NumVertices; 
}

const Vector3& Limiting_Polygon::GetVert(int iVert) 
{ 
	return m_pVertices[iVert % m_NumVertices]; 
}

void Limiting_Polygon::SetVertex(int iVert, const Vector3& vert)
{
	if(!m_pVertices || iVert >= m_NumVertices)
		return;

	m_pVertices[iVert] = vert;
	m_pVertices[iVert].x = 1.0f;
}

// Stuff below forces an IK joint to stay within the limiting
// polygon.  It assumes the polygon has the x axis as its surface
// normal.  To handle multiple polygons that are not coplanar,
// (so that you can have joint limits that span more than one
// hemisphere), this code will have to be generalized a little.
// It is not hard to do though.  I was going to do it but ran
// out of time.
Vector3 Limiting_Polygon::IntersectWithPlane(const Vector3& vector) 
{
	Vector3 result = vector;

	const float YZ_FAR = 1000.0f;
	const float X_MIN = 0.01f;
	if (result.x < X_MIN) {
		result.x = 1;
		result.y *= YZ_FAR;
		result.z *= YZ_FAR;
	}

	float factor = 1.0f / result.x;
	result = result * factor;

	return result;
}

void Limiting_Polygon::Constrian(Vector3& vec)
{
	if (m_NumVertices == 0) return;

	float best_distance2 = 1.0E38f;
	Vector3 target = IntersectWithPlane(vec);

	int i;
	for (i = 0; i < m_NumVertices; i++) {
		const Vector3& v0 = m_pVertices[i];
		const Vector3& v1 = m_pVertices[(i + 1) % m_NumVertices];

		float dx = v1.y - v0.y;
		float dy = v1.z - v0.z;

		float rx = target.y - v0.y;
		float ry = target.z - v0.z;

		float dot = -dy * rx + dx * ry;
		if (dot >= 0) continue;        

		// Find the projection of the target point onto this line
		// (clamping at the enpoint of the line segments).

		float l_dot_l = dx*dx + dy*dy;

		float proj_x, proj_y;
		float factor = (rx * dx + ry * dy) / l_dot_l;
		proj_x = factor * dx;
		proj_y = factor * dy;

		float j_dot_j = proj_x*proj_x + proj_y*proj_y;
		float j_dot_l = proj_x*dx + proj_y*dy;

		if (j_dot_j > l_dot_l) 
		{
			proj_x = v1.y - v0.y;
			proj_y = v1.z - v0.z;
		} 
		else if (j_dot_l < 0) 
		{
			proj_x = 0;
			proj_y = 0;
		}

		float dist_dx, dist_dy;
		dist_dx = rx - proj_x;
		dist_dy = ry - proj_y;

		float dist2 = dist_dx*dist_dx + dist_dy*dist_dy;
		if (dist2 < best_distance2) 
		{
			best_distance2 = dist2;
			vec.x = target.x;
			vec.y = proj_x + v0.y;
			vec.z = proj_y + v0.z;

			vec = IntersectWithPlane(vec);
		}
	}
}

//////////////////////////////////////////////////////////////
// class LineSeg2d

LineSeg2d::LineSeg2d()
{
	memset(this, 0, sizeof(LineSeg2d));
}

LineSeg2d::LineSeg2d(const Vector2& a, const Vector2& b)
{
	Construct(a, b);
}

//allow a line to be recomputed
void LineSeg2d::Construct(const Vector2& a, const Vector2& b)
{
	m_point = a;
	m_end = b;
	m_ray = a - b;
	m_ray.Normalize();
}

//produce a vector normal to this line passing through this point.
Vector2 LineSeg2d::ClosestVectorTo(const Vector2& point, LineSegResult& offEnd)
{
	Vector2 deltaPoint = m_point - point;
	float dot = deltaPoint.Dot(m_ray);

	Vector2 deltaPointEnd = m_end - point;
	Vector2 negRay = m_ray * -1;
	float dotEnd = deltaPointEnd.Dot(negRay);

	if (dot < 0.0f)
		offEnd = eOffEndA;
	else if (dotEnd < 0.0f)
		offEnd = eOffEndB;
	else
		offEnd = eOnLine;

	if(offEnd != eOnLine)
		return point;

	return (m_ray * dot) - deltaPoint;
}

//transform the point by the normal vector that places it on the line
Vector2 LineSeg2d::ClosestPointOnLineTo(const Vector2& point, LineSegResult& offEnd)
{
	Vector2 vectorTo = ClosestVectorTo(point, offEnd);
	return point + vectorTo;
}

float LineSeg2d::DistSqrdFrom(const Vector2& point)
{
	LineSegResult offEnd = eOnLine;

	Vector2 p = ClosestPointOnLineTo(point, offEnd);
	
	p -= point;
	
	return p.Mag2();
}

//take the triangle defined by the points a, b, and c. Find the barry centric coord BCC for the point p.
//return false if off the triangle.
bool Tri2d::CalcBarryCentricCoords(const Vector2& a, const Vector2& b, const Vector2& c, 
									const Vector2& p, Vector3& BCC)
{
	float b0 =  (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);

	if(b0 == 0.0f)
		return false;

	BCC.x = ((b.x - p.x) * (c.y - p.y) - (c.x - p.x) * (b.y - p.y)) / b0;
	BCC.y = ((c.x - p.x) * (a.y - p.y) - (a.x - p.x) * (c.y - p.y)) / b0;
	BCC.z = ((a.x - p.x) * (b.y - p.y) - (b.x - p.x) * (a.y - p.y)) / b0;
	return (BCC.x > 0.0f && BCC.y > 0.0f && BCC.z > 0.0f);
}

//same as above but uses this data
bool Tri2d::CalcBarryCentricCoords(const Vector2& p, Vector3& BCC)
{
	const Vector2& a = m_Corners[0];
	const Vector2& b = m_Corners[1];
	const Vector2& c = m_Corners[2];

	float b0 =  (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);

	if(b0 == 0.0f)
		return false;

	BCC.x = ((b.x - p.x) * (c.y - p.y) - (c.x - p.x) * (b.y - p.y)) / b0;
	BCC.y = ((c.x - p.x) * (a.y - p.y) - (a.x - p.x) * (c.y - p.y)) / b0;
	BCC.z = ((a.x - p.x) * (b.y - p.y) - (b.x - p.x) * (a.y - p.y)) / b0;

	return (BCC.x > 0.0f && BCC.y > 0.0f && BCC.z > 0.0f);
}

void Quad2d::UpdateRays()
{
	for (int iP = 0; iP < 4; iP++)
	{
		const Vector2& pA = m_Corners[iP];
		const Vector2& pB = m_Corners[(iP + 1)%4];
		Vector2& ray = m_Rays[iP];
		ray = (pA - pB);
		ray.Normalize();
	}
}

bool Quad2d::CalcBarryCentricCoords(const Vector2& p, Vector3& BCC)
{
	bool intersect = Tri2d::CalcBarryCentricCoords(m_Corners[0], m_Corners[1], m_Corners[2], p, BCC);

	if(!intersect)
		intersect = Tri2d::CalcBarryCentricCoords(m_Corners[2], m_Corners[3], m_Corners[0], p, BCC);

	return intersect;
}

bool Quad2d::Contains(const Vector2& point)
{
	Vector3 BCC;
	return CalcBarryCentricCoords(point, BCC);
}

Plane::Plane()
{
	m_Pos = Vector3(0.0f, 0.0f, 0.0f);
	m_Normal = Vector3(1.0f, 0.0f, 0.0f);
}

Plane::Plane(const Vector3& p, const Vector3& n)
{
	Construct(p, n);
}

void Plane::operator*=(const Matrix33& rot)
{
	m_Normal = rot * m_Normal;
}

void Plane::Construct(const Vector3& p, const Vector3& n)
{
	m_Pos = p;
	m_Normal = n;
	//why can't I normalize this!!! It screws up the intersect!!!
	//m_Normal.Normalize();
	m_D = -(p.Dot(n));
}

bool Plane::Intersect(const Ray& ray, Vector3& intersection) const
{
	float fNormDotDir = ray.m_dir.Dot(m_Normal);

	// Compute the parametric term t
	float t = -(m_D + (m_Normal.Dot(ray.m_point)))/(fNormDotDir);

	//neg dir of ray, so it's behind the point.
	if (t < 0.0f)
		return false;

	// The plane intersection
	intersection = ray.m_point + ( ray.m_dir * t );

	return true;
}

bool Plane::Intersect(const Line& line, Vector3& intersection) const
{
	float fNormDotDir = line.m_dir.Dot(m_Normal);

	// Compute the parametric term t
	float t = -(m_D + (m_Normal.Dot(line.m_point)))/(fNormDotDir);

	// The plane intersection
	intersection = line.m_point + ( line.m_dir * t );

	return true;
}

void LineSeg::Construct(const Vector3& from, const Vector3& to)
{
	m_point = from;
	m_end = to;
	m_dir = to - from;
	m_dir.Normalize();
}
