#pragma once

#include "Application.h"
#include <glm/mat4x4.hpp>
#include <vector>
#include "Gizmos.h"
#include "Input.h"
#include <iostream>
#include <Texture.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "gl_core_4_4.h"

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

protected:
	// camera transforms
	Camera *m_Camera;
	glm::mat4	m_viewMatrix;
	glm::mat4	m_projectionMatrix;

	


};