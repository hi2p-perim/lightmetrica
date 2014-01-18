/*
	L I G H T  M E T R I C A

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
#include <lightmetrica/objmesh.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/pugihelper.h>
#include <pugixml.hpp>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <regex>

LM_NAMESPACE_BEGIN

class LogStream : public Assimp::LogStream
{
public:

	LogStream(Logger::LogLevel level)
		: level(level)
	{

	}

	virtual void write( const char* message )
	{
		// Remove new line
		std::string str(message);
		str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

		// Remove initial string
		std::regex re("[a-zA-Z]+, +T[0-9]+: (.*)");
		str = std::regex_replace(str, re, "$1");

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

class ObjMesh::Impl : public Object
{
public:

	Impl(ObjMesh* self);
	bool LoadAsset(const pugi::xml_node& node, const Assets& assets);

public:

	ObjMesh* self;
	std::vector<Math::Float> positions;
	std::vector<Math::Float> normals;
	std::vector<Math::Float> texcoords;
	std::vector<unsigned int> faces;

};

ObjMesh::Impl::Impl( ObjMesh* self )
	: self(self)
{

}

bool ObjMesh::Impl::LoadAsset( const pugi::xml_node& node, const Assets& assets )
{
	// Find 'path' element
	auto pathNode = node.child("path");
	if (!pathNode)
	{
		LM_LOG_ERROR("Missing 'path' element");
		return false;
	}

	// Find 'group' element and get its value (this is optional)
	//std::string groupName = node.child("group").child_value();

	// Prepare for the logger of Assimp
	Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
	Assimp::DefaultLogger::get()->attachStream(new LogStream(Logger::LogLevel::Information), Assimp::Logger::Info);
	Assimp::DefaultLogger::get()->attachStream(new LogStream(Logger::LogLevel::Warning), Assimp::Logger::Warn);
	Assimp::DefaultLogger::get()->attachStream(new LogStream(Logger::LogLevel::Error), Assimp::Logger::Err);
#ifdef LM_DEBUG_MODE
	Assimp::DefaultLogger::get()->attachStream(new LogStream(Logger::LogLevel::Debug), Assimp::Logger::Debugging);
#endif

	// Load file
	Assimp::Importer importer;

	std::string path = pathNode.child_value();
	const aiScene* scene = importer.ReadFile(path.c_str(),
		aiProcess_GenNormals |
		//aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices);

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

// --------------------------------------------------------------------------------

ObjMesh::ObjMesh( const std::string& id )
	: TriangleMesh(id)
	, p(new Impl(this))
{

}

ObjMesh::~ObjMesh()
{
	LM_SAFE_DELETE(p);
}

bool ObjMesh::LoadAsset( const pugi::xml_node& node, const Assets& assets )
{
	return p->LoadAsset(node, assets);
}

int ObjMesh::NumVertices() const
{
	return static_cast<int>(p->positions.size());
}

int ObjMesh::NumFaces() const
{
	return static_cast<int>(p->faces.size());
}

const Math::Float* ObjMesh::Positions() const
{
	return p->positions.empty() ? nullptr : &p->positions[0];
}

const Math::Float* ObjMesh::Normals() const
{
	return p->normals.empty() ? nullptr : &p->normals[0];
}

const Math::Float* ObjMesh::TexCoords() const
{
	return p->texcoords.empty() ? nullptr : &p->texcoords[0];
}

const unsigned int* ObjMesh::Faces() const
{
	return p->faces.empty() ? nullptr : &p->faces[0];
}

LM_NAMESPACE_END