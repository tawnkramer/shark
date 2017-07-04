// TMath.h
//
// Author : Tawn Kramer
// Nov 12, 2003

#ifndef __T_MATH__
#define __T_MATH__

namespace TMath
{
	class Vector2;
	class Vector3;
	class Vector4;
	class Matrix22;
	class Matrix33;
	class Matrix43;
	class Matrix44;
	class Quaternion;
	class EulerAngles;
	class Ray;
	class Plane;
	class Tri;
	class Quad;
	class Sphere;
	class LineSeg;
	class Square;
	class Rect;
	class Cube;
	class Box;
	class Limiting_Polygon;
	class LineSeg2d;
	class Quad2d;

	extern const double dPI;
	extern const float PI;

	inline float	RadToDeg(float x)	{ return ((x) / PI * 180.0f); }
	inline float	DegToRad(float x)	{ return ((x) / 180.0f * PI); }
	inline double	RadToDeg(double x)	{ return ((x) / dPI * 180.0f); }
	inline double	DegToRad(double x)	{ return ((x) / 180.0f * dPI); }

	//Misc
	template <class T>
	void swap(T& a, T& b)
	{
		T temp = a;
		a = b;
		b = temp;
	}

	template <class T>
	T clamp(T& a, const T& lo, const T& hi)
	{
		if( a < lo)
			return lo;

		if(a > hi)
			return hi;

		return a;
	}

	class Vector2
	{
	public:
		Vector2();
		Vector2(float x, float y);
		Vector2(const Vector2& v);
		Vector2(const Vector3& v);
		Vector2(const Vector4& v);
		Vector2& operator=(const Vector2& v);
		Vector2& operator=(const Vector3& v);
		Vector2& operator=(const Vector4& v);
		Vector2	operator+(const Vector2& v) const;
		Vector2	operator-(const Vector2& v) const;		
		Vector2	operator*(const float scalar) const;
		Vector2	operator/(const float scalar) const;
		Vector2&	operator+=(const Vector2& v);
		Vector2&	operator-=(const Vector2& v);
		Vector2&	operator*=(const float scalar);
		Vector2&	operator/=(const float scalar);
		bool		operator==(const Vector2& v) const;
		float	operator[](const int i) const;
		float	Dot(const Vector2& v) const;
		float	Normalize();
		float	Mag() const;
		float	Mag2() const;
		float	Dist(const Vector2& v) const;
		float	Cross(const Vector2& v) const;
		float	RadAngleBetween(const Vector2& v) const;
		float	DegAngleBetween(const Vector2& v) const;

		//data
		float x, y;
	};

	class Vector3
	{
	public:
		Vector3();
		Vector3(float x, float y, float z);
		Vector3(const Vector2& v);
		Vector3(const Vector3& v);
		Vector3(const Vector4& v);
		Vector3& operator=(const Vector2& v);
		Vector3& operator=(const Vector3& v);
		Vector3& operator=(const Vector4& v);		
		Vector3	operator+(const Vector3& v) const;
		Vector3	operator-(const Vector3& v) const;
		Vector3	operator*(const Vector3& v) const;
		Vector3	operator/(const float scalar) const;
		Vector3	operator*(const float scalar) const;
		Vector3&	operator+=(const Vector3& v);
		Vector3&	operator-=(const Vector3& v);
		Vector3&	operator*=(const float scalar);
		Vector3&	operator/=(const float scalar);
		bool		operator==(const Vector3& v) const;
		bool		operator!=(const Vector3& v) const;
		float	operator[](const int i) const;
		float&	operator[](const int i);
		float	Dot(const Vector3& v) const;
		Vector3	Cross(const Vector3& v) const;
		float	Normalize();
		float	Mag() const;
		float	Mag2() const;
		float	Dist(const Vector3& v) const;
		float	RadAngleBetween(const Vector3& v) const;
		float	DegAngleBetween(const Vector3& v) const;
		Quaternion DeltaRotBetween(const Vector3& v) const;
		Vector3	Lerp(const Vector3&, float percent) const;

		//data
		float x, y, z;
	};

