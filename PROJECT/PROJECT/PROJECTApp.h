#pragma once

#include <glm/mat4x4.hpp>
#include <iostream>
#include <Texture.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <vector>
#include <imgui.h>
#include <glm/gtx/matrix_decompose.hpp>
#include "Application.h"
#include "Gizmos.h"
#include "Input.h"
#include "gl_core_4_4.h"
#include "CameraProjection.h"
#include "FBXFile.h"
#include "FBXUtility.h"
#include "Shaders.h"
///////////////////////////////////////////////////////////////////////////////
//																			 //
///																			///
//// Navin Brohoof - navinnotebrohier.github.io - navin.brohier@gmail.com  ////
///																			///
//																			 //
///////////////////////////////////////////////////////////////////////////////

class Camera;
class ParticleEmitter;

namespace aie
{
	class Texture;
}



class PROJECTApp : public aie::Application
{
public:

	PROJECTApp();
	virtual ~PROJECTApp();

	virtual bool startup();
	virtual void shutdown();

	virtual void update(float deltaTime);
	virtual void draw();
	
	//Post processing
	void SetupFrameBuffer();
	void SetupQuad();

	// Frustum Culling
	void GetFrustumPlanes(const glm::mat4& transform, glm::vec4* planes);

	//shader
	void LoadShader();
	void UnloadShader();

	void CreateLandScape();
	void DestroyLandScape();

	// Load/Unload/Setup Textures
	void LoadTex(char* Location);
	void LoadMap(char* Location);

	void SetupTex(GLchar* a_handle, int a_index, unsigned int a_shader);

	void UnloadTex();
	void UnloadMap();

	//Particles
	void LoadEmitter(int EmitRate, int MaxParticles, float LifeTimeMin,
		float LifetimeMax, float VelocityMin, float VelocityMax, float StartSize,
		float EndSize, glm::vec4 StartColor, glm::vec4 EndColor);
	void UnloadEmitter();

	// Camera transforms
	Camera *m_Camera;
	glm::mat4	m_viewMatrix;
	glm::mat4	m_projectionMatrix;

protected:
	// Constants 
	int WindowWidth = 1280;
	int WindowHeight = 720;

	// Textures
	std::vector<aie::Texture*> m_TexList;
	std::vector<aie::Texture*> m_MapList;

	// Stats for landscape
	const int m_LandWidth = 255;
	const int m_LandLength = 255;
	const float m_vertSeperation = 0.1f;
	const float m_maxHeight = 3.0f;

	// Stats for lights
	glm::vec3 m_LightPosition;
	glm::vec3 m_LightColor;
	glm::vec3 m_LightSpecColor;

	float LightSphereSize = 0.25;

	float m_LightAmbientStrength;
	float m_SpecStrength;

	// Shaders
	unsigned int m_shader;
	unsigned int m_ModelShader;
	unsigned int m_AnimationShader;
	unsigned int m_ParticleShader;
	unsigned int m_ParticleShaderImage;
	unsigned int m_PostProcessingShader;

	unsigned int m_IndicesCount;
	unsigned int m_VertCount;

	unsigned int m_Vao;
	unsigned int m_Vbo;
	unsigned int m_Ibo;

	//Post processing
	GLuint m_fbo;
	unsigned int m_fboTexture;
	GLuint m_fboDepth;
	GLuint m_vao;
	GLuint m_vbo;

	int PostEffectNum = 0;
	bool PostOne = false;
	bool PostTwo = true;
	bool PostThree = false;

	// Frustum
	bool visible;

	// Emitters;
	ParticleEmitter *m_Emitter;
	std::vector<ParticleEmitter*>m_EmitterList;

	float lifetimemin = 1, lifetimemax = 1;
	float velocitymin = 1, velocitymax = 1;
	float startsize =1, endsize = 1;
	int maxparticles = 500;
	int	emitrate = 1000;
	glm::vec4 startcolor = glm::vec4(1, 0, 0, 1), endcolor = glm::vec4(1, 1, 0, 1);

	// Vertex
	struct Vertex
	{
		glm::vec4 pos;
		glm::vec2 uv;
		glm::vec4 normal;
		glm::vec4 color;
		static void SetupVertexAttribPointers();
	};

};


class AABB 
{
public:
	AABB() { reset(); };
	virtual ~AABB() {};

	void reset()
	{
		min.x = min.y = min.z = 1e37f;
		max.x = max.y = max.z = -1e37f;
	}

	void fit(const std::vector<glm::vec3>& points)
	{
		for (auto& p : points)
		{
			if (p.x < min.x) min.x = p.x;
			if (p.y < min.y) min.y = p.y;
			if (p.z < min.z) min.z = p.z;
			if (p.x > max.x) max.x = p.x;
			if (p.y > max.y) max.y = p.y;
			if (p.z > max.z) max.z = p.z;
		}

		centre = (min + max) * 0.5f;
		radius = glm::distance(min, centre);

	}

	glm::vec3 min, max;
	glm::vec3 centre;
	float radius;



};

class BoundingSphere
{
public:
	BoundingSphere() : centre(0), radius(0) {};
	~BoundingSphere() {};

	void fit(const std::vector < glm::vec3>& points)
	{
		glm::vec3 min(1e37f), max(-1e37f);
		for(auto&  p : points )
		{
			if (p.x < min.x) min.x = p.x;
			if (p.y < min.y) min.y = p.y;
			if (p.z < min.z) min.z = p.z;
			if (p.x > max.x) max.x = p.x;
			if (p.y > max.y) max.y = p.y;
			if (p.z > max.z) max.z = p.z;
		}
		centre = (min + max) * 0.5f;
		radius = glm::distance(min, centre);
	}

	glm::vec3 centre;
	float radius;

};
