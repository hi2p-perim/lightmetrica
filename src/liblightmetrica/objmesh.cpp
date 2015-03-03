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
#include <lightmetrica/trianglemesh.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/pugihelper.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/config.h>
#include <lightmetrica/pathutils.h>
#include <lightmetrica/fp.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <boost/regex.hpp>

LM_NAMESPACE_BEGIN

class LogStream final : public Assimp::LogStream
{
public:

	LogStream(Logger::LogLevel level)
		: level(level)
	{

	}

	virtual void write(const char* message) override
	{
		// Remove new line
		std::string str(message);
		str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

		// Remove initial string
		boost::regex re("[a-zA-Z]+, +T[0-9]+: (.*)");
		str = boost::regex_replace(str, re, "$1");

		switch (level)
		{
			case Logger::LogLevel::Debug:
				LM_LOG_DEBUG(str);
				break;
			case Logger::LogLevel::Warning:
				LM_LOG_WARN(str);
				break;
			case Logger::LogLevel::Error:
				LM_LOG_ERROR(str);
				break;
			default:
				LM_LOG_INFO(str);
		}
	}

private:

	Logger::LogLevel level;

};

/*!
	Obj mesh.
	Triangle mesh implementation for Wavefront obj files.
	The class partially supports the specification of the Wavefront obj files.
*/
class ObjMesh final : public TriangleMesh
{
public:

	LM_COMPONENT_IMPL_DEF("obj");

public:

	ObjMesh() {}
	virtual ~ObjMesh() override {}

public:

	virtual bool Load(const ConfigNode& node, const Assets& assets) override;

public:

	virtual int NumVertices() const override				{ return static_cast<int>(positions.size()); }
	virtual int NumFaces() const override					{ return static_cast<int>(faces.size()); }
	virtual const Math::Float* Positions() const override	{ return positions.empty() ? nullptr : &positions[0]; }
	virtual const Math::Float* Normals() const override		{ return normals.empty() ? nullptr : &normals[0]; }
	virtual const Math::Float* TexCoords() const override	{ return texcoords.empty() ? nullptr : &texcoords[0]; }
	virtual const unsigned int* Faces() const override		{ return faces.empty() ? nullptr : &faces[0]; }

private:

	std::vector<Math::Float> positions;
	std::vector<Math::Float> normals;
	std::vector<Math::Float> texcoords;
	std::vector<unsigned int> faces;

};

bool ObjMesh::Load( const ConfigNode& node, const Assets& /*assets*/ )
{
	namespace fs = boost::filesystem;

	// Find 'path' element
	std::string path;
	if (!node.ChildValue("path", path))
	{
		return false;
	}
	
	// Resolve the path
	path = PathUtils::ResolveAssetPath(*node.GetConfig(), path);

	// Find 'group' element and get its value (this is optional)
	//std::string groupName = node.child("group").child_value();

	// Prepare for the logger of Assimp
	Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
	Assimp::DefaultLogger::get()->attachStream(new LogStream(Logger::LogLevel::Information), Assimp::Logger::Info);
	Assimp::DefaultLogger::get()->attachStream(new LogStream(Logger::LogLevel::Warning), Assimp::Logger::Warn);
	Assimp::DefaultLogger::get()->attachStream(new LogStream(Logger::LogLevel::Error), Assimp::Logger::Err);
#if LM_DEBUG_MODE
	Assimp::DefaultLogger::get()->attachStream(new LogStream(Logger::LogLevel::Debug), Assimp::Logger::Debugging);
#endif

#if LM_STRICT_FP && LM_PLATFORM_WINDOWS
	if (!FloatintPointUtils::EnableFPControl())
	{
		return false;
	}
#endif

	// Load file
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path.c_str(),
		aiProcess_GenNormals |
		//aiProcess_GenSmoothNormals |
		//aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices);

#if LM_STRICT_FP && LM_PLATFORM_WINDOWS
	if (!FloatintPointUtils::DisableFPControl())
	{
		return false;
	}
#endif

	if (!scene)
	{
		LM_LOG_ERROR(importer.GetErrorString());
		return false;
	}

	// Clear current data
	positions.clear();
	normals.clear();
	texcoords.clear();

	// Load triangle meshes
	// TODO : select mesh by name
	unsigned int lastNumFaces = 0;
	for (unsigned int meshIdx = 0; meshIdx < scene->mNumMeshes; meshIdx++)
	{
		auto* mesh = scene->mMeshes[meshIdx];

		// Positions and normals
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			auto& p = mesh->mVertices[i];
			auto& n = mesh->mNormals[i];
			positions.push_back(Math::Float(p.x));
			positions.push_back(Math::Float(p.y));
			positions.push_back(Math::Float(p.z));
			normals.push_back(Math::Float(n.x));
			normals.push_back(Math::Float(n.y));
			normals.push_back(Math::Float(n.z));
		}

		// Texture coordinates
		if (mesh->HasTextureCoords(0))
		{
			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				auto& uv = mesh->mTextureCoords[0][i];
				texcoords.push_back(Math::Float(uv.x));
				texcoords.push_back(Math::Float(uv.y));
			}
		}

		// Faces
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			// The mesh is already triangulated
			auto& f = mesh->mFaces[i];
			faces.push_back(lastNumFaces + f.mIndices[0]);
			faces.push_back(lastNumFaces + f.mIndices[1]);
			faces.push_back(lastNumFaces + f.mIndices[2]);
		}

		lastNumFaces += mesh->mNumFaces;
	}

	Assimp::DefaultLogger::kill();

	return true;
}

LM_COMPONENT_REGISTER_IMPL(ObjMesh, TriangleMesh);

LM_NAMESPACE_END