	class Vector4
	{
	public:
		Vector4();
		Vector4(float x, float y, float z, float w);
		Vector4(const Vector2& v);
		Vector4(const Vector3& v);
		Vector4(const Vector4& v);

		Vector4	operator+(const Vector4& v) const;
		Vector4	operator-(const Vector4& v) const;
		Vector4	operator*(const Vector4& v) const;
		Vector4	operator*(const float scalar) const;
		Vector4	operator/(const float scalar) const;
		Vector4&	operator+=(const Vector4& v);
		Vector4&	operator-=(const Vector4& v);
		Vector4&	operator*=(const float scalar);
		Vector4&	operator/=(const float scalar);
		bool		operator==(const Vector4& v) const;
		float	operator[](const int i) const;
		float	Dot(const Vector4& v) const;
		float	Normalize();
		float	Mag() const;
		void	Set(float x, float y, float z, float w);
		void	Identity() { Set(0.0f, 0.0f, 0.0f, 1.0f); }

		//data
		float x, y, z, w;
	};

	class Quaternion : public Vector4
	{
	public:

		Quaternion();
		Quaternion(float x, float y, float z, float w);
		Quaternion	operator*(const Quaternion& quat) const;
		Quaternion	operator*(const float scalar) const;
		Vector3		operator*(const Vector3& vec) const;
		void		operator*=(const Quaternion& quat);
		Quaternion  operator-(const Quaternion& quat) const;
		Quaternion	Lerp(const Quaternion& to, float percent) const;
		Quaternion	Slerp(const Quaternion& to, float percent) const;
		Quaternion	CatmullRom(const Quaternion &Q00, const Quaternion &Q01, const Quaternion &Q02, const Quaternion &Q03, float percent) const;
		void		ToAxisAngle(Vector3& axis, float& angle) const;
		void		FromAxisAngle(const Vector3& axis, const float& angle);
		Quaternion  Inverse() const;
		Quaternion	Conjugate() const;
		void		FromMatrix(const Matrix44& mtx);
		void		AsRotX(float angle);
		void		AsRotY(float angle);
		void		AsRotZ(float angle);
		float		GetYAxisRot() const;
		Vector3		GetZeroRollOrient() const;
		float		GetRotAboutAxis(const Vector3& axis) const;
		//from Jonnathon blows routines to limit the ik joint
		void		AsRotationFromXAxis(const Vector3 &normalizedVector);
		Vector3		RotateTheXAxis() const;
		Quaternion	LimitRotation(Limiting_Polygon *polygon,	float max_axial_angle) const;
		Quaternion	LimitRotation(float coneAngle) const;
		Quaternion	LimitRotation(float startLimit, float endLimit, const Quaternion& prev) const;
		Vector3		Rotate(const Vector3 &vector) const;

	};

	class Matrix22
	{
	public:
		Matrix22();
		Matrix22(	const float row0_col0,
					const float row0_col1, 
					const float row1_col0, 
					const float row1_col1);

		float Determinant() const;

		//data: row then column
		float x0, x1;
		float y0, y1;
	};

	class Matrix33
	{
	public:
		enum Matrix33RotFlag
		{
			cRotXRad,
			cRotYRad,
			cRotZRad,

			cRotXDeg,
			cRotYDeg,
			cRotZDeg,
		};

		Matrix33();
		Matrix33(const Matrix33RotFlag rotFlag,const float rot);
		Matrix33(const Matrix43& mtx43);
		Matrix33(const Matrix44& mtx44);

		void		Identity();
		Matrix33&	Transposed();
		Matrix33	Transpose() const;
		Matrix33	Inverse() const;
		Matrix33	operator*(const Matrix33& mtx33) const;
		float		Determinant() const;
		void		FromQuat(const Quaternion& q);
		Vector3		operator*(const Vector3 &directionVector) const;

		//data
		float x0, x1, x2;
		float y0, y1, y2;
		float z0, z1, z2;
	};

	class Matrix43
	{
	public:

		//data
		float x0, x1, x2, x3;
		float y0, y1, y2, y3;
		float z0, z1, z2, z3;
	};

	class Matrix44
	{
	public:

