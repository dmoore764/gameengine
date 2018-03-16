#pragma once

#include "stdint.h"

#define DEGREES_TO_RADIANS 0.0174533f
#define RADIANS_TO_DEGREES 57.2958f

struct v2i
{
	union
	{
		struct 
		{
			int x;
			int y;
		};

		struct 
		{
			int w;
			int h;
		};
	};
};

v2i V2I(int x, int y);
bool PointInAABB(v2i p, v2i bl, v2i ur);
int LengthSq(v2i a);
bool operator==(const v2i &a, const v2i &b);

struct v2
{
	float x,y;
};

v2 V2(float x, float y);
v2 operator+(const v2 &a, const v2 &b);
v2 operator-(const v2 &a, const v2 &b);
v2 operator*(float a, const v2 &b);
v2 operator*(const v2 &b, float a);
float Inner(const v2 &a, const v2 &b);
v2 Normalize(const v2 &a);
float LengthSq(const v2 &a);
float Length(const v2 &a);


struct v3
{
	float x,y,z;
};

v3 V3(float x, float y, float z);
v3 operator+(const v3 &a, const v3 &b);
v3 operator-(const v3 &a, const v3 &b);
v3 operator*(float a, const v3 &b);
v3 operator*(const v3 &b, float a);
float Inner(const v3 &a, const v3 &b);
v3 Cross(const v3 &a, const v3 &b);
v3 Normalize(const v3 &a);
float Length(const v3 &a);

struct v4
{
	float x, y, z, w;
};

v4 V4(float x, float y, float z, float w);
float Inner(const v4 &a, const v4 &b);

struct m4
{
	union
	{
		v4 col[4];
		struct
		{
			float a, e, i, m;
			float b, f, j, n;
			float c, g, k, o;
			float d, h, l, p;
		};
		float vals[16];
	};
	float& operator[](int i);
	const float& operator[](int i) const;
};

m4 M4();
m4 M4(float val);
m4 M4(float *vals);
v4 operator*(const m4 &m, const v4 &v);
m4 operator*(const m4 &matA, const m4 &matB);
m4 Inverse(const m4 &m);
m4 Transpose(const m4 &m);
m4 MakeTranslation(float x, float y, float z);
m4 MakeScale(float x, float y, float z);
m4 MakePerspective(float fov, float aspect, float n, float f);
m4 MakeLookat(v3 pos, v3 up, v3 at);

struct plane
{
	v3 normal;
	float d;
};

struct frust
{
	union
	{
		struct {
			plane N;
			plane L;
			plane R;
			plane T;
			plane B;
			plane F;
		};
		plane NLRTBF[6];
	};
};

plane PlaneFromThreeNonColinearPoints(const v3 &p1, const v3 &p2, const v3 &p3);
plane PlaneFromNormalAndPoint(const v3 &n, const v3 &p);
bool PointOnPositivePlaneSide(const plane &p, const v3 &p0);
bool PointInFrust(const frust &f, const v3 &p);
bool OBBInFrust(const frust &f, v3 *p);

struct quat
{
	union
	{
		struct
		{
			float x;
			float y;
			float z;
		};
		v3 u;
	};
	float w;
};

quat CreateQuat(float angle, v3 vector);
quat CreateQuatRAD(float angle, v3 vector);
quat Normalize(const quat &q);
quat Inverse(const quat &q);
quat Nlerp(const quat &q1, const quat &q2, float blend);
v3 RotateByQuat(const v3 &p, const quat &q);
quat RotateQuatByQuat(const quat &pInit, const quat &pRot);
quat RotateQuatByRotationVec(const quat &q, const v3 &rot);
quat operator+(const quat &a, const quat &b);
quat operator-(const quat &a, const quat &b);
quat operator-(const quat &a);
quat operator*(const float &f, const quat &q);
m4 QuatToM4(const quat &q);
quat M4ToQuat(const m4 &m);
m4 RotationPositionToM4(const quat &r, const v3 &p);
m4 RotationPositionScaleToM4(const quat &r, const v3 &p, const v3 &s);

struct line_seg_2d
{
	v2 start;
	v2 end;
};

struct line_seg_3d
{
	v3 start;
	v3 end;
};

void LinePlaneIntersection(const line_seg_3d &line, const plane &p, v3 *point, bool *parallel);
line_seg_3d WindowPointToWorldLineSeg(const v2 &ndcPoint, const m4 &perspectiveView);
v2 WorldPointToNDCScreenPoint(const v3 &worldPoint, const m4 &perspectiveView);
float DistanceToLineSeg2D(const line_seg_2d &seg, const v2 &p, v2 *pointOnSeg);
float DistanceToLine(const line_seg_2d &line, const v2 &p, v2 *pointOnLine);
void ClosestPointsOn3DLines(const line_seg_3d &lineA, const line_seg_3d &lineB, v3 *ptA, v3 *ptB, bool *parallel);

float ReciprocalSqrt(float n);
float SafeDivide(float n, float d);

float Min(float a, float b);
float Max(float a, float b);
float ClampToRange(float min, float val, float max);
void ClampToRange(float min, float *val, float max);
float WrapToRange(float min, float val, float max);
void WrapToRange(float min, float *val, float max);
float Abs(float a);
float Sign(float a);
float SnapToGrid(float val, float grid);

int Min(int a, int b);
int Max(int a, int b);
int ClampToRange(int min, int val, int max);
void ClampToRange(int min, int *val, int max);
int WrapToRange(int min, int val, int max);
int Abs(int a);
int Sign(int a);

float Lerp(float current, float destination, float proportional_change);

uint32_t Min(uint32_t a, uint32_t b);
uint32_t Max(uint32_t a, uint32_t b);
uint32_t ClampToRange(uint32_t min, uint32_t val, uint32_t max);
void ClampToRange(uint32_t min, uint32_t *val, uint32_t max);

bool FloatEquals(float a, float b, float epsilon);
