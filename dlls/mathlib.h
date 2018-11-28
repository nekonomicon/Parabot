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
// mathlib.h
#pragma once
#if !defined(_MATHLIB_H)
#define _MATHLIB_H
#include <math.h>

typedef union {
	struct {
		float x, y;
	};
	float at[2];
} Vec2D;

typedef union {
	struct {
		float x, y, z;
	};
	float at[3];
} Vec3D;

#if !(defined M_PI)
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif // M_PI

enum {
	NANMASK = (255<<23)
};

enum {	// up / down
	PITCH,
	// left / right
	YAW,
	// fall over
	ROLL
};

inline bool
is_nan(float f)
{
	return (((*(int *)&f) & NANMASK) == NANMASK);
}

void	fixangle(Vec3D *angle);
float	anglediff(float destAngle, float srcAngle);

float	degtorad(float a);
float	anglemod(float a);
bool	vcomp(const Vec3D *v1, const Vec3D *v2);
void	vma(const Vec3D *veca, float scale, const Vec3D *vecb, Vec3D *vecc);
float	dotproduct(const Vec3D *v1, const Vec3D *v2);
float	dotproduct2D(const Vec2D *v1, const Vec2D *v2);
void	vsub(const Vec3D *veca, const Vec3D *vecb, Vec3D *out);
void	vadd(const Vec3D *veca, const Vec3D *vecb, Vec3D *out);
void	vcopy(const Vec3D *in, Vec3D *out);
void	crossproduct(Vec3D *v1, Vec3D *v2, Vec3D *cross);
float	vlen(Vec3D *v);
float	vlen2d(Vec2D *v);
float	normalize(Vec3D *v);
void	normalize_fast(Vec3D *v);
void	vinv(Vec3D *v);
void	vscale(Vec3D *in, float scale, Vec3D *out);
float	getprob(float x);

#define BOX_ON_PLANE_SIDE(emins, emaxs, p)	\
	(((p)->type < 3)?						\
	(										\
		((p)->dist <= (emins)[(p)->type])?	\
			1								\
		:									\
		(									\
			((p)->dist >= (emaxs)[(p)->type])?\
				2							\
			:								\
				3							\
		)									\
	)										\
	:										\
		BoxOnPlaneSide( (emins), (emaxs), (p)))
#endif // _MATHLIB_H
