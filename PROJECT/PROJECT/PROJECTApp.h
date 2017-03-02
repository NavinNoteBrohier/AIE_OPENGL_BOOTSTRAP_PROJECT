#pragma once
#include <glm/mat4x4.hpp>
#include <iostream>
#include <Texture.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <vector>
#include "Application.h"
#include "Gizmos.h"
#include "Input.h"
#include "gl_core_4_4.h"
#include "CameraProjection.h"
#include "FBXFile.h"


// Now is not the time to run from the accusations that I am irredeemably broken.Resistance is saying that, not me.I am not broken.My code is broken.I am whole and I am doing the right thing.This is all part of the process.I have wrestled these demons before and I have won.I have looked defeat in the face and it is not a mirror, it is a mirage.I am going to make a game that some people will like to play.
// Attributed to Shaid Kamal Ahmad: https://medium.com/@shahidkamal/my-code-is-broken-and-so-am-i-923b0b1abf1b#.opajp7qro
// My code is broken. And I am going to fix it.

class Camera;
class ParticleEmitter;

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
	void LoadFBX(char* Location);
	void UnloadFBX();

protected:
	// camera transforms
	Camera *m_Camera;
	glm::mat4	m_viewMatrix;
	glm::mat4	m_projectionMatrix;

	//Textures
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
	unsigned int m_ParticleShader;

	unsigned int m_IndicesCount;
	unsigned int m_VertCount;

	unsigned int m_Vao;
	unsigned int m_Vbo;
	unsigned int m_Ibo;

	//FBX models
	//unsigned int m_shader;
	FBXFile *m_myFbxModel;
	std::vector<FBXFile*> m_FBXList;

	//Emitters;
	ParticleEmitter *m_Emitter;
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