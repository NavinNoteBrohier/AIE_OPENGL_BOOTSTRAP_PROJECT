
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

class Camera {
public:
	Camera();
	~Camera();


	void Update(float deltaTime);

	void LookAt(glm::vec3 target);
	void SetPosition(glm::vec3 position);
	const glm::vec3 &GetPosition();

	const glm::mat4 & GetView();

private:
	glm::mat4 m_viewMatrix;										// Matrix we'll pass to OpenGL so it knows how the cmaera looks at the world

	glm::vec3 m_cameraLook;										// Actual direction camera is aiming
	glm::vec3 m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);			// Setting camera orientation so that the y axis is always up

	glm::vec3 m_position;										// Position in world
	float m_yaw = 0.0f;											// left/right rotation of view
	float m_pitch = 0.0f;										// up/down rotation of view
	float m_roll = 0.0f;										// rotating orientation of view

	const float m_mouseSensitivity = 10.0f;
	const float m_moveSpeed = 5.0f;
	const float m_modSpeed = m_moveSpeed * 2;


	int m_lastMouseXPos = 0;									// Recording previous mouse pos to see how far its moved
	int m_lastMouseYPos = 0;

	void CalculateLook();										// Gives us the vector of direction using yaw, pitch and roll
	void CalculateView();										// sets up the view matrix based on our camera information

};
