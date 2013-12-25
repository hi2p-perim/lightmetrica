/*
	nanon : A research-oriented renderer

	Copyright (c) 2014 Hisanari Otsu (hi2p.perim@gmail.com)

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

#include "pch.h"
#include <nanon/objmesh.h>
#include <nanon/logger.h>
#include <nanon/pugihelper.h>
#include <pugixml.hpp>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

NANON_NAMESPACE_BEGIN

class LogStream : public Assimp::LogStream
{
public:

	LogStream(Logger::LogLevel level)
		: level(level)
	{

	}

	virtual void write( const char* message )
	{
		switch (level)
		{
			case Logger::LogLevel::Debug:
				NANON_LOG_DEBUG(message);
				break;
			case Logger::LogLevel::Warning:
				NANON_LOG_WARN(message);
				break;
			case Logger::LogLevel::Error:
				NANON_LOG_ERROR(message);
				break;
			default:
				NANON_LOG_INFO(message);
		}
	}

private:

	Logger::LogLevel level;

};

class ObjMesh::Impl
{
public:

	Impl(ObjMesh* self);
	bool Load(const pugi::xml_node& node, const Assets& assets);

public:

	ObjMesh* self;
	std::vector<Math::Vec3> positions;
	std::vector<Math::Vec3> normals;
	std::vector<Math::Vec2> texcoords;
	std::vector<Math::Vec3i> faces;

};

ObjMesh::Impl::Impl( ObjMesh* self )
	: self(self)
{

}

bool ObjMesh::Impl::Load( const pugi::xml_node& node, const Assets& assets )
{
	// Check name and type
	if (node.name() != self->Name())
	{
		NANON_LOG_ERROR(boost::str(boost::format("Invalid node name '%s'") % node.name()));
		return false;
	}

	if (node.attribute("type").as_string() != self->Type())
	{
		NANON_LOG_ERROR(boost::str(boost::format("Invalid triangle mesh type '%s'") % node.attribute("type").as_string()));
		return false;
	}

	// Find 'path' element
	auto pathNode = node.child("path");
	if (!pathNode)
	{
		NANON_LOG_ERROR("Missing 'path' element");
		return false;
	}

	// Find 'group' element and get its value (this is optional)
	//std::string groupName = node.child("group").child_value();

	// Prepare for the logger of Assimp
	Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
	Assimp::DefaultLogger::get()->attachStream(new LogStream(Logger::LogLevel::Information), Assimp::Logger::Info);
	Assimp::DefaultLogger::get()->attachStream(new LogStream(Logger::LogLevel::Warning), Assimp::Logger::Warn);
	Assimp::DefaultLogger::get()->attachStream(new LogStream(Logger::LogLevel::Error), Assimp::Logger::Err);
#ifdef NANON_DEBUG_MODE
	Assimp::DefaultLogger::get()->attachStream(new LogStream(Logger::LogLevel::Debug), Assimp::Logger::Debugging);
#endif

	// Load file
	Assimp::Importer importer;

	std::string path = pathNode.child_value();
	const aiScene* scene = importer.ReadFile(path.c_str(),
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices);

	if (!scene)
	{
		NANON_LOG_ERROR(importer.GetErrorString());
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
			positions.push_back(Math::Vec3(Math::Float(p.x), Math::Float(p.y), Math::Float(p.z)));
			normals.push_back(Math::Vec3(Math::Float(n.x), Math::Float(n.y), Math::Float(n.z)));
		}

		// Texture coordinates
		if (mesh->HasTextureCoords(0))
		{
			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				auto& uv = mesh->mTextureCoords[0][i];
				texcoords.push_back(Math::Vec2(Math::Float(uv.x), Math::Float(uv.y)));
			}
		}

		// Faces
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			// The mesh is already triangulated
			auto& f = mesh->mFaces[i];
			faces.push_back(Math::Vec3i(lastNumFaces + f.mIndices[0], lastNumFaces + f.mIndices[1], lastNumFaces + f.mIndices[2]));
		}

		lastNumFaces += mesh->mNumFaces;
	}

	Assimp::DefaultLogger::kill();

	return true;

	// --

	// Load obj file
	//std::string path = pathNode.child_value();
	//std::vector<tinyobj::shape_t> shapes;

	//auto error = tinyobj::LoadObj(shapes, path.c_str());
	//if (!error.empty())
	//{
	//	NANON_LOG_ERROR(error);
	//	return false;
	//}

	//NANON_LOG_INFO(boost::str(boost::format("Loaded obj file : %s") % path));

	//// Clear current data
	//positions.clear();
	//normals.clear();
	//texcoords.clear();

	//// Load shapes
	//// Polygons are automatically triangulated
	//auto convToFloat = [](float v){ return Math::Float(v); };
	//for (const auto& shape : shapes)
	//{
	//	const auto& p = shape.mesh.positions;
	//	const auto& n = shape.mesh.normals;
	//	const auto& t = shape.mesh.texcoords;

	//	if (p.empty())
	//	{
	//		NANON_LOG_ERROR("Position array is empty");
	//		return false;
	//	}
	//	else
	//	{
	//		for (size_t i = 0; i < p.size(); i+=3)
	//		{
	//			positions.push_back(Math::Vec3(
	//				Math::Float(p[i]),
	//				Math::Float(p[i+1]),
	//				Math::Float(p[i+2])));
	//		}

	//		for (size_t i = 0; i < n.size(); i++)
	//		{
	//			normals.push_back(Math::Vec3(
	//				Math::Float(n[i]),
	//				Math::Float(n[i+1]),
	//				Math::Float(n[i+2])));
	//		}

	//		for (size_t i = 0; i < n.size(); i++)
	//		{
	//			texcoords.push_back(Math::Vec2(
	//				Math::Float(t[i]),
	//				Math::Float(t[i+1])));
	//		}
	//	}
	//}

	//return true;
}

// --------------------------------------------------------------------------------

ObjMesh::ObjMesh( const std::string& id )
	: TriangleMesh(id)
	, p(new Impl(this))
{

}

ObjMesh::~ObjMesh()
{
	NANON_SAFE_DELETE(p);
}

bool ObjMesh::Load( const pugi::xml_node& node, const Assets& assets )
{
	return p->Load(node, assets);
}

int ObjMesh::NumVertices() const
{
	return static_cast<int>(p->positions.size());
}

int ObjMesh::NumFaces() const
{
	return static_cast<int>(p->faces.size());
}

const Math::Vec3* ObjMesh::Positions() const
{
	return p->positions.empty() ? nullptr : &p->positions[0];
}

const Math::Vec3* ObjMesh::Normals() const
{
	return p->normals.empty() ? nullptr : &p->normals[0];
}

const Math::Vec2* ObjMesh::TexCoords() const
{
	return p->texcoords.empty() ? nullptr : &p->texcoords[0];
}

const Math::Vec3i* ObjMesh::Faces() const
{
	return p->faces.empty() ? nullptr : &p->faces[0];
}

NANON_NAMESPACE_END