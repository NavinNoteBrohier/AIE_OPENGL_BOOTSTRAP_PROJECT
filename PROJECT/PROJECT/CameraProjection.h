
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "gl_core_4_4.h"
#include "Application.h"

class aie::Application;

class Camera {
public:
	Camera(aie::Application* a_app);
	~Camera();


	void Update(float deltaTime);

	void LookAt(glm::vec3 target);
	void SetPosition(glm::vec3 position);

	//Test if something is in the frtum
	bool IsVisible(glm::vec3 a_Position, glm::vec3 a_furthestPosition);

	const glm::vec3 &GetPosition();

	const glm::mat4& GetView();

	const glm::mat4& GetProjection();

private:
	glm::mat4 m_viewMatrix;										// Matrix we'll pass to OpenGL so it knows how the cmaera looks at the world

	glm::vec3 m_cameraLook;										// Actual direction camera is aiming
	glm::vec3 m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);			// Setting camera orientation so that the y axis is always up

	glm::vec3 m_position;										// Position in world
	float m_yaw = 0.0f;											// left/right rotation of view
	float m_pitch = 0.0f;										// up/down rotation of view
	float m_roll = 0.0f;										// rotating orientation of view

	glm::mat4 projectionMatrix;
	
	aie::Application* app;

	const float m_mouseSensitivity = 10.0f;
	const float m_moveSpeed = 5.0f;
	const float m_modSpeed = m_moveSpeed * 2;


	int m_lastMouseXPos = 0;									// Recording previous mouse pos to see how far its moved
	int m_lastMouseYPos = 0;

	void CalculateLook();										// Gives us the vector of direction using yaw, pitch and roll
	void CalculateView();										// sets up the view matrix based on our camera information
	void CalculateProjection();
};
