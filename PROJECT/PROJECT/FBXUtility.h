#pragma once
#include "FBXFile.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/mat4x4.hpp>
#include <iostream>
#include <imgui.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <Texture.h>
#include "gl_core_4_4.h"
#include "Gizmos.h"

class GLMesh
{
public:
	unsigned int vao;
	unsigned int vbo;
	unsigned int ibo;
};

class FBXFILES
{
public:
	FBXFILES();
	virtual ~FBXFILES();


	//Fbx Loading
	void CreateFBXOpenGLBuffers(FBXFile *file);
	void CreateFBXOpenGLBuffers(FBXFile *file, bool additionalAtribs);
	void CleanupFBXOpenGLBuffers(FBXFile *file);

	//Load/Unload/Setup Models
	void LoadFBX(char* Location);
	void LoadFBX(char* Location, bool anim);
	void UnloadFBX();
	void FBXLoop(unsigned int a_Shader, FBXFile& a_Model, float a_scale, glm::mat4 m_projectionMat, glm::mat4 Cameraview);
	void FBXLoop(unsigned int a_Shader, FBXFile& a_Model, float a_scale, bool a_Skeleton, glm::mat4 m_projectionMat, glm::mat4 Cameraview);

	//Animations
	void LoadFBXAnimations(std::string a_String[]);
	void UpdateFBXAnimation(FBXFile* a_model, FBXFile* a_anims);

	bool m_renderwireframe = false;
	bool m_renderbones = false;
	float m_AnimationTimer = 0;
	int numfiles = 0;
	int m_currentanimation = 0;

	FBXFile *m_myFbxModel;
	std::vector<FBXFile*> m_FBXList;

	//animations

	static const int max_anims = 100;
	FBXFile *m_GhoulAnims[max_anims];
	std::vector<FBXFile**> m_AnimationList;



};