/*
	Lightmetrica : A research-oriented renderer

	Copyright (c) 2014 Hisanari Otsu

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "pch.h"
#include <lightmetrica/scenefactory.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/naivescene.h>
#include <lightmetrica/bvhscene.h>
#if LM_SSE2 && LM_SINGLE_PRECISION
#include <lightmetrica/qbvhscene.h>
#endif

LM_NAMESPACE_BEGIN

SceneFactory::SceneFactory()
{

}

SceneFactory::~SceneFactory()
{

}

Scene* SceneFactory::Create( const std::string& type ) const
{
	if (type == "naive")
	{
		return new NaiveScene();
	}
	else if (type == "bvh")
	{
		return new BVHScene();
	}
	else if (type == "qbvh")
	{
#if LM_SSE2 && LM_SINGLE_PRECISION
		return new QBVHScene();
#else
		LM_LOG_ERROR("QBVH implementation requires SSE2 support and single precision mode");
		return nullptr;
#endif
	}
	else
	{
		LM_LOG_ERROR("Invalid scene type '" + type + "'");
		return nullptr;
	}
}

LM_NAMESPACE_END
