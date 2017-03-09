#define GLM_SWIZZLE
#include "CameraProjection.h"
#include <Input.h>

#include <glm\glm.hpp>
#include <glm\ext.hpp>

Camera::Camera(aie::Application* a_app)
{
	app = a_app;
}

Camera::~Camera()
{
}

void Camera::Update(float deltaTime)
{
	aie::Input *input = aie::Input::getInstance();	// Get a pointer to the input manager

	float SpeeeeEEEEEEEeeEEEEeeeeed = m_moveSpeed;
	
	if (input->isKeyDown(aie::INPUT_KEY_RIGHT_SHIFT) || input->isKeyDown(aie::INPUT_KEY_LEFT_SHIFT))
		SpeeeeEEEEEEEeeEEEEeeeeed = m_modSpeed;

	// Move camera using keys
	if (input->isKeyDown(aie::INPUT_KEY_W))
		m_position += SpeeeeEEEEEEEeeEEEEeeeeed * deltaTime * m_cameraLook;

	if (input->isKeyDown(aie::INPUT_KEY_S))
		m_position -= SpeeeeEEEEEEEeeEEEEeeeeed * deltaTime * m_cameraLook;

	glm::vec3 strafevec = glm::normalize(glm::cross(m_cameraLook, m_cameraUp));

	if (input->isKeyDown(aie::INPUT_KEY_A))
		m_position -= SpeeeeEEEEEEEeeEEEEeeeeed * deltaTime * strafevec;

	if (input->isKeyDown(aie::INPUT_KEY_D))
		m_position += SpeeeeEEEEEEEeeEEEEeeeeed * deltaTime * strafevec;

	// Rotate the camera base don mouse movement
	//aie::Input *input = aie::Input::getInstance();	// Get a pointer to the input manager
	if (input->isMouseButtonDown(aie::INPUT_MOUSE_BUTTON_RIGHT)) {
		float rotationAmount = m_mouseSensitivity * deltaTime;
		int mouseX, mouseY;							// Coords from input
		float xOffset, yOffset;					// How far we want to rotate camersa in these axes
		input->getMouseXY(&mouseX, &mouseY);		// Get mouse location and place it in mouseX/Y variables

													// Find out how far mouse has moved since last frame
		xOffset = (mouseX - m_lastMouseXPos) * rotationAmount;
		yOffset = (mouseY - m_lastMouseYPos) * rotationAmount;

		// Use these values in Yaw and Pitch values (so we can rotate that far)
		m_yaw += xOffset;
		m_pitch += yOffset;

		// Save last mouse position values
		m_lastMouseXPos = mouseX;
		m_lastMouseYPos = mouseY;


	}
	else {
		// Track last position of the mouse regardless of whether we're clicking
		input->getMouseXY(&m_lastMouseXPos, &m_lastMouseYPos);
	}

	// Calculate the new cameraLook vector
	CalculateLook();

	CalculateProjection();
}

void Camera::LookAt(glm::vec3 target)
{
	glm::vec3 look = glm::normalize(target - m_position); // Create a vector which is the direction to the target
	m_pitch = glm::degrees(glm::asin(look.y));
	m_yaw = glm::degrees(atan2(look.y, look.x));
	// roll

	CalculateLook();
}

void Camera::SetPosition(glm::vec3 position)
{
	m_position = position;
	CalculateView();
}

const glm::vec3 &Camera::GetPosition()
{
	return m_position;
}

const glm::mat4 & Camera::GetView()
{
	return m_viewMatrix;
}

const glm::mat4 & Camera::GetProjection()
{
	return projectionMatrix;
	// TODO: insert return statement here
}

bool Camera::IsVisible(glm::vec3 a_Position, glm::vec3 a_furthestPosition)
{
	//we're using centre position of an object as well as another position which
	//is on a theoretical sphere that surrounds the object

	//Step One - Transform centre and surface positions into screen space.
	glm::mat4 projView = projectionMatrix * m_viewMatrix;
	a_Position = (projView * glm::vec4(a_Position,1)).xyz;
	a_furthestPosition = (projView * glm::vec4(a_furthestPosition, 1)).xyz;
	

	//Step Two - Now that our sphere is in screen space, we can test it against the frustum cube
	glm::vec3 radial = a_Position - a_furthestPosition;
	float radius = radial.length();
	//Now test the centre and add/remove the radius and see if its ever within
	//-1 to 1 (so would be drawn)
	////if (a_Position.x <= 1 && a_Position.x >= -1
	////	&& a_Position.y <= 1 && a_Position.y >= -1
	////	&& a_Position.z <= 1 && a_Position.z >= -1)
	////{
	////	// The centre is inside the viewable area.
	////	return true;
	////}
	
	for (int i = 0; i < 3; i++)
	{
		if (abs(a_Position[i]) + radius >= 1) //if we're definitely outside 
		{
			return false;
		}
	}

	// We want to return whether this sphere is touching or indside the camera's frustum
	return false;
}

void Camera::CalculateLook()
{
	// Circle geometry nmaths to convert the viewing angle from
	// Yaw, Pitch and Roll into a single normalized vector

	// Protect against gimble lock
	if (m_pitch >= 90)
		m_pitch = 89.9;
	if (m_pitch <= -90)
		m_pitch = -89.0;


	glm::vec3 look;
	look.x = glm::cos(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
	look.y = glm::sin(glm::radians(m_pitch)) * glm::cos(glm::radians(m_roll));
	look.z = glm::sin(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
	m_cameraLook = glm::normalize(look);

	CalculateView();
}

void Camera::CalculateView()
{
	m_viewMatrix = glm::lookAt(m_position, m_position + m_cameraLook, m_cameraUp);
}

void Camera::CalculateProjection()
{
	projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f,
		app->getWindowWidth() / (float)app->getWindowHeight(),
		0.1f, 1000.0f);
	
}