		Matrix44	operator*(const Matrix44&) const;
		Vector3		operator*(const Vector3 &v) const;
		Matrix44	Inverse() const;
		void		Identity();
		void		FromQuat(const Quaternion& q);
		void		ContructViewMtx(float fFocalLength, float fAspect, float fNear, float fFar);
		void		OpenGLMatrix(float* targetMtx);
		void		LookAt(const Vector3& vCamera, const Vector3& vTarget);
		void		LookAt(const float eyex, const float eyey, const float eyez, 
							const float centerx, const float centery, const float centerz, 
							const float upx, float const upy, float const upz );
		void		Normalize();
		Vector4&	v(int index);
		void		SetPos(const Vector3& p);
		Vector4&	GetPos();
		void		Scale(const float uniformScale);

		//data
		Vector4 a, b, c, d;
	};

	class Tri
	{
	public:
		Vector3 m_Points[3];
	};

	class Quad
	{
	public:
		Vector3 m_Points[3];
	};

	class IntersectionInfo
	{
	public:
		Vector3 hitPos;
		Vector3 normal;
	};

	class Line
	{
	public:
		//position and normalized dir.
		Vector3 m_point, m_dir;
	};

	class Ray : public Line
	{
	public:
	};

	class LineSeg : public Ray
	{
	public:
		void Construct(const Vector3& from, const Vector3& to);
		Vector3 m_end;
	};

	class Sphere
	{
	public:
		Vector3 center;
		float	radius;
	};

	class Rect
	{
	public:
		float left, right, top, bottom;
	};

	class Plane
	{
	public:
		Plane();
		Plane(const Vector3& p, const Vector3& n);
		void Construct(const Vector3& p, const Vector3& n);
		bool Intersect(const Ray& ray, Vector3& intersection) const;
		bool Intersect(const Line& ray, Vector3& intersection) const;
		void operator*=(const Matrix33& rot);

		Vector3 m_Normal, m_Pos;
		float m_D;
	};

	//axis aligned box, useful for bounds tests
	class AABox
	{
	public:
		Vector3 m_Min;
		Vector3 m_Max;
	};

	//an N sided 2d polygon used to constrain operations
	class Limiting_Polygon
	{
	public:
		Limiting_Polygon();
		~Limiting_Polygon();
		void Init(int nVertices);
		void SetVertex(int iVert, const Vector3& vert);
		int GetNumVerts();
		const Vector3& GetVert(int iVert);
		void Constrian(Vector3& vec);

	protected:

		Vector3 IntersectWithPlane(const Vector3& vector);

		Vector3*	m_pVertices;
		int			m_NumVertices;
	};

	class LineSeg2d
	{
	public:
		
		enum LineSegResult
		{
			eOnLine,
			eOffEndB,
			eOffEndA,
		};

		LineSeg2d();
		LineSeg2d(const Vector2& a, const Vector2& b);
		void	Construct(const Vector2& a, const Vector2& b);
		Vector2 ClosestVectorTo(const Vector2& point, LineSegResult& offEnd);
		Vector2 ClosestPointOnLineTo(const Vector2& point, LineSegResult& offEnd);
		float	DistSqrdFrom(const Vector2& point);

		Vector2 m_point, m_ray, m_end;
	};

	class Tri2d
	{
	public:
		bool CalcBarryCentricCoords(const Vector2& p, Vector3& BCC);
		//returns false if outside of tri
		static bool CalcBarryCentricCoords(const Vector2& a, const Vector2& b, const Vector2& c, 
			const Vector2& p, Vector3& BCC);
		Vector2 m_Corners[3];
	};

	class Quad2d
	{
	public:
		void UpdateRays();//call after corners change.

		//returns false if outside of tri
		bool CalcBarryCentricCoords(const Vector2& p, Vector3& BCC);
		bool Contains(const Vector2& p);
		Vector2 m_Corners[4];
		Vector2 m_Rays[4];
	};
	
};

struct BitMask
{
	BitMask() { m_Mask = 0;	}

	void SetABit(int iBit) 
	{
		m_Mask |= (1 << iBit);
	}

	bool TestABit(int iBit)
	{
		return ((m_Mask & (1 << iBit)) != 0);
	}

	unsigned long m_Mask;
};

#endif //__T_MATH__