/*
Copyright 2017 Zachary Blystone <zakblystone@gmail.com>

This file is part of Metacade.

Metacade is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Metacade is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Metacade.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
===============================================================================
halfplane.h: Vec3 with plane characteristics
===============================================================================
*/

#pragma once

#include "metacade_private.h"

namespace Arcade
{

class METACADE_API CHalfPlane : public CVec3
{
public:
	CHalfPlane();
	CHalfPlane(const CVec2& dir, float distance);
	CHalfPlane(const CVec2& dir, const CVec2& origin);

	inline float distance(const CVec2& point) const;
	inline EPointClassify intersection(const CVec2& start, const CVec2& end, float& fraction) const;
	inline EPointClassify clasifyPoint(const CVec2& point, bool checkOn = false) const;
};

}