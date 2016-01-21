#include "SMath.h"


const SMatrix SMatrix::Identity(SVector3(1.0f,0.0f,0.0f), SVector3(0.0f, 1.0f, 0.0f), SVector3(0.0f, 0.0f, 1.0f));

SMatrix& SMatrix::Scale(const SVector3& scale)
{
	m[0][0] *= scale.x;
	m[1][1] *= scale.y;
	m[2][2] *= scale.z;
	return *this;
}

SMatrix& SMatrix::Rotate(const SQuaternion& rot)
{
	SMatrix temp = *this;
	*this = rot.RotationMatrix() * temp;
	return *this;
}

SMatrix& SMatrix::Translate(const SVector3& location)
{
	m[3].x += location.x;
	m[3].y += location.y;
	m[3].z += location.z;
	return *this;
}

float SMatrix::Determinant() const
{
	return m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[2][1])
		- m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[2][2])
		+ m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]);
}

SVector3 Cross(const SVector3& a, const SVector3& b)
{	
	SVector3 v;

	v.x = a.y*b.z - a.z*b.y;
	v.y = a.z*b.x - a.x*b.z;
	v.z = a.x*b.y - a.y*b.x;

	return v;
}

float Dot(const SVector3 & a, const SVector3 & b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float Dot(const SVector4 & a, const SVector4 & b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

SMatrix MatrixPerspectiveFOV(float FovAngleY, float AspectRatio, float NearZ, float FarZ)
{
	SMatrix M;

	float    SinFov;
	float    CosFov;
	ScalarSinCos(&SinFov, &CosFov, 0.5f * FovAngleY);
	const float TanFov = SinFov / CosFov;

	float top = NearZ * TanFov;
	float bottom = -top;
	float right = top * AspectRatio;
	float left = -right;

	return MatrixFrustum(left, right, bottom, top, NearZ, FarZ);
}

SMatrix MatrixFrustum(float left, float right, float bottom, float top, float zNear, float zFar)
{
	SMatrix M;

	float fDivRange = 1.0f / (zFar - zNear);

	M.m[0][0] = 2.0f * zNear / (right - left);
	M.m[0][1] = 0.0f;
	M.m[0][2] = 0.0f;
	M.m[0][3] = 0.0f;

	M.m[1][0] = 0.0f;
	M.m[1][1] = 2.0f * zNear / (top - bottom);
	M.m[1][2] = 0.0f;
	M.m[1][3] = 0.0f;

	M.m[2][0] = (left + right) / (right - left);
	M.m[2][1] = (bottom + top) / (top - bottom);
	M.m[2][2] = -(zNear + zFar)* fDivRange;
	M.m[2][3] = -1.0f;

	M.m[3][0] = 0.0f;
	M.m[3][1] = 0.0f;
	M.m[3][2] = -2.0f * zNear * zFar * fDivRange;
	M.m[3][3] = 0.0f;
	return M;
}

SMatrix MatrixLookAt(const SVector3& eye, const SVector3& lookAt, const SVector3& up)
{
	const SVector3 zaxis = (eye - lookAt).normal();
	const SVector3 xaxis = Cross(up, zaxis).normal();
	const SVector3 yaxis = Cross(zaxis, xaxis);

	SMatrix m;
	m.m[0] = { xaxis.x, yaxis.x, zaxis.x, 0 };
	m.m[1] = { xaxis.y, yaxis.y, zaxis.y, 0 };
	m.m[2] = { xaxis.z, yaxis.z, zaxis.z, 0 };
	m.m[3] = { -Dot(xaxis, eye), -Dot(yaxis, eye), -Dot(zaxis, eye), 1.0f };

	return m;
}
