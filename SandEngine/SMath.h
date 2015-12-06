#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <cassert>

struct SVector2
{
	SVector2()
	{}
	SVector2(const float x, const float y)
		:x(x), y(y)
	{}

	float x = 0.0f, y = 0.0f;
};

struct SVector3
{
	SVector3()
	{}
	SVector3(const float x, const float y, const float z)
		:x(x), y(y), z(z)
	{}

	float x = 0.0f, y = 0.0f, z = 0.0f;

	SVector3 operator+(const SVector3& rhs) const
	{
		return SVector3(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	SVector3& operator+=(const SVector3& rhs)
	{
		*this = *this + rhs;
		return *this;
	}

	SVector3 operator-(const SVector3& rhs) const
	{
		return SVector3(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	SVector3& operator-=(const SVector3& rhs)
	{
		*this = *this - rhs;
		return *this;
	}

	SVector3 operator*(const float& t) const
	{
		return SVector3(x*t, y*t, z*t);
	}

	SVector3 operator/(const float& t) const
	{
		return SVector3(x / t, y / t, z / t);
	}

	float& operator[](const unsigned int index)
	{
		assert(0 <= index && index <= 2);
		switch (index)
		{
		case 0: return x;
		case 1: return y;
		case 2: return z;
		}
	}
	
	float squaredSize() const
	{
		return x*x + y*y + z*z;
	}

	float size() const
	{
		return sqrt(squaredSize());
	}

	SVector3 normal() const
	{
		return *this / size();
	}

	void normalize() &
	{
		*this = normal();
	}
};

struct alignas(16) SVector4
{
	SVector4()
	{}
	SVector4(const float x, const float y, const float z, const float w)
		:x(x), y(y), z(z), w(w)
	{}
	SVector4(const SVector3 vec3, const float w)
		:x(vec3.x), y(vec3.y), z(vec3.z), w(w)
	{}

	float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

	SVector4& operator=(const SVector3& vec3)
	{
		x = vec3.x;
		y = vec3.y;
		z = vec3.z;
		return *this;
	}

	SVector4 operator+(const SVector4& rhs) const
	{
		return SVector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
	}

	SVector4& operator+=(const SVector4& rhs)
	{
		*this = *this + rhs;
		return *this;
	}

	SVector4 operator-(const SVector4& rhs) const
	{
		return SVector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
	}

	SVector4& operator-=(const SVector4& rhs)
	{
		*this = *this - rhs;
		return *this;
	}

	SVector4 operator*(const float& t) const
	{
		return SVector4(x*t, y*t, z*t, w*t);
	}

	SVector4 operator/(const float& t) const
	{
		return SVector4(x/t, y/t, z/t, w/t);
	}

	float& operator[](const unsigned int index)
	{
		assert(0 <= index && index <= 3);
		switch (index)
		{
		case 0: return x;
		case 1: return y;
		case 2: return z;
		case 3: return w;
		}
	}

	float squaredSize() const
	{
		return x*x + y*y + z*z + w*w;
	}

	float size() const
	{
		return sqrt(squaredSize());
	}

	SVector4 normal() const
	{
		return *this / size();
	}

	void normalize() &
	{
		*this = normal();
	}
};

SVector3 Cross(const SVector3& a, const SVector3& b);
float Dot(const SVector3& a, const SVector3& b);
float Dot(const SVector4& a, const SVector4& b);

struct alignas(16) SMatrix
{
	//Column Major Matrix
	//[0][0] [1][0] [2][0] [3][0]
	//[0][1] [1][1] [2][1] [3][1]
	//[0][2] [1][2] [2][2] [3][2]
	//[0][3] [1][3] [2][3] [3][3]

	SVector4 m[4];

	static const SMatrix Identity;

	SMatrix::SMatrix()
	{

	}

	SMatrix::SMatrix(const SVector3& X_Axis, const SVector3& Y_Axis, const SVector3& Z_Axis)
	{
		m[0] = SVector4(X_Axis, 0.0f);
		m[1] = SVector4(Y_Axis, 0.0f);
		m[2] = SVector4(Z_Axis, 0.0f);
		m[3] = SVector4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	SVector4 operator *(SVector4& rhs)
	{
		SVector4 result;
		result.x = m[0][0] * rhs.x + m[1][0] * rhs.y + m[2][0] * rhs.z + m[3][0] * rhs.z;
		result.y = m[0][1] * rhs.x + m[1][1] * rhs.y + m[2][1] * rhs.z + m[3][1] * rhs.z;
		result.z = m[0][2] * rhs.x + m[1][2] * rhs.y + m[2][2] * rhs.z + m[3][2] * rhs.z;
		result.w = m[0][3] * rhs.x + m[1][3] * rhs.y + m[2][3] * rhs.z + m[3][3] * rhs.z;

		return result;
	}

	SMatrix operator *(SMatrix& rhs)
	{
		SMatrix mat;
		mat.m[0][0] = m[0][0] * rhs.m[0][0] + m[1][0] * rhs.m[0][1] + m[2][0] * rhs.m[0][2] + m[3][0] * rhs.m[0][3];
		mat.m[1][0] = m[0][0] * rhs.m[1][0] + m[1][0] * rhs.m[1][1] + m[2][0] * rhs.m[1][2] + m[3][0] * rhs.m[1][3];
		mat.m[2][0] = m[0][0] * rhs.m[2][0] + m[1][0] * rhs.m[2][1] + m[2][0] * rhs.m[2][2] + m[3][0] * rhs.m[2][3];
		mat.m[3][0] = m[0][0] * rhs.m[3][0] + m[1][0] * rhs.m[3][1] + m[2][0] * rhs.m[3][2] + m[3][0] * rhs.m[3][3];

		mat.m[0][1] = m[0][1] * rhs.m[0][0] + m[1][1] * rhs.m[0][1] + m[2][1] * rhs.m[0][2] + m[3][1] * rhs.m[0][3];
		mat.m[1][1] = m[0][1] * rhs.m[1][0] + m[1][1] * rhs.m[1][1] + m[2][1] * rhs.m[1][2] + m[3][1] * rhs.m[1][3];
		mat.m[2][1] = m[0][1] * rhs.m[2][0] + m[1][1] * rhs.m[2][1] + m[2][1] * rhs.m[2][2] + m[3][1] * rhs.m[2][3];
		mat.m[3][1] = m[0][1] * rhs.m[3][0] + m[1][1] * rhs.m[3][1] + m[2][1] * rhs.m[3][2] + m[3][1] * rhs.m[3][3];

		mat.m[0][2] = m[0][2] * rhs.m[0][0] + m[1][2] * rhs.m[0][1] + m[2][2] * rhs.m[0][2] + m[3][2] * rhs.m[0][3];
		mat.m[1][2] = m[0][2] * rhs.m[1][0] + m[1][2] * rhs.m[1][1] + m[2][2] * rhs.m[1][2] + m[3][2] * rhs.m[1][3];
		mat.m[2][2] = m[0][2] * rhs.m[2][0] + m[1][2] * rhs.m[2][1] + m[2][2] * rhs.m[2][2] + m[3][2] * rhs.m[2][3];
		mat.m[3][2] = m[0][2] * rhs.m[3][0] + m[1][2] * rhs.m[3][1] + m[2][2] * rhs.m[3][2] + m[3][2] * rhs.m[3][3];

		mat.m[0][3] = m[0][3] * rhs.m[0][0] + m[1][3] * rhs.m[0][1] + m[2][3] * rhs.m[0][2] + m[3][3] * rhs.m[0][3];
		mat.m[1][3] = m[0][3] * rhs.m[1][0] + m[1][3] * rhs.m[1][1] + m[2][3] * rhs.m[1][2] + m[3][3] * rhs.m[1][3];
		mat.m[2][3] = m[0][3] * rhs.m[2][0] + m[1][3] * rhs.m[2][1] + m[2][3] * rhs.m[2][2] + m[3][3] * rhs.m[2][3];
		mat.m[3][3] = m[0][3] * rhs.m[3][0] + m[1][3] * rhs.m[3][1] + m[2][3] * rhs.m[3][2] + m[3][3] * rhs.m[3][3];

		return mat;
	}

	void Translation(SVector3 location)
	{
		m[3].x = location.x;
		m[3].y = location.y;
		m[3].z = location.z;
	}
};

struct SQuaternion
{
	float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

	SQuaternion operator*(const SQuaternion& rhs)
	{
		SQuaternion quat;
		quat.x = w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y;
		quat.y = w * rhs.y - x * rhs.z + y * rhs.w - z * rhs.x;
		quat.z = w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w;
		quat.w = w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z;

		return quat;
	}

	SMatrix RotationMatrix() const
	{
		SMatrix mat;
		mat.m[0][0] = w*w + x*x - y*y - z*z;	mat.m[1][0] = 2 * x * y - w * w * z;	mat.m[2][0] = 2 * x * z + 2 * w * y;	mat.m[3][0] = 0.0f;
		mat.m[0][1] = 2 * x * y + w * w * z;	mat.m[1][1] = w*w - x*x + y*y - z*z;	mat.m[2][1] = 2 * y * z + 2 * w * x;	mat.m[3][1] = 0.0f;
		mat.m[0][2] = 2 * x * z - 2 * w * y;	mat.m[1][2] = 2 * y * z - 2 * w * x;	mat.m[2][2] = w*w - x*x - y*y + z*z;	mat.m[3][2] = 0.0f;
		mat.m[0][3] = 0.0f;				 		mat.m[1][3] = 0.0f;						mat.m[2][3] = 0.0f;						mat.m[3][3] = 1.0f;

		return mat;
	}

	auto SizeSquared() const
	{
		return x*x + y*y + z*z + w*w;
	}

	auto Size() const
	{
		return sqrt(SizeSquared());
	}
};

extern SMatrix MatrixPerspectiveFOV(float FovAngleY,float AspectRatio,float NearZ,float FarZ);
extern SMatrix MatrixFrustum(float left, float right, float bottom, float top, float zNear, float zFar);
extern SMatrix MatrixLookAt(const SVector3& eye, const SVector3& lookAt, const SVector3& up);

inline void ScalarSinCos
(
	float* pSin,
	float* pCos,
	float  Value
	)
{
	assert(pSin);
	assert(pCos);

	static constexpr double SM_1DIV2PI = 1.0 / 2.0 * M_PI;
	static constexpr double SM_2PI = M_PI * 2.0;
	static constexpr double SM_PIDIV2 = M_PI / 2.0;

	// Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
	float quotient = SM_1DIV2PI * Value;
	if (Value >= 0.0f)
	{
		quotient = (float)((int)(quotient + 0.5f));
	}
	else
	{
		quotient = (float)((int)(quotient - 0.5f));
	}
	float y = Value - SM_2PI*quotient;

	// Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
	float sign;
	if (y > SM_PIDIV2)
	{
		y = M_PI - y;
		sign = -1.0f;
	}
	else if (y < -SM_PIDIV2)
	{
		y = -M_PI - y;
		sign = -1.0f;
	}
	else
	{
		sign = +1.0f;
	}

	float y2 = y * y;

	// 11-degree minimax approximation
	*pSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

	// 10-degree minimax approximation
	float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
	*pCos = sign*p;
}