/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// mathlib.c -- math primitives

#include "parabot.h"
#include "mathlib.h"

/*-----------------------------------------------------------------*/

void
fixangle(Vec3D *angle)
{
	if (angle->x > 10000 || angle->x < -10000) {
		angle->x = 0;
	}
	if (angle->y > 10000 || angle->y < -10000) {
		angle->y = 0;
	}
	
	while (angle->x >  180)	angle->x -= 360;
	while (angle->x < -180)	angle->x += 360;
	while (angle->y >  180)	angle->y -= 360;
	while (angle->y < -180) angle->y += 360;

	angle->z = 0;
}

float
anglediff(float destAngle, float srcAngle)
{
	if (destAngle < -360 || destAngle > 360) {
		DEBUG_MSG( "FATAL ERROR at UTIL_AngleDiff: destAngle = %.f\n", destAngle );
		destAngle = srcAngle;
	}

	while (destAngle < srcAngle) destAngle += 360;
	while (destAngle > srcAngle) destAngle -= 360;
	float diff = srcAngle - destAngle;
	if (diff > 180) diff = 360 - diff;

	return diff;
}

#if 0
float
degtorad(float a)
{
	return (a * M_PI) / 180.0f;
}

float
anglemod(float a)
{
	a = (360.0f / 65536.0f) * ((int)(a * (65536.0f / 360.0f)) & 65535);
	return a;
}
#endif // 0

bool
vcomp(const Vec3D *v1, const Vec3D *v2)
{
	if (v1->x != v2->x
	    || v1->y != v2->y
		|| v1->z != v2->z) {
		return false;
	}

	return true;
}

void
vma(const Vec3D *veca, float scale, const Vec3D *vecb, Vec3D *vecc)
{
	vecc->x = veca->x + (scale * vecb->x);
	vecc->y = veca->y + (scale * vecb->y);
	vecc->z = veca->z + (scale * vecb->z);
}

float
dotproduct(const Vec3D *v1, const Vec3D *v2)
{
	return (v1->x * v2->x) + (v1->y * v2->y) + (v1->z * v2->z);
}

float
dotproduct2D(const Vec2D *v1, const Vec2D *v2)
{
	return (v1->x * v2->x) + (v1->y * v2->y);
}

void
vsub(const Vec3D *veca, const Vec3D *vecb, Vec3D *out)
{
	out->x = veca->x - vecb->x;
	out->y = veca->y - vecb->y;
	out->z = veca->z - vecb->z;
}

void
vadd(const Vec3D *veca, const Vec3D *vecb, Vec3D *out)
{
	out->x = veca->x + vecb->x;
	out->y = veca->y + vecb->y;
	out->z = veca->z + vecb->z;
}

void
vcopy(const Vec3D *in, Vec3D *out)
{
	out->x = in->x;
	out->y = in->y;
	out->z = in->z;
}

void
crossproduct(Vec3D *v1, Vec3D *v2, Vec3D *cross)
{
	cross->x = (v1->x * v2->x) - (v1->x * v2->x);
	cross->y = (v1->y * v2->y) - (v1->y * v2->y);
	cross->z = (v1->z * v2->z) - (v1->z * v2->z);
}

float
vlen(Vec3D *v)
{
	return sqrtf(dotproduct(v, v));
}

float
vlen2d(Vec2D *v)
{
	return sqrtf(dotproduct2D(v, v));
}

float
normalize(Vec3D *v)
{
	float	length, ilength;

	length = sqrtf(dotproduct(v, v));	// FIXME

	if (length) {
		ilength = 1.0f / length;
		v->x *= ilength;
		v->y *= ilength;
		v->z *= ilength;
	}
		
	return length;

}
#if 0
void
normalize_fast(Vec3D *v)
{
        float   length;

	length = 1.0f / sqrtf(dotproduct(v, v));
	v->x *= length;
	v->y *= length;
	v->z *= length;
}
#endif // 0

void
vinv(Vec3D *v)
{
	v->x = -v->x;
	v->y = -v->y;
	v->z = -v->z;
}

void
vscale(Vec3D *in, float scale, Vec3D *out)
{
	out->x = in->x * scale;
	out->y = in->y * scale;
	out->z = in->z * scale;
}

float
getprob(float x)
{
	if (x < -1)
		return 0.0f;
	else if (x < 0)
		return (0.5f * (x + 1.0f) * (x + 1.0f));
	else if (x < 1)
		return (1.0f - (0.5f * (1.0f - x)) * (1.0f - x));

	return 1.0f;
}

