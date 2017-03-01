#pragma once

#include "Application.h"
#include <glm/mat4x4.hpp>
#include "Gizmos.h"
#include "Input.h"
#include <iostream>
#include <Texture.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "gl_core_4_4.h"
#include "CameraProjection.h"
#include "FBXFile.h"
#include <vector>

class Camera;
namespace aie
{
	class Texture;
}

class GLMesh
{
public:
	unsigned int vao;
	unsigned int vbo;
	unsigned int ibo;
};


class PROJECTApp : public aie::Application
{
public:

	PROJECTApp();
	virtual ~PROJECTApp();

	virtual bool startup();
	virtual void shutdown();

	virtual void update(float deltaTime);
	virtual void draw();

	//FBX render
	void CreateFBXOpenGLBuffers(FBXFile *file);
	void CleanupFBXOpenGLBuffers(FBXFile *file);

	//shader
	void LoadShader();
	void UnloadShader();

	void CreateLandScape();
	void DestroyLandScape();

	// Load Textures
	void LoadTex(char* Location);
	void LoadMap(char* Location);

	void SetupTex(GLchar* a_handle, int a_index, unsigned int a_shader);

	void UnloadTex();
	void UnloadMap();

	//Load Models


protected:
	// camera transforms
	Camera *m_Camera;
	glm::mat4	m_viewMatrix;
	glm::mat4	m_projectionMatrix;

	std::vector<aie::Texture*> m_TexList;
	std::vector<aie::Texture*> m_MapList;

	//stats for landscape
	const int m_LandWidth = 255;
	const int m_LandLength = 255;
	const float m_vertSeperation = 0.1f;
	const float m_maxHeight = 3.0f;

	//stats for light
	glm::vec3 m_LightPosition;
	glm::vec3 m_LightColor;
	glm::vec3 m_LightSpecColor;
	float m_LightAmbientStrength;
	float m_SpecStrength;
	//Shaders
	unsigned int m_shader;
	unsigned int m_ModelShader;

	unsigned int m_IndicesCount;
	unsigned int m_VertCount;

	unsigned int m_Vao;
	unsigned int m_Vbo;
	unsigned int m_Ibo;

	//FBX models
	//unsigned int m_shader;
	FBXFile *m_myFbxModel;

	//vertex
	struct Vertex
	{
		glm::vec4 pos;
		glm::vec2 uv;
		glm::vec4 normal;
		glm::vec4 color;
		static void SetupVertexAttribPointers();
	};
};