#include "Particle.h"

ParticleEmitter::ParticleEmitter()

	:m_Particles(nullptr),
		m_FirstDead(0),
		m_MaxParticles(0),
		m_Postion(0,0,0),
		m_vao(0),m_vbo(0),m_ibo(0),
		m_VertexData(nullptr) 
{

}


ParticleEmitter::~ParticleEmitter()
{
	delete[] m_Particles;
	delete[] m_VertexData;

	glDeleteVertexArrays(1 ,&m_vao);
	glDeleteBuffers(1 ,&m_vbo);
	glDeleteBuffers(1, &m_ibo);

}

void ParticleEmitter::Initialise(unsigned int a_EmitRate, unsigned int	a_MaxParticles, float a_LifeTimeMin, float a_LifetimeMax, float a_VelocityMin, float a_Velocitymax, float a_StartSize, float a_EndSize, const vec4 & a_StartColor, const vec4 & a_EndColor)
{
	// Setup Emit Timers
	m_EmitTimer = 0;
	m_EmitRate = 1.0f / a_EmitRate;

	// Store all Variables passed in

	m_StartColor = a_StartColor;
	m_EndColor = a_EndColor;

	m_StartSize = a_StartSize;
	m_EndSize = a_EndSize;

	m_VelocityMin = a_VelocityMin;
	m_VelocityMax = a_Velocitymax;

	m_LifeSpanMin = a_LifeTimeMin;
	m_LifeSpanMax = a_LifetimeMax;
	
	m_MaxParticles = a_MaxParticles;

	// Create a new particle array.
	m_Particles = new Particle[m_MaxParticles];
	m_FirstDead = 0;

	// Create the array of vertices for the particles.
	// 4 vertices per particle for a quad,
	// will be  filled during update.

	m_VertexData = new ParticleVertex[m_MaxParticles * 4];

	// Create the index buffer data for the particles
	// 6 indices per quad of 2 triangles
	// fill it now as it never changes

	unsigned int* indexData = new unsigned int[m_MaxParticles * 6];
	for (unsigned int i = 0; i < m_MaxParticles; i++)
	{
		indexData[i * 6 + 0] = i * 4 + 0;
		indexData[i * 6 + 1] = i * 4 + 1;
		indexData[i * 6 + 2] = i * 4 + 2;

		indexData[i * 6 + 3] = i * 4 + 0;
		indexData[i * 6 + 4] = i * 4 + 2;
		indexData[i * 6 + 5] = i * 4 + 3;
	}

	// Create openGL  buffers
	
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo);
	glGenBuffers(1, &m_ibo);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData
	(
		GL_ARRAY_BUFFER, m_MaxParticles * 4 * sizeof(ParticleVertex),
		m_VertexData, GL_DYNAMIC_DRAW
		);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData
	(
		GL_ELEMENT_ARRAY_BUFFER, m_MaxParticles * 6 * sizeof(unsigned int),
		indexData, GL_STATIC_DRAW
	);


	glEnableVertexAttribArray(0); // Position
	glEnableVertexAttribArray(1); // Colors
	glVertexAttribPointer(
		0, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), 0);
	glVertexAttribPointer(
		1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), ((char*)0) + 16);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

	delete[] indexData;

  //
}

void ParticleEmitter::Emit()
{
	// only emit if there is a dead particle to use.
	if (m_FirstDead >= m_MaxParticles)
		return;

	// resurrect the first dead particle
	Particle& particle = m_Particles[m_FirstDead++];

	// Assign its starting position
	particle.Position = m_Postion;

	// Randomise its lifespan
	particle.LifeTime = 0;
	particle.LifeSpan = (rand() / (float)RAND_MAX) * (m_LifeSpanMax - m_LifeSpanMax) + m_LifeSpanMin;

	// Set its starting size and color;
	particle.Color = m_StartColor;
	particle.Size = m_StartSize;
		
	// Randomise velocity direction and strength
	float Velocity = (rand() / (float)RAND_MAX) * (m_VelocityMax - m_VelocityMin) + m_VelocityMin;
	particle.Velocity.x = (rand() / (float)RAND_MAX) * 2 - 1;
	particle.Velocity.y = (rand() / (float)RAND_MAX) * 2 - 1;
	particle.Velocity.z = (rand() / (float)RAND_MAX) * 2 - 1;
	particle.Velocity = glm::normalize(particle.Velocity) * Velocity;
}

void ParticleEmitter::Update(float a_DeltaTime, Camera a_camera)
{

	// Spawn particles
	m_EmitTimer += a_DeltaTime;

	while (m_EmitTimer > m_EmitRate)
	{
		Emit();
		m_EmitTimer -= m_EmitRate;
	}
	unsigned int quad = 0;

	// Update particles and turn live particles into billboard quads
	for (unsigned int i = 0; i < m_FirstDead; i++)
	{
		Particle* particle = &m_Particles[i];

		particle->LifeTime += a_DeltaTime;

		if (particle->LifeTime >= particle->LifeSpan)
		{
			// Swap the last alive with the first one
			*particle = m_Particles[m_FirstDead - 1];
			m_FirstDead--;
		}
		else
		{
			// Move particle
			particle->Position += particle->Velocity * a_DeltaTime;

			// Size particle
			particle->Size = glm::mix(m_StartSize, m_EndSize,
				particle->LifeTime / particle->LifeSpan);

			// Color particle
			particle->Color = glm::mix(m_StartColor, m_EndColor,
				particle->LifeTime / particle->LifeSpan);
			
			// Make a quad the correct size and color
			float HalfSize = particle->Size * 0.5f;

			m_VertexData[quad * 4 + 0].Position = 
				vec4(HalfSize,HalfSize, 0, 1);

			m_VertexData[quad * 4 + 0].Color = particle->Color;

			m_VertexData[quad * 4 + 1].Position =
				vec4(-HalfSize, HalfSize, 0, 1);

			m_VertexData[quad * 4 + 1].Color = particle->Color;

			m_VertexData[quad * 4 + 2].Position =
				vec4(-HalfSize, -HalfSize, 0, 1);

			m_VertexData[quad * 4 + 2].Color = particle->Color;

			m_VertexData[quad * 4 + 3].Position =
				vec4(HalfSize, -HalfSize, 0, 1);

			m_VertexData[quad * 4 + 3].Color = particle->Color;

			//Create billboard transform.
			vec3 zAxis = glm::normalize(a_camera.GetPosition() - particle->Position);
			vec3 xAxis = glm::cross(vec3(a_camera.GetView()[1]), zAxis);
			vec3 yAxis = glm::cross(zAxis, zAxis);
			glm::mat4 BillBoard
			(
				vec4(xAxis, 0),
				vec4(yAxis, 0),
				vec4(zAxis, 0),
				vec4(0, 0, 0, 1)
			);

			m_VertexData[quad * 4 + 0].Position = BillBoard *
				m_VertexData[quad * 4 + 0].Position +
				vec4(particle->Position, 0);

			m_VertexData[quad * 4 + 1].Position = BillBoard *
				m_VertexData[quad * 4 + 1].Position +
				vec4(particle->Position, 0);

			m_VertexData[quad * 4 + 2].Position = BillBoard *
				m_VertexData[quad * 4 + 2].Position +
				vec4(particle->Position, 0);

			m_VertexData[quad * 4 + 3].Position = BillBoard *
				m_VertexData[quad * 4 + 3].Position +
				vec4(particle->Position, 0);

			++quad;

		}
	}

}

void ParticleEmitter::Draw()
{
	// Sync the particle vertex buffer
	// based on how many alive particles 
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_FirstDead * 4 * sizeof(ParticleVertex), m_VertexData);

	//Draw particles
	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, m_FirstDead * 6, GL_UNSIGNED_INT, 0);

}
