#include "game_math.h"
#include "math.h"
#include "compiler_helper.h"
#include "float.h"
#include "memory.h"

v2i V2I(int x, int y)
{
	v2i result;
	result.x = x;
	result.y = y;
	return result;
}

v2 operator+(const v2 &a, const v2 &b)
{
	v2 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

v2 operator-(const v2 &a, const v2 &b)
{
	v2 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	return result;
}

v2 operator*(float a, const v2 &b)
{
	v2 result;
	result.x = a * b.x;
	result.y = a * b.y;
	return result;
}

v2 operator*(const v2 &b, float a)
{
	v2 result;
	result.x = a * b.x;
	result.y = a * b.y;
	return result;
}

inline float Inner(const v2 &a, const v2 &b)
{
	float result;
	result = a.x*b.x + a.y*b.y;
	return result;
}

v2 Normalize(const v2 &a)
{
	float factor = SafeDivide(1.0f , sqrtf(a.x*a.x + a.y*a.y));
	v2 result = factor * a;
	return result;
}

float LengthSq(const v2 &a)
{
	float result = Inner(a, a);
	return result;
}


v3 V3(float x, float y, float z)
{
	v3 result;
	result.x = x;
	result.y = y;
	result.z = z;
	return result;
}

v3 operator+(const v3 &a, const v3 &b)
{
	v3 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	return result;
}

v3 operator-(const v3 &a, const v3 &b)
{
	v3 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return result;
}

v3 operator*(float a, const v3 &b)
{
	v3 result;
	result.x = a * b.x;
	result.y = a * b.y;
	result.z = a * b.z;
	return result;
}

v3 operator*(const v3 &b, float a)
{
	v3 result;
	result.x = a * b.x;
	result.y = a * b.y;
	result.z = a * b.z;
	return result;
}

inline float Inner(const v3 &a, const v3 &b)
{
	float result;
	result = a.x*b.x + a.y*b.y + a.z*b.z;
	return result;
}

inline v3 Cross(const v3 &a, const v3 &b)
{
	v3 result;
	result.x = a.y*b.z - a.z*b.y;
	result.y = a.z*b.x - a.x*b.z;
	result.z = a.x*b.y - a.y*b.x;
	return result;
}

v3 Normalize(const v3 &a)
{
	float factor = SafeDivide(1.0f , sqrtf(a.x*a.x + a.y*a.y + a.z*a.z));
	v3 result = factor * a;
	return result;
}

float Length(const v3 &a)
{
	float result;
	result = sqrt(Inner(a, a));
	return result;
}

v4 V4(float x, float y, float z, float w)
{
	v4 result;
	result.x = x;
	result.y = y;
	result.z = z;
	result.w = w;
	return result;
}

float Inner(const v4 &a, const v4 &b)
{
	float result;
	result = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
	return result;
}

m4 M4()
{
	m4 result;
	result.col[0] = V4(1,0,0,0);
	result.col[1] = V4(0,1,0,0);
	result.col[2] = V4(0,0,1,0);
	result.col[3] = V4(0,0,0,1);
	return result;
}

m4 M4(float val)
{
	m4 result;
	result.col[0] = V4(val, val, val, val);
	result.col[1] = V4(val, val, val, val);
	result.col[2] = V4(val, val, val, val);
	result.col[3] = V4(val, val, val, val);
	return result;
}

m4 M4(float *vals)
{
	m4 result;
	memcpy(&result[0], vals, sizeof(m4));
	return result;
}

float& m4::operator[](int i)
{
	assert(i >= 0 && i < 16);
	return vals[i];
}

const float& m4::operator[](int i) const
{
	assert(i >= 0 && i < 16);
	return vals[i];
}

v4 operator*(const m4 &m, const v4 &v)
{
	v4 result;
	result.x = m.a*v.x + m.b*v.y + m.c*v.z + m.d*v.w;
	result.y = m.e*v.x + m.f*v.y + m.g*v.z + m.h*v.w;
	result.z = m.i*v.x + m.j*v.y + m.k*v.z + m.l*v.w;
	result.w = m.m*v.x + m.n*v.y + m.o*v.z + m.p*v.w;
	return result;
}

m4 operator*(const m4 &matA, const m4 &matB)
{
	m4 result;
	result.a = matA.a*matB.a + matA.b*matB.e + matA.c*matB.i + matA.d*matB.m;
	result.b = matA.a*matB.b + matA.b*matB.f + matA.c*matB.j + matA.d*matB.n;
	result.c = matA.a*matB.c + matA.b*matB.g + matA.c*matB.k + matA.d*matB.o;
	result.d = matA.a*matB.d + matA.b*matB.h + matA.c*matB.l + matA.d*matB.p;

	result.e = matA.e*matB.a + matA.f*matB.e + matA.g*matB.i + matA.h*matB.m;
	result.f = matA.e*matB.b + matA.f*matB.f + matA.g*matB.j + matA.h*matB.n;
	result.g = matA.e*matB.c + matA.f*matB.g + matA.g*matB.k + matA.h*matB.o;
	result.h = matA.e*matB.d + matA.f*matB.h + matA.g*matB.l + matA.h*matB.p;

	result.i = matA.i*matB.a + matA.j*matB.e + matA.k*matB.i + matA.l*matB.m;
	result.j = matA.i*matB.b + matA.j*matB.f + matA.k*matB.j + matA.l*matB.n;
	result.k = matA.i*matB.c + matA.j*matB.g + matA.k*matB.k + matA.l*matB.o;
	result.l = matA.i*matB.d + matA.j*matB.h + matA.k*matB.l + matA.l*matB.p;

	result.m = matA.m*matB.a + matA.n*matB.e + matA.o*matB.i + matA.p*matB.m;
	result.n = matA.m*matB.b + matA.n*matB.f + matA.o*matB.j + matA.p*matB.n;
	result.o = matA.m*matB.c + matA.n*matB.g + matA.o*matB.k + matA.p*matB.o;
	result.p = matA.m*matB.d + matA.n*matB.h + matA.o*matB.l + matA.p*matB.p;
	return result;
}

m4 Inverse(const m4 &m)
{
	
    float inv[16], det;
    int i;

    inv[0] = m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return M4();

    det = 1.0f / det;
	
	m4 result;

    for (i = 0; i < 16; i++)
        result[i] = inv[i] * det;

	return result;
}

m4 Transpose(const m4 &m)
{
	m4 result;
	result.col[0] = V4(m.a, m.b, m.c, m.d);
	result.col[1] = V4(m.e, m.f, m.g, m.h);
	result.col[2] = V4(m.i, m.j, m.k, m.l);
	result.col[3] = V4(m.m, m.n, m.o, m.p);
	return result;
}

m4 MakeTranslation(float x, float y, float z)
{
	m4 result = M4();
	result.col[3] = V4(x, y, z, 1);
	return result;
}

m4 MakePerspective(float fov, float aspect, float n, float f)
{
	m4 result = M4(0.0f);

	float t = n * tanf (fov/2.0f);
	float r = aspect * t;

	result.col[0].x = SafeDivide(n,r);
	result.col[1].y = SafeDivide(n,t);
	result.col[2].z = SafeDivide(-(f + n), (f - n));
	result.col[2].w = -1;
	result.col[3].z = SafeDivide(-2*f*n, (f - n));
	return result;
}

m4 MakeLookat(v3 pos, v3 up, v3 at)
{
	m4 result;
	m4 trans = MakeTranslation(-pos.x, -pos.y, -pos.z);
	up = Normalize(up);
	v3 forward = Normalize(pos - at);
	v3 right = Normalize(Cross(up, forward));
	result.a = right.x;
	result.b = right.y;
	result.c = right.z;
	result.d = 0;

	result.e = up.x;
	result.f = up.y;
	result.g = up.z;
	result.h = 0;

	result.i = forward.x;
	result.j = forward.y;
	result.k = forward.z;
	result.l = 0;

	result.m = 0;
	result.n = 0;
	result.o = 0;
	result.p = 1;

	result = result*trans;
	return result;
}


plane PlaneFromThreeNonColinearPoints(const v3 &p1, const v3 &p2, const v3 &p3)
{
	v3 p21 = p2 - p1;
	v3 p32 = p3 - p2;
	plane result;
	result.normal = Normalize(Cross(p32, p21));
	result.d = -result.normal.x*p1.x - result.normal.y*p1.y - result.normal.z*p1.z;
	return result;
}

inline bool PointOnPositivePlaneSide(const plane &p, const v3 &p0)
{
	return (p.normal.x*p0.x + p.normal.y*p0.y + p.normal.z*p0.z + p.d > 0);
}

bool PointInFrust(const frust &f, const v3 &p)
{
	if (!PointOnPositivePlaneSide(f.N, p))
		return false;
	if (!PointOnPositivePlaneSide(f.L, p))
		return false;
	if (!PointOnPositivePlaneSide(f.R, p))
		return false;
	if (!PointOnPositivePlaneSide(f.B, p))
		return false;
	if (!PointOnPositivePlaneSide(f.T, p))
		return false;
	if (!PointOnPositivePlaneSide(f.F, p))
		return false;
	return true;
}



quat CreateQuat(float angle, v3 vector)
{
	quat result;
	float halfTheta = angle * DEGREES_TO_RADIANS * 0.5f;
	float cosHalfTheta = cosf (halfTheta);
	float sinHalfTheta = sinf (halfTheta);

	result.w = cosHalfTheta;
	if (Inner(vector, vector) == 0)
		vector = {1,0,0};
	else
		vector = Normalize(vector);

	vector = sinHalfTheta * vector;

	result.u = vector;
	return (result);
}

quat CreateQuatRAD(float angle, v3 vector)
{
	quat result;
	float halfTheta = angle * 0.5f;
	float cosHalfTheta = cosf (halfTheta);
	float sinHalfTheta = sinf (halfTheta);

	result.w = cosHalfTheta;
	if (Inner(vector, vector) == 0)
		vector = {1,0,0};
	else
		vector = Normalize(vector);

	vector = sinHalfTheta * vector;

	result.u = vector;
	return (result);
}

quat Inverse(const quat &q)
{
	quat result;
	float oneOverDenom = SafeDivide(1.0f, q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z);
	result.w = q.w * oneOverDenom;
	result.u = q.u * -oneOverDenom;
	return result;
}

quat Normalize(const quat &q)
{
	quat result;
	float oneOverDenom = SafeDivide(1.0f, sqrt(q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z));
	result.w = q.w * oneOverDenom;
	result.u = q.u * oneOverDenom;
	return result;
}

quat Nlerp(const quat &q0, const quat &q1, float blend)
{
	quat result;

	float dot = q0.w*q1.w + q0.x*q1.x + q0.y*q1.y + q0.z*q1.z;
	float blendI = 1.0f - blend;
	if(dot < 0.0f)
	{
		quat tmpF;
		tmpF.w = -q1.w;
		tmpF.x = -q1.x;
		tmpF.y = -q1.y;
		tmpF.z = -q1.z;
		result.w = blendI*q0.w + blend*tmpF.w;
		result.x = blendI*q0.x + blend*tmpF.x;
		result.y = blendI*q0.y + blend*tmpF.y;
		result.z = blendI*q0.z + blend*tmpF.z;
	}
	else
	{
		result.w = blendI*q0.w + blend*q1.w;
		result.x = blendI*q0.x + blend*q1.x;
		result.y = blendI*q0.y + blend*q1.y;
		result.z = blendI*q0.z + blend*q1.z;
	}
	result = Normalize(result);

	return result;
};

v3 RotateByQuat(const v3 &p, const quat &q)
{

	//Test path 2
	v3 t = 2 * Cross(q.u, p);
	v3 result = p + q.w * t + Cross(q.u, t);
	return (result);
}


quat RotateQuatByQuat(const quat &pInit, const quat &pRot)
{
	quat result;

	result.w = (pRot.w*pInit.w - pRot.u.x*pInit.u.x - pRot.u.y*pInit.u.y - pRot.u.z*pInit.u.z);
	result.u.x = (pRot.w*pInit.u.x + pRot.u.x*pInit.w + pRot.u.y*pInit.u.z - pRot.u.z*pInit.u.y);
	result.u.y = (pRot.w*pInit.u.y - pRot.u.x*pInit.u.z + pRot.u.y*pInit.w + pRot.u.z*pInit.u.x);
	result.u.z = (pRot.w*pInit.u.z + pRot.u.x*pInit.u.y - pRot.u.y*pInit.u.x + pRot.u.z*pInit.w);

	return result;
}

quat RotateQuatByRotationVec(const quat &q, const v3 &rot)
{
	quat result;

	float theta = Length(rot); //in radians
	quat rotQuat = CreateQuatRAD(theta, rot);
	
	result = RotateQuatByQuat(q, rotQuat);

	return result;
}

quat operator+(const quat &a, const quat &b)
{
	quat result = RotateQuatByQuat(a, b);
	return result;
}

quat operator-(const quat &a, const quat &b)
{
	quat result = RotateQuatByQuat(a, Inverse(b));
	return result;
}

quat operator*(const float &f, const quat &q)
{
	quat result = q;
	result.u = f * result.u;
	result.w = f * result.w;
	return result;
}

//TODO: rearrange to eliminate duplicate multiplies
m4 QuatToM4(const quat &q)
{
	m4 result;
	result.a = 1 - 2*q.y*q.y - 2*q.z*q.z;
	result.b = 2*q.x*q.y - 2*q.w*q.z;
	result.c = 2*q.x*q.z + 2*q.w*q.y;
	result.d = 0.0f;
	result.e = 2*q.x*q.y + 2*q.w*q.z;
	result.f = 1 - 2*q.x*q.x - 2*q.z*q.z;
	result.g = 2*q.y*q.z - 2*q.w*q.x;
	result.h = 0.0f;
	result.i = 2*q.x*q.z - 2*q.w*q.y;
	result.j = 2*q.y*q.z + 2*q.w*q.x;
	result.k = 1 - 2*q.x*q.x - 2*q.y*q.y;
	result.l = 0.0f;
	result.m = 0;
	result.n = 0;
	result.o = 0;
	result.p = 1;
	return result;
}

quat M4ToQuat(const m4 &m)
{
	quat result;

	float s0, s1, s2;
	int k0, k1, k2, k3;
	float *q = (float *)(&result);
	if ( m[0 * 4 + 0] + m[1 * 4 + 1] + m[2 * 4 + 2] > 0.0f ) {
		k0 = 3;
		k1 = 2;
		k2 = 1;
		k3 = 0;
		s0 = 1.0f;
		s1 = 1.0f;
		s2 = 1.0f;
	} else if ( m[0 * 4 + 0] > m[1 * 4 + 1] && m[0 * 4 + 0] > m[2 * 4 + 2] ) {
		k0 = 0;
		k1 = 1;
		k2 = 2;
		k3 = 3;
		s0 = 1.0f;
		s1 = -1.0f;
		s2 = -1.0f;
	} else if ( m[1 * 4 + 1] > m[2 * 4 + 2] ) { 
		k0 = 1;
		k1 = 0;
		k2 = 3;
		k3 = 2;
		s0 = -1.0f;
		s1 = 1.0f;
		s2 = -1.0f;
	} else {
		k0 = 2;
		k1 = 3;
		k2 = 0;
		k3 = 1;
		s0 = -1.0f;
		s1 = -1.0f;
		s2 = 1.0f;
	}
	float t = s0 * m[0 * 4 + 0] + s1 * m[1 * 4 + 1] + s2 * m[2 * 4 + 2] + 1.0f;
	float s = ReciprocalSqrt( t ) * 0.5f;
	q[k0] = s * t;
	q[k1] = ( m[0 * 4 + 1] - s2 * m[1 * 4 + 0] ) * s;
	q[k2] = ( m[2 * 4 + 0] - s1 * m[0 * 4 + 2] ) * s;
	q[k3] = ( m[1 * 4 + 2] - s0 * m[2 * 4 + 1] ) * s; 

	return result;
}

float ReciprocalSqrt( float n)
{
	union {
		float f;
		long i;
	} conv;
	
	float x2;
	const float threehalfs = 1.5f;

	x2 = n * 0.5f;
	conv.f  = n;
	conv.i  = 0x5f3759df - ( conv.i >> 1 );	// what the fuck? 
	conv.f  = conv.f * ( threehalfs - ( x2 * conv.f * conv.f ) );
	return conv.f;
}

float SafeDivide(float n, float d)
{
	float result = 0;
	if (d == 0)
	{
		if (n < 0)
			result = -FLT_MAX;
		else
			result = FLT_MAX;
	}
	else
	{
		result = n/d;
	}
	return result;
}

float Min(float a, float b)
{
	float result = a;
	if (b < a)
		result = b;
	return result;
}

float Max(float a, float b)
{
	float result = a;
	if (b > a)
		result = b;
	return result;
}

float ClampToRange(float min, float val, float max)
{
	float result = Max(Min(val, max), min);
	return result;
}

void ClampToRange(float min, float *val, float max)
{
	*val = ClampToRange(min, *val, max);
}

float Abs(float a)
{
	float result = a;
	if (a < 0)
		result *= -1;
	return result;
}

float Sign(float a)
{
	float result = 0;
	if (a < 0)
		result = -1;
	if (a > 0)
		result = 1;
	return result;
}

int Min(int a, int b)
{
	int result = a;
	if (b < a)
		result = b;
	return result;
}

int Max(int a, int b)
{
	int result = a;
	if (b > a)
		result = b;
	return result;
}

int ClampToRange(int min, int val, int max)
{
	int result = Max(Min(val, max), min);
	return result;
}

void ClampToRange(int min, int *val, int max)
{
	*val = ClampToRange(min, *val, max);
}

int WrapToRange(int min, int val, int max)
{
	int result = val;
	while (result < min)
	{
		result += (max - min);
	}
	while (result >= max)
	{
		result -= (max - min);
	}
	return result;
}

float Lerp(float current, float destination, float proportional_change)
{
	float result = current + (destination - current)*proportional_change;
	return result;
}

int Abs(int a)
{
	int result = a;
	if (a < 0)
		result *= -1;
	return result;
}

int Sign(int a)
{
	int result = 0;
	if (a < 0)
		result = -1;
	if (a > 0)
		result = 1;
	return result;
}


uint32_t Min(uint32_t a, uint32_t b)
{
	uint32_t result = a;
	if (b < a)
		result = b;
	return result;
}

uint32_t Max(uint32_t a, uint32_t b)
{
	uint32_t result = a;
	if (b > a)
		result = b;
	return result;
}

uint32_t ClampToRange(uint32_t min, uint32_t val, uint32_t max)
{
	uint32_t result = Max(Min(val, max), min);
	return result;
}

void ClampToRange(uint32_t min, uint32_t *val, uint32_t max)
{
	*val = ClampToRange(min, *val, max);
}

inline bool PointInAABB(v2i p, v2i bl, v2i ur)
{
	return (p.x >= bl.x && p.x <= ur.x && p.y >= bl.y && p.y <= ur.y);
}

inline int LengthSq(v2i a)
{
	int result = (a.x*a.x) + (a.y*a.y);
	return (result);
}
