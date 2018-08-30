#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <cassert>
#include <algorithm>

#include "Utils.h"

#define S_PI 3.14159265358979323846

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

	const float& operator[](const unsigned int index) const
	{
		assert(0 <= index && index <= 2);
		switch (index)
		{
		case 0: return x;
		case 1: return y;
		case 2: return z;
		}
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

struct SVector4
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
		w = 0.0f;
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

	const float& operator[](const unsigned int index) const
	{
		assert(0 <= index && index <= 3);
		switch (index)
		{
		case 0: return x;
		case 1: return y;
		case 2: return z;
		case 3: return w;
		}
		return w;
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
		return w;
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

struct SQuaternion;
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
		m[0][0] = 1.0f;
		m[1][1] = 1.0f;
		m[2][2] = 1.0f;
		m[3][3] = 1.0f;
	}

	SMatrix::SMatrix(const SVector3& X_Axis, const SVector3& Y_Axis, const SVector3& Z_Axis)
	{
		m[0] = SVector4(X_Axis, 0.0f);
		m[1] = SVector4(Y_Axis, 0.0f);
		m[2] = SVector4(Z_Axis, 0.0f);
		m[3] = SVector4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	SVector4 operator *(const SVector4& rhs) const
	{
		SVector4 result;
		result.x = m[0][0] * rhs.x + m[1][0] * rhs.y + m[2][0] * rhs.z + m[3][0] * rhs.z;
		result.y = m[0][1] * rhs.x + m[1][1] * rhs.y + m[2][1] * rhs.z + m[3][1] * rhs.z;
		result.z = m[0][2] * rhs.x + m[1][2] * rhs.y + m[2][2] * rhs.z + m[3][2] * rhs.z;
		result.w = m[0][3] * rhs.x + m[1][3] * rhs.y + m[2][3] * rhs.z + m[3][3] * rhs.z;

		return result;
	}

	SMatrix operator *(const SMatrix& rhs) const
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

	SMatrix& operator *=(const SMatrix& rhs)
	{
		SMatrix mat = *this;
		*this = mat * rhs;
		return *this;
	}

	bool IsOrthogonal() const;
	SMatrix& Scale(const SVector3& scale);
	SMatrix& Rotate(const SQuaternion& rot);
	SMatrix& Translate(const SVector3& location);
	float Determinant() const;
	float RotDeterminan() const;
	SMatrix Inverse() const;
	SMatrix RotInverse() const;
};

struct SQuaternion
{
	SQuaternion()
	{}
	SQuaternion(const float x, const float y, const float z, const float w)
		:x(x), y(y), z(z), w(w)
	{
		Normalize();
	}
	float x = 0.0f, y = 0.0f, z = 0.0f, w = 1.0f;

	SQuaternion operator*(const SQuaternion& rhs) const
	{
		SQuaternion quat;
		quat.x = w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y;
		quat.y = w * rhs.y - x * rhs.z + y * rhs.w - z * rhs.x;
		quat.z = w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w;
		quat.w = w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z;

		quat.Normalize();
		return quat;
	}

	SVector3 operator*(const SVector3& vec3) const
	{
		SQuaternion v{vec3.x, vec3.y, vec3.z, 0.0f};
		SQuaternion invQ = Conjugate();
		SQuaternion r = *this * v * invQ;

		return SVector3(r.x, r.y, r.z);
	}

	SMatrix RotationMatrix() const
	{
		SMatrix mat;
		mat.m[0][0] = 1.0f - 2.0f * (y * y + z * z);	mat.m[1][0] = 2.0f * (x * y - z * w);		mat.m[2][0] = 2.0f * (x * z + y * w);		mat.m[3][0] = 0.0f;
		mat.m[0][1] = 2.0f * (x * y + z * w);			mat.m[1][1] = 1.0f - 2.0f * (x * x + z * z);mat.m[2][1] = 2.0f * (y * z - x * w);		mat.m[3][1] = 0.0f;
		mat.m[0][2] = 2.0f * (x * z - y * w);			mat.m[1][2] = 2.0f * (y * z + x * w);		mat.m[2][2] = 1.0f - 2.0f * (x * x + y * y);mat.m[3][2] = 0.0f;
		mat.m[0][3] = 0.0f;				 				mat.m[1][3] = 0.0f;							mat.m[2][3] = 0.0f;							mat.m[3][3] = 1.0f;
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

	void Normalize()
	{
		auto size = Size();
		if (size > 0.0f)
		{
			x = x / size;
			y = y / size;
			z = z / size;
			w = w / size;
		}
	}

	SQuaternion Normal() const
	{
		SQuaternion q = *this;
		q.Normalize();
		return q;
	}

	SQuaternion& Conjugate()
	{
		x = -x;
		y = -y;
		z = -z;
		return *this;
	}

	SQuaternion Conjugate() const
	{
		SQuaternion quat;
		quat.x = -x;
		quat.y = -y;
		quat.z = -z;
		quat.w = w;
		return quat;
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

	static constexpr float SM_1DIV2PI = 1.0f / 2.0f * static_cast<float>(S_PI);
	static constexpr float SM_2PI = static_cast<float>(S_PI) * 2.0f;
	static constexpr float SM_PIDIV2 = static_cast<float>(S_PI) / 2.0f;

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
		y = static_cast<float>(S_PI) - y;
		sign = -1.0f;
	}
	else if (y < -SM_PIDIV2)
	{
		y = -static_cast<float>(S_PI) - y;
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

namespace SMath
{
	template <typename T>
	T Clamp(T val, T min, T max)
	{
		T result;
		if (val < min)
			result = min;
		else if (max < val)
			result = max;
		else
			result = val;
		return result;
	}

	inline SMatrix Translation(const SVector3& location)
	{
		SMatrix m(SMatrix::Identity);
		m.Translate(location);
		return m;
	}

	inline SMatrix Scale(const SVector3& scale)
	{
		SMatrix m(SMatrix::Identity);
		m.Scale(scale);
		return m;
	}

	inline SMatrix Transform(const SVector3& scale, const SVector3& location, const SQuaternion& quat)
	{
		SMatrix m;
		return m.Scale(scale).Rotate(quat).Translate(location);
	}

	inline SMatrix Transpose(const SMatrix& matrix)
	{
		SMatrix mat = matrix;
		std::swap(mat.m[0][1], mat.m[1][0]);
		std::swap(mat.m[0][2], mat.m[2][0]);
		std::swap(mat.m[0][3], mat.m[3][0]);
		std::swap(mat.m[1][2], mat.m[2][1]);
		std::swap(mat.m[1][3], mat.m[3][1]);
		std::swap(mat.m[2][3], mat.m[3][2]);

		return mat;
	}

	/// inverse transpose
	inline SMatrix NormalMatrix(const SMatrix& matrix)
	{
		SMatrix normalMatrix = matrix;
		normalMatrix.m[3][0] = normalMatrix.m[3][1] = normalMatrix.m[3][2] = 0.0f;
		normalMatrix.m[3][3] = 1.0f;
		return Transpose(normalMatrix.RotInverse());
	}

	inline SQuaternion Slerp(const SQuaternion& qa, const SQuaternion& qb, float factor)
	{
		SQuaternion quat;
		// Calculate angle between them.
		float cosHalfTheta = qa.w * qb.w + qa.x * qb.x + qa.y * qb.y + qa.z * qb.z;
		// if qa=qb or qa=-qb then theta = 0 and we can return qa
		if (abs(cosHalfTheta) >= 1.0f) {
			quat.w = qa.w;quat.x = qa.x;quat.y = qa.y;quat.z = qa.z;
			return quat;
		}
		// Calculate temporary values.
		float halfTheta = acos(cosHalfTheta);
		float sinHalfTheta = sqrt(1.0f - cosHalfTheta*cosHalfTheta);
		// if theta = 180 degrees then result is not fully defined
		// we could rotate around any axis normal to qa or qb
		if (fabs(sinHalfTheta) < 0.001f) { // fabs is floating point absolute
			quat.w = (qa.w * 0.5f + qb.w * 0.5f);
			quat.x = (qa.x * 0.5f + qb.x * 0.5f);
			quat.y = (qa.y * 0.5f + qb.y * 0.5f);
			quat.z = (qa.z * 0.5f + qb.z * 0.5f);
			return quat;
		}
		float ratioA = sin((1.0f - factor) * halfTheta) / sinHalfTheta;
		float ratioB = sin(factor * halfTheta) / sinHalfTheta;
		//calculate Quaternion.
		quat.w = (qa.w * ratioA + qb.w * ratioB);
		quat.x = (qa.x * ratioA + qb.x * ratioB);
		quat.y = (qa.y * ratioA + qb.y * ratioB);
		quat.z = (qa.z * ratioA + qb.z * ratioB);
		return quat;
	}
};