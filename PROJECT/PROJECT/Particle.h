#pragma once
#include "PROJECTApp.h"

using glm::vec3;
using glm::vec4;
using glm::mat4;

struct Particle
{
	vec3 Position;
	vec3 Velocity;
	vec4 Color;
	float Size;
	float LifeTime;
	float LifeSpan;
};

struct ParticleVertex
{
	vec4 Position;
	vec4 Color;
};

class ParticleEmitter
{
public:
	ParticleEmitter();
	virtual ~ParticleEmitter();

	void Initialise
	(
		unsigned int	a_EmitRate,		unsigned int	a_MaxParticles,
		float			a_LifeTimeMin,	float			a_LifetimeMax,
		float			a_VelocityMin,	float			a_Velocitymax,
		float			a_StartSize,	float			a_EndSize,
		const vec4&		a_StartColor,	const vec4&		a_EndColor
	);

	void Emit();

	void Update(float a_DeltaTime, Camera a_camera);
	void SetVariables(int EmitRate, int MaxParticles, float LifeTimeMin,
		float LifetimeMax, float VelocityMin, float VelocityMax, float StartSize,
		float EndSize, glm::vec4 StartColor, glm::vec4 EndColor);

	void setImage(char* a_tex);

	void Draw();

protected:
	Particle*		m_Particles;
	unsigned int	m_FirstDead;
	unsigned int	m_MaxParticles;

	unsigned int	m_vao, m_vbo, m_ibo;
	ParticleVertex* m_VertexData;

	vec3			m_Postion;

	float			m_EmitTimer;
	float			m_EmitRate;
	
	float			m_LifeSpanMin;
	float			m_LifeSpanMax;

	float			m_VelocityMin;
	float			m_VelocityMax;

	float			m_StartSize;
	float			m_EndSize;

	aie::Texture* m_texture;
	bool m_TexActive = false;

	vec4			m_StartColor;
	vec4			m_EndColor;
};