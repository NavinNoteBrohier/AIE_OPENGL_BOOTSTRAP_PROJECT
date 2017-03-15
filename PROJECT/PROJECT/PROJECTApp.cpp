#include "PROJECTApp.h"
#include "Particle.h"

using glm::vec3;
using glm::vec4;
using glm::mat4;
using aie::Gizmos;

static std::string ghoul_animation_filenames[] =
{
	"./models/micro_ghoul/animations/mic_ghoul_attack.fbx",
	"./models/micro_ghoul/animations/mic_ghoul_block.fbx",
	"./models/micro_ghoul/animations/mic_ghoul_die.fbx",
	"./models/micro_ghoul/animations/mic_ghoul_gethit_front.fbx",
	"./models/micro_ghoul/animations/mic_ghoul_gethit_left.fbx",
	"./models/micro_ghoul/animations/mic_ghoul_gethit_right.fbx",
	"./models/micro_ghoul/animations/mic_ghoul_idle.fbx",
	"./models/micro_ghoul/animations/mic_ghoul_laugh.fbx",
	"./models/micro_ghoul/animations/mic_ghoul_multi_attack.fbx",
	"./models/micro_ghoul/animations/mic_ghoul_panic.fbx",
	"./models/micro_ghoul/animations/mic_ghoul_roar.fbx",
	"./models/micro_ghoul/animations/mic_ghoul_search.fbx",
	"./models/micro_ghoul/animations/mic_ghoul_spawn.fbx",
	"./models/micro_ghoul/animations/mic_ghoul_talk.fbx",
	"./models/micro_ghoul/animations/mic_ghoul_walk.fbx",
	"./models/micro_ghoul/animations/mic_ghoul_walk_end.fbx",
	"./models/micro_ghoul/animations/mic_ghoul_walk_Start.fbx",
	"./models/micro_ghoul/animations/micro_ghoul_allinone.fbx",
};

PROJECTApp::PROJECTApp() 
{

}

PROJECTApp::~PROJECTApp() 
{

}

bool PROJECTApp::startup() {

	setBackgroundColour(0.0f, 0.2f, 0.5f);

	//Camera
	m_Camera = new Camera(this);
	m_Camera->SetPosition(vec3(10.0f, 10.0f, 10.0f));
	m_Camera->LookAt(vec3(0.0f));

	// create simple camera transforms
	m_viewMatrix = glm::lookAt(vec3(10), vec3(0), vec3(0, 1, 0));
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 1000.0f);

	// initialise gizmo primitive counts
	Gizmos::create(10000, 10000, 10000, 10000);

	//Setup light position and colour
	m_LightPosition = glm::vec3(3.0f, 15.0f, 0.0f);
	m_LightColor = glm::vec3(0.5f, 0.5f, 0.5f);
	m_LightAmbientStrength = 0.25f;
	m_SpecStrength = 0.7f;
	m_LightSpecColor = glm::vec3(0.5f, 0.5f, 0.5f);

	//Load texture and heightmap
	//Textures
	LoadTex("textures/Tile.png");
	LoadTex("textures/grass.png");
	LoadTex("textures/sand.png");
	LoadTex("textures/snow.png");
	LoadTex("textures/rock.png");
	LoadTex("textures/bubble.png");
	// Heightmap
	LoadMap("textures/HM.bmp");


	//Load models and animations
	// boolean indicates bones for animation
	LoadFBX("./models/micro_ghoul/models/micro_ghoul.fbx", true);
	LoadFBX("./models/soulspear/soulspear.fbx");

	numfiles = sizeof(ghoul_animation_filenames) / sizeof(std::string);
	for (int i = 0; i < numfiles; i++)
	{
		m_GhoulAnims[i] = new FBXFile();
		m_GhoulAnims[i]->loadAnimationsOnly(ghoul_animation_filenames[i].c_str(), FBXFile::UNITS_CENTIMETER);
	
	}
	
	
	//LoadFBXAnimations(ghoul_animation_filenames);
	
	//Load emitters 
	LoadEmitter(100, 5000, 0.1f, 1.0f, 1, 5, 0.2f, 0.02f, glm::vec4(1, 0, 0, 1), glm::vec4(1, 1, 0, 1));
	LoadEmitter(100, 5000, 0.1f, 1.0f, 1, 5, 0.2f, 0.02f, glm::vec4(0, 1, 1, 1), glm::vec4(0, 0, 0, 1));
	m_EmitterList.at(1)->setImage(5);
	
	//

	//Post Processing down there |
							//   V


	LoadShader();
	SetupFrameBuffer();
	SetupQuad();
	CreateLandScape();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}

void PROJECTApp::shutdown()
{
	UnloadTex();
	UnloadMap();
	UnloadFBX();

	for (int i = 0; i < numfiles - 1; i++)
	{
		CleanupFBXOpenGLBuffers(m_GhoulAnims[i]);
		m_GhoulAnims[i]->unload();
		delete m_GhoulAnims[i];
	}

	Gizmos::destroy();
}

void PROJECTApp::update(float deltaTime)
{
	// query time since application started
	float time = getTime();

	// rotate camera
	m_Camera->Update(deltaTime);


	
	ImGui::Begin("frustum cull");
	ImGui::Checkbox("Sphere is visible", &visible);
	ImGui::End();


	// Particles
	ImGui::Begin("particles");

	ImGui::SliderInt("Emit rate",&emitrate,0,5000);
	ImGui::SliderInt("Max Particles", &maxparticles, 0, 1000);

	ImGui::SliderFloat("Lifetime Min", &lifetimemin, 0, 100);
	ImGui::SliderFloat("Lifetime Max", &lifetimemax, 0, 100);

	ImGui::SliderFloat("Velocity Min", &velocitymin, 0, 1000);
	ImGui::SliderFloat("Velocity Max", &velocitymax, 0, 1000);

	ImGui::SliderFloat("Start Size", &startsize, 0, 10);
	ImGui::SliderFloat("End Size", &endsize, 0, 10);

	ImGui::ColorEdit4("Start Color", glm::value_ptr(startcolor));
	ImGui::ColorEdit4("End Color", glm::value_ptr(endcolor));

	ImGui::End();

	
	for (unsigned int i = 0; i < m_EmitterList.size(); i++)
	{

		m_EmitterList.at(i)->SetVariables(emitrate,maxparticles,lifetimemin,lifetimemax,velocitymin,velocitymax,startsize,endsize,startcolor,endcolor);
		m_EmitterList.at(i)->Update(deltaTime, *m_Camera);
	}

	//Animation
	m_AnimationTimer += deltaTime;
	UpdateFBXAnimation(m_FBXList.at(0),m_GhoulAnims[m_currentanimation]);




	// Imgui
	ImGui::Begin("Lights");
	ImGui::SliderFloat("Light Position X", &m_LightPosition.x, -20, 20);
	ImGui::SliderFloat("Light Position Y", &m_LightPosition.y, -20, 20);
	ImGui::SliderFloat("Light Position Z", &m_LightPosition.z, -20, 20);
	ImGui::SliderFloat("LightSphere Size", &LightSphereSize, 0, 100);

	ImGui::SliderFloat("Light Ambient Strength", 
		&m_LightAmbientStrength, 0, 1);
	ImGui::SliderFloat("Light Spec Strength", 
		&m_SpecStrength, 0, 100);

	ImGui::ColorEdit3("Ambient Light Color", glm::value_ptr(m_LightColor));
	ImGui::ColorEdit3("Specular Light Color", glm::value_ptr(m_LightSpecColor));

	ImGui::End();
	// Anim iMGUI

	ImGui::Begin("Animations");
	ImGui::Checkbox("Render Wireframe", &m_renderwireframe);
	ImGui::Checkbox("Render bones", &m_renderbones);
	for (int i = 0; i < numfiles; i++)
	{
		char buffer[64];

		std::string name = ghoul_animation_filenames[i].substr(ghoul_animation_filenames[i].find_last_of('/') + 1);

		sprintf_s(buffer, "%d %s", i, name.c_str());
		if (ImGui::Button(buffer))m_currentanimation = i;
	}
	ImGui::End();
	// wipe the gizmos clean for this frame
	
	// quit if we press escape
	aie::Input* input = aie::Input::getInstance();

	if (input->isKeyDown(aie::INPUT_KEY_ESCAPE))
		quit();
}

void PROJECTApp::draw()
{


#pragma region //Init
	// wipe the screen to the background colour
	clearScreen();

	// Bind our target
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glViewport(0, 0, this->getWindowWidth(), this->getWindowHeight());
	// Clear the target
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// update perspective in case window resized
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f,
		getWindowWidth() / (float)getWindowHeight(),
		0.1f, 1000.f);
#pragma endregion

#pragma region //Shader

	// STEP 1: enable the shader program for rendering
	glUseProgram(m_shader);

	// Step 2: send uniform variables to the shader
	glm::mat4 projectionView = m_projectionMatrix * m_Camera->GetView();
	glUniformMatrix4fv(
		glGetUniformLocation(m_shader, "projectionView"),
		1,
		false,
		glm::value_ptr(projectionView));

	//Setup Texture in OpenGL
	//Select the first texture as active, then bind it
	// also set it up as a uniform variable for the shader
	SetupTex("texture", 0, m_shader);

	//Set up the grass texture
	SetupTex("grass", 1, m_shader);

	//Set up the sand texture
	SetupTex("sand", 2, m_shader);

	//Set up the snow texture
	SetupTex("snow", 3, m_shader);

	//Set up the rock texture
	SetupTex("rock", 4, m_shader);

	// Setup the light/color information
	glUniform3fv(glGetUniformLocation(m_shader, "LightPosition"), 1, &m_LightPosition[0]);
	glUniform3fv(glGetUniformLocation(m_shader, "LightColor"), 1, &m_LightColor[0]);
	glUniform3fv(glGetUniformLocation(m_shader, "LightSpecColor"), 1, &m_LightSpecColor[0]);
	glUniform1fv(glGetUniformLocation(m_shader, "LightAmbientStrength"), 1, &m_LightAmbientStrength);

	glUniform3fv(glGetUniformLocation(m_shader, "CameraPosition"), 1, &m_Camera->GetPosition()[0]);
	glUniform1fv(glGetUniformLocation(m_shader, "SpecLightPower"), 1, &m_SpecStrength);

	// Step 3: Bind the VAO
	// When we setup the geometry, we did a bunch of glEnableVertexAttribArray and glVertexAttribPointer method calls
	// we also Bound the vertex array and index array via the glBindBuffer call.
	// if we where not using VAO's we would have to do thoes method calls each frame here.
	glBindVertexArray(m_Vao);

	// Step 4: Draw Elements. We are using GL_verts.
	// we need to tell openGL how many indices there are, and the size of our indices
	// when we setup the geometry, our indices where an unsigned char (1 byte for each indicy)

	glDrawElements(GL_TRIANGLES, m_IndicesCount, GL_UNSIGNED_INT, 0);
	//DrawLandScape();

	// Step 5: Now that we are done drawing the geometry
	// unbind the vao, we are basicly cleaning the opengl state
	glBindVertexArray(0);

	// Step 6: de-activate the shader program, dont do future rendering with it any more.
	glUseProgram(0);


#pragma endregion

#pragma region //FBX LOADING

	FBXLoop(m_ModelShader, *m_FBXList.at(1), 0.5f);

#pragma endregion

#pragma region //FBX ANIMATION


	FBXLoop(m_AnimationShader, *m_FBXList.at(0), 0.05f, true);


#pragma endregion

#pragma region //Emitters

	glUseProgram(m_ParticleShader);
	int loc = glGetUniformLocation(m_ParticleShader, "ProjectionView");
	glUniformMatrix4fv(loc, 1, GL_FALSE,
		glm::value_ptr(projectionView));

	for (unsigned int i = 0; i < m_EmitterList.size(); i++)
	{
		if (m_EmitterList.at(i)->m_TexActive == NULL)
		{
			m_EmitterList.at(i)->Draw();
		}
	}
	glUseProgram(0);

	glUseProgram(m_ParticleShaderImage);
	SetupTex("PTex", 5, m_ParticleShaderImage);
	glUniformMatrix4fv(glGetUniformLocation(m_ParticleShaderImage, "ProjectionView"), 1, GL_FALSE,
		glm::value_ptr(projectionView));


	for (unsigned int i = 0; i < m_EmitterList.size(); i++)
	{
		if (m_EmitterList.at(i)->m_TexActive != NULL)
		{
			m_EmitterList.at(i)->Draw();
		}
	}
	glUseProgram(0);

#pragma endregion

#pragma region //Frustum Culling
	Gizmos::clear();
	BoundingSphere sphere;
	sphere.centre = vec3(0, 0, 0);
	sphere.radius = 0.5f;
	vec4 planes[6];
	GetFrustumPlanes((m_Camera->GetProjection() * m_Camera->GetView()), planes);

	Gizmos::addSphere(sphere.centre, sphere.radius, 8, 8, vec4(1, 0, 1, 1));

	visible = false;

	for (int i = 0; i < 6; i++)
	{
		float d = glm::dot(vec3(planes[i]), sphere.centre) + planes[i].w;

		if (d < -sphere.radius)
		{
		
			visible = false;
			break;
		}
		else if (d < sphere.radius)
		{
			//visible = true;
	
		}
		else
		{
			visible = true;

		}
	}


#pragma endregion

#pragma region //Gizmos

	Gizmos::addSphere(vec3(m_LightPosition.x, m_LightPosition.y, m_LightPosition.z),
		LightSphereSize, 10, 10, glm::vec4(1, 1, 1, 0.5));
	Gizmos::draw(m_projectionMatrix * m_Camera->GetView());
#pragma endregion
	
#pragma region //Post Processing
	// Bind the back buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, this->getWindowWidth(), this->getWindowHeight());

	//just clear the back-buffer depth as each pixel will be filled.
	glClear(GL_DEPTH_BUFFER_BIT);

	//Draw our fullscreen quad
	glUseProgram(m_PostProcessingShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_fboTexture);

	glUniform1i(glGetUniformLocation(m_PostProcessingShader, "target"), 0);

	glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUseProgram(0);
#pragma endregion

	
}

const int PROJECTApp::GetWindowWidth()
{
	return WindowWidth;
}

const int PROJECTApp::GetWindowHeight()
{
	return WindowHeight;
}

void PROJECTApp::SetupFrameBuffer()
{
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	glGenTextures(1, &m_fboTexture);
	glBindTexture(GL_TEXTURE_2D, m_fboTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, this->getWindowWidth(), this->getWindowHeight());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_fboTexture, 0);
	glGenRenderbuffers(1, &m_fboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, m_fboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,this->getWindowWidth(),this->getWindowHeight());
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_fboDepth);

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PROJECTApp::SetupQuad()
{
	glm::vec2 texelSize = 1.0f / glm::vec2(WindowWidth, WindowHeight);
	glm::vec2 HalfTexelSize = 1.0f / glm::vec2(WindowWidth, WindowHeight) * 0.5f;

	float m_vertexData[] =
	{
		-1,-1, 0, 1,     HalfTexelSize.x,     HalfTexelSize.y,
		1, 1, 0, 1, 1 - HalfTexelSize.x, 1 - HalfTexelSize.y,
		-1, 1, 0, 1,     HalfTexelSize.x, 1 - HalfTexelSize.y,

		-1,-1, 0, 1,     HalfTexelSize.x,     HalfTexelSize.y,
		1,-1, 0, 1, 1 - HalfTexelSize.x,     HalfTexelSize.y,
		1, 1, 0, 1, 1 - HalfTexelSize.x, 1 - HalfTexelSize.y
	};

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 6, m_vertexData, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 6,0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 6, ((char*)0) + 16);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PROJECTApp::CreateFBXOpenGLBuffers(FBXFile * file)
{
	// FBX Files contain multiple meshes, each with seperate material information
	// loop through each mesh within the FBX file and cretae VAO, VBO and IBO buffers for each mesh.
	// We can store that information within the mesh object via its "user data" void pointer variable.
	for (unsigned int i = 0; i < file->getMeshCount(); i++)
	{
		//Get current mesh from file
		FBXMeshNode *fbxMesh = file->getMeshByIndex(i);

		GLMesh *glData = new GLMesh();

		glGenVertexArrays(1, &glData->vao);
		glBindVertexArray(glData->vao);

		glGenBuffers(1, &glData->vbo);
		glGenBuffers(1, &glData->ibo);

		glBindBuffer(GL_ARRAY_BUFFER, glData->vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glData->ibo);

		//Fill the vbo with our vertices.
		// the FBXLoader has conveniently already defined a vertex structure for us.
		glBufferData
		(
			GL_ARRAY_BUFFER,
			fbxMesh->m_vertices.size() * sizeof(FBXVertex),
			fbxMesh->m_vertices.data(), GL_STATIC_DRAW
		);

		// fill the ibo with the indices.
		// fbx meshes can be large, so indices are stored as an unsigned int.
		glBufferData
		(
			GL_ELEMENT_ARRAY_BUFFER,
			fbxMesh->m_indices.size() * sizeof(unsigned int),
			fbxMesh->m_indices.data(), GL_STATIC_DRAW
		);

		// Setup Vertex Attrib pointers
		//Remember, we only need to setup the appropriate attributes for the shaders that will be rendering this fbx object
		glEnableVertexAttribArray(0); // position
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), 0);

		glEnableVertexAttribArray(1);//normal
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), ((char*)0) + FBXVertex::NormalOffset);

		glEnableVertexAttribArray(2);// uv
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), ((char*)0) + FBXVertex::TexCoord1Offset);

		// TODO: add any additional attribute pointers required for shader use. ??

		// Unbind
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//Attach our GLMesh Object to the m_userData pointer.
		fbxMesh->m_userData = glData;

	}
}

void PROJECTApp::CreateFBXOpenGLBuffers(FBXFile * file, bool additionalAtribs)
{
	// FBX Files contain multiple meshes, each with seperate material information
	// loop through each mesh within the FBX file and cretae VAO, VBO and IBO buffers for each mesh.
	// We can store that information within the mesh object via its "user data" void pointer variable.
	for (unsigned int i = 0; i < file->getMeshCount(); i++)
	{
		//Get current mesh from file
		FBXMeshNode *fbxMesh = file->getMeshByIndex(i);

		GLMesh *glData = new GLMesh();

		glGenVertexArrays(1, &glData->vao);
		glBindVertexArray(glData->vao);

		glGenBuffers(1, &glData->vbo);
		glGenBuffers(1, &glData->ibo);

		glBindBuffer(GL_ARRAY_BUFFER, glData->vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glData->ibo);

		//Fill the vbo with our vertices.
		// the FBXLoader has conveniently already defined a vertex structure for us.
		glBufferData
			(
				GL_ARRAY_BUFFER,
				fbxMesh->m_vertices.size() * sizeof(FBXVertex),
				fbxMesh->m_vertices.data(), GL_STATIC_DRAW
				);

		// fill the ibo with the indices.
		// fbx meshes can be large, so indices are stored as an unsigned int.
		glBufferData
			(
				GL_ELEMENT_ARRAY_BUFFER,
				fbxMesh->m_indices.size() * sizeof(unsigned int),
				fbxMesh->m_indices.data(), GL_STATIC_DRAW
				);

		// Setup Vertex Attrib pointers
		//Remember, we only need to setup the appropriate attributes for the shaders that will be rendering this fbx object
		glEnableVertexAttribArray(0); // position
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (void*)FBXVertex::PositionOffset);

		glEnableVertexAttribArray(1);//normal
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), (void*)FBXVertex::NormalOffset);

		glEnableVertexAttribArray(2);// tangents
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), (void*)FBXVertex::TangentOffset);

		glEnableVertexAttribArray(3);// texcoords
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (void*)FBXVertex::TexCoord1Offset);

		glEnableVertexAttribArray(4);// weights
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (void*)FBXVertex::WeightsOffset);

		glEnableVertexAttribArray(5);// indices
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (void*)FBXVertex::IndicesOffset);

		// TODO: add any additional attribute pointers required for shader use. ??

		// Unbind
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//Attach our GLMesh Object to the m_userData pointer.
		fbxMesh->m_userData = glData;
	}
}

void PROJECTApp::CleanupFBXOpenGLBuffers(FBXFile * file)
{
	for (unsigned int i = 0; i < file->getMeshCount(); i++)
	{
		FBXMeshNode *fbxMesh = file->getMeshByIndex(i);
		GLMesh *glData = (GLMesh *)fbxMesh->m_userData;

		glDeleteVertexArrays(1, &glData->vao);
		glDeleteBuffers(1, &glData->vbo);
		glDeleteBuffers(1, &glData->ibo);

		delete glData;

	}
}

void PROJECTApp::GetFrustumPlanes(const glm::mat4 & transform, glm::vec4 * planes)
{
	// Right side
	planes[0] = vec4(
		transform[0][3] - transform[0][0],
		transform[1][3] - transform[1][0],
		transform[2][3] - transform[2][0],
		transform[3][3] - transform[3][0]
	);
	// Left side
	planes[1] = vec4(
		transform[0][3] + transform[0][0],
		transform[1][3] + transform[1][0],
		transform[2][3] + transform[2][0],
		transform[3][3] + transform[3][0]
	);
	// Top
	planes[2] = vec4(
		transform[0][3] - transform[0][1],
		transform[1][3] - transform[1][1],
		transform[2][3] - transform[2][1],
		transform[3][3] - transform[3][1]
	);
	// Bottom
	planes[3] = vec4(
		transform[0][3] + transform[0][1],
		transform[1][3] + transform[1][1],
		transform[2][3] + transform[2][1],
		transform[3][3] + transform[3][1]
	);
	// Far
	planes[4] = vec4(
		transform[0][3] - transform[0][2],
		transform[1][3] - transform[1][2],
		transform[2][3] - transform[2][2],
		transform[3][3] - transform[3][2]
	);
	// Near
	planes[5] = vec4(
		transform[0][3] + transform[0][2],
		transform[1][3] + transform[1][2],
		transform[2][3] + transform[2][2],
		transform[3][3] + transform[3][2]
	);

	for (int i = 0; i < 6; i++)
	{
		float d = glm::length(vec3(planes[i]));
		planes[i] /= d;
	}
}

void PROJECTApp::LoadShader()
{
#pragma region //m_shader


	static const char* vertex_shader =
		"#version 400\n									\
	in vec4 vPosition;\n								\
	in vec2 vUv;\n										\
	in vec4 vNormal;\n									\
														\
	out vec3 fPos;\n									\
	out vec4 fNormal;\n									\
	out vec2 fUv;\n										\
	uniform mat4 projectionView; \n						\
	void main ()\n										\
	{\n													\
		fPos = vPosition.xyz;\n							\
		fNormal = vNormal;\n							\
		fUv = vUv;\n									\
		gl_Position = projectionView * vPosition;\n		\
	}";

	static const char* fragment_shader =
		"#version 400\n																			\
	in vec2 fUv;\n																				\
	in vec3 fPos;\n																				\
	in vec4 fNormal;\n																			\
																								\
	out vec4 frag_color;\n																		\
																								\
	uniform sampler2D sand;\n																	\
	uniform sampler2D rock;\n																	\
	uniform sampler2D grass;\n																	\
	uniform sampler2D snow;\n																	\
																								\
	uniform float LightAmbientStrength;\n														\
	uniform float SpecLightPower;\n																\
																								\
	uniform vec3	LightPosition;\n															\
	uniform vec3	LightColor;\n																\
	uniform vec3	LightSpecColor;\n															\
	uniform vec3	CameraPosition;\n															\
																								\
	void main ()\n																				\
	{\n																							\
		vec3 Norm = normalize(fNormal.xyz);\n													\
		vec3 LightDir = normalize(LightPosition - fPos);\n										\
																								\
		vec3 Reflected = reflect(-LightDir,Norm);\n												\
		vec3 SurfaceToEye = normalize(CameraPosition - fPos);\n									\
																								\
		float diff = max(dot(Norm, LightDir),0.0f);\n											\
																								\
		vec3 diffColor = diff * LightColor;\n													\
		vec3 ambient = LightColor * LightAmbientStrength;\n										\
																								\
		float SpecTerm = pow(max(0.0f, dot(Reflected, SurfaceToEye)), SpecLightPower);\n		\
		vec3 Specular = LightSpecColor * vec3(1,1,1) * SpecTerm;\n								\
																								\
		vec4 tempOne;\n																			\
		vec4 tempTwo;\n																			\
		vec4 texColor;\n																		\
		tempOne = mix(texture2D(grass,fUv *4),texture2D(rock,fUv*4),(fPos.y));\n						\
		tempTwo = mix(texture2D(sand,fUv*4),texture2D(snow,fUv*4),(fPos.y));\n						\
		if(fPos.y >= 3.0f)																		\
		{																						\
		texColor = mix(tempOne,tempTwo,(fPos.y) / 3);\n											\
		}																						\
		if(fPos.y < 3.0f)																		\
		{																						\
		texColor = mix(tempOne,tempTwo,(fPos.y) / 4);\n											\
		}																						\
		if(fPos.y < 3.0f)																		\
		{																						\
		texColor = mix(tempOne,tempTwo,(fPos.y) / 2);\n											\
		}																						\
		if(fPos.y < 3.0f)																		\
		{																						\
		texColor = mix(tempOne,tempTwo,(fPos.y) / 1);\n											\
		}																						\
		frag_color = texColor * vec4(ambient + diffColor + Specular, 1.0f);\n					\
	}";


	//uniform sampler2D texture;\n		


	// Step 1:
	// Load the vertex shader, provide it with the source code and compile it.
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, NULL);
	glCompileShader(vs);
	
	// Step 2:
	// Load the fragment shader, provide it with the source code and compile it.
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader, NULL);
	glCompileShader(fs);

	// step 3:
	// Create the shader program
	m_shader = glCreateProgram();

	// Step 4:
	// attach the vertex and fragment shaders to the m_shader program
	glAttachShader(m_shader, vs);
	glAttachShader(m_shader, fs);

	// Step 5:
	// describe the location of the shader inputs the link the program
	glBindAttribLocation(m_shader, 0, "vPosition");
	glBindAttribLocation(m_shader, 1, "vUv");
	glBindAttribLocation(m_shader, 2, "vNormal");
	glLinkProgram(m_shader);

	// step 6:
	// delete the vs and fs shaders
	glDeleteShader(vs);
	glDeleteShader(fs);

#pragma endregion

#pragma region //Model Shader
	// Model Shader
	const char* vsSource =
		"#version 410\n												\
		in vec4 position;\n											\
		in vec4 normal;\n											\
		in vec2 uv;\n 												\
		out vec4 vnormal;\n											\
		out vec2 vuv;\n												\
		uniform mat4 ProjectionViewWorldMatrix;\n					\
																	\
		void main()\n												\
		{\n															\
			vnormal = normal;\n										\
			vuv = uv;\n												\
			gl_Position = ProjectionViewWorldMatrix * position;\n	\
																	\
		}\n															\
		";

	const char* fsSource =
		"#version 410\n														\
		in vec4 vnormal;\n													\
		in vec2 vuv;\n														\
		out vec4 FragColor;\n												\
		uniform sampler2D diffuseTexture;\n									\
		void main()\n														\
		{\n																	\
		FragColor = texture2D(diffuseTexture, vuv) * vec4(1,1,1,1);\n		\
		}\n																	\
		";

	GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShader, 1, &vsSource, 0);
	glCompileShader(VertexShader);

	GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShader, 1, &fsSource, 0);
	glCompileShader(FragmentShader);

	m_ModelShader = glCreateProgram();

	glAttachShader(m_ModelShader, VertexShader);
	glAttachShader(m_ModelShader, FragmentShader);

	glBindAttribLocation(m_ModelShader, 0, "position");
	glBindAttribLocation(m_ModelShader, 1, "normal");
	glBindAttribLocation(m_ModelShader, 2, "uv");

	glLinkProgram(m_ModelShader);

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);
#pragma endregion

#pragma region // Animation Shader

	const char* animationVertex =
		"#version 410\n													\
		in vec4 Aposition;\n											\
		in vec4 Anormal;\n												\
		in vec4 Atangent;\n												\
		in vec2 Atexcoord;\n											\
		in vec4 Aweights;\n												\
		in vec4 Aindices;\n												\
																		\
		out vec4 vNormal;\n												\
		out vec2 vuV;\n													\
																		\
		uniform mat4 AnimProjectionViewWorldMatrix;\n					\
		uniform mat4 AnimModel;\n										\
		// we need to give our bone array a limit\n						\
		const int MAX_BONES = 200;\n									\
		uniform mat4 bones[MAX_BONES];\n								\
																		\
		void main()\n													\
		{\n																\
		vNormal = Anormal;\n											\
		vuV = Atexcoord;\n												\
																		\
		ivec4 index = ivec4(Aindices);\n								\
																		\
		vec4 P =														\
			 bones[index.x] * Aposition * Aweights.x;\n					\
		P += bones[index.y] * Aposition * Aweights.y;\n					\
		P += bones[index.z] * Aposition * Aweights.z;\n					\
		P += bones[index.w] * Aposition * Aweights.w;\n					\
																		\
		gl_Position = AnimProjectionViewWorldMatrix * AnimModel * P;\n	\
																		\
		};																\
		";

	const char* animationFragment =
		"#version 410\n														\
		in vec4 vNormal;\n													\
		in vec2 vuV;\n														\
		out vec4 FragColor;\n												\
		uniform sampler2D diffuseTexture;\n									\
																			\
		void main()\n														\
		{\n																	\
			FragColor = texture2D(diffuseTexture, vuV) * vec4(1,1,1,1);\n	\
		};\n																	\
		";

	GLuint AnimVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(AnimVertexShader, 1, &animationVertex, 0);
	glCompileShader(AnimVertexShader);

	GLuint AnimFragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(AnimFragShader, 1, &animationFragment, 0);
	glCompileShader(AnimFragShader);

	m_AnimationShader = glCreateProgram();

	glAttachShader(m_AnimationShader, AnimVertexShader);
	glAttachShader(m_AnimationShader, AnimFragShader);

	glBindAttribLocation(m_AnimationShader, 0, "Aposition");
	glBindAttribLocation(m_AnimationShader, 1, "Anormal");
	glBindAttribLocation(m_AnimationShader, 2, "Atangent");
	glBindAttribLocation(m_AnimationShader, 3, "Atexcoord");
	glBindAttribLocation(m_AnimationShader, 4, "Aweights");
	glBindAttribLocation(m_AnimationShader, 5, "Aindices");

	glLinkProgram(m_AnimationShader);

	int success = GL_TRUE;
	glGetProgramiv(m_AnimationShader, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		int infoLogLength = 0;
		glGetProgramiv(m_AnimationShader, GL_INFO_LOG_LENGTH, &infoLogLength);

		char* error = new char[infoLogLength + 1];
		glGetProgramInfoLog(m_AnimationShader, infoLogLength, 0, error);
	
		delete[] error;
	}

	glDeleteShader(AnimVertexShader);
	glDeleteShader(AnimFragShader);

#pragma endregion

#pragma region //Particle Shader

	const char* ParticleVertexShader =
		"#version 410\n									\
		in vec4 Position;\n								\
		in vec4 inColor;\n								\
														\
		out vec4 Color;\n								\
														\
		uniform mat4 ProjectionView;\n					\
														\
		void main()\n									\
		{\n												\
			Color = inColor;\n							\
			gl_Position = ProjectionView * Position;\n	\
														\
		};\n											";

	const char* ParticleFragmentShader =
		"#version 410\n									\
														\
		in vec4 Color;\n								\
		out	vec4 FragColor;\n							\
														\
		void main()\n									\
		{\n												\
			FragColor = Color;\n						\
		};\n											";

	GLuint PaVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(PaVertexShader, 1, &ParticleVertexShader, 0);
	glCompileShader(PaVertexShader);

	GLuint PaFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(PaFragmentShader, 1, &ParticleFragmentShader, 0);
	glCompileShader(PaFragmentShader);

	m_ParticleShader = glCreateProgram();

	glAttachShader(m_ParticleShader, PaVertexShader);
	glAttachShader(m_ParticleShader, PaFragmentShader);

	glBindAttribLocation(m_ParticleShader, 0, "Position");
	glBindAttribLocation(m_ParticleShader, 1, "inColor");

	glLinkProgram(m_ParticleShader);

	int successs = GL_TRUE;
	glGetProgramiv(m_ParticleShader, GL_LINK_STATUS, &successs);
	if (successs == GL_FALSE)
	{
		int infoLogLength = 0;
		glGetProgramiv(m_ParticleShader, GL_INFO_LOG_LENGTH, &infoLogLength);

		char* error = new char[infoLogLength + 1];
		glGetProgramInfoLog(m_ParticleShader, infoLogLength, 0, error);

		delete[] error;
	}

	glDeleteShader(PaVertexShader);
	glDeleteShader(PaFragmentShader);

#pragma endregion

#pragma region //Particles with image.

	const char* ParticleVertexShaderImage =
		"#version 410\n									\
		in vec4 Position;\n								\
		in vec4 inColor;\n								\
														\
		in vec2 TvUv;\n									\
		in vec4 TvNormal;\n								\
														\
		out vec4 Color;\n		 						\
		out vec4 TfNormal;\n							\
		out vec2 TfUv;\n								\
														\
		uniform mat4 ProjectionView;\n					\
														\
		void main()\n									\
		{\n												\
			TfUv = TvUv;\n								\
			TfNormal = TvNormal;\n						\
														\
														\
			Color = inColor;\n							\
			gl_Position = ProjectionView * Position;\n	\
														\
		};\n											";

	const char* ParticleFragmentShaderImage =
		"#version 410\n									\
														\
		in vec4 Color;\n								\
														\
		in vec4 TfNormal;\n								\
		in vec2 TfUv;\n									\
				out	vec4 FragColor;\n					\
		uniform sampler2D PTex;\n						\
											\
		void main()\n									\
		{\n												\
			FragColor = (Color * vec4(TfUv,0,0));\n					\
		};\n											";

	GLuint TPaVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(TPaVertexShader, 1, &ParticleVertexShaderImage, 0);
	glCompileShader(TPaVertexShader);

	GLuint TPaFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(TPaFragmentShader, 1, &ParticleFragmentShaderImage, 0);
	glCompileShader(TPaFragmentShader);

	m_ParticleShaderImage = glCreateProgram();

	glAttachShader(m_ParticleShaderImage, TPaVertexShader);
	glAttachShader(m_ParticleShaderImage, TPaFragmentShader);

	glBindAttribLocation(m_ParticleShaderImage, 0, "Position");
	glBindAttribLocation(m_ParticleShaderImage, 1, "inColor");
	glBindAttribLocation(m_ParticleShaderImage, 3, "TvUv");
	glBindAttribLocation(m_ParticleShaderImage, 4, "TvNormal");

	glLinkProgram(m_ParticleShaderImage);

	successs = GL_TRUE;
	glGetProgramiv(m_ParticleShaderImage, GL_LINK_STATUS, &successs);
	if (successs == GL_FALSE)
	{
		int infoLogLength = 0;
		glGetProgramiv(m_ParticleShaderImage, GL_INFO_LOG_LENGTH, &infoLogLength);

		char* error = new char[infoLogLength + 1];
		glGetProgramInfoLog(m_ParticleShaderImage, infoLogLength, 0, error);

		delete[] error;
	}

	glDeleteShader(TPaVertexShader);
	glDeleteShader(TPaFragmentShader);

#pragma endregion

#pragma region //PostProcessing Shader
	const char* PPVertShader =
		"#version 410\n\
in vec4 position;\n\
in vec2 texCoord;\n\
out vec2 fTexCoord;\n\
void main()\n\
{\n\
\n\
gl_Position = position;\n\
fTexCoord = texCoord;\n\
}";

	const char* PPFragShader =
		"#version 410\n\
in vec2 fTexCoord;\n\
out vec4 fragColor;\n\
uniform sampler2D target;\n\
vec4 Simple()\n\
{\
return texture(target, fTexCoord);\n\
};\
\
vec4 BoxBlur()\
{\
vec2 texel = 1.0f / textureSize(target,0).xy;\n\
\
vec4 color = texture(target,fTexCoord);\
color += texture(target, fTexCoord + vec2(-texel.x, texel.y));\
color += texture(target, fTexCoord + vec2(-texel.x, 0));\
color += texture(target, fTexCoord + vec2(-texel.x, -texel.y));\
color += texture(target, fTexCoord + vec2(0, texel.y));\
color += texture(target, fTexCoord + vec2(0, -texel.y));\
color += texture(target, fTexCoord + vec2(texel.x, texel.y));\
color += texture(target, fTexCoord + vec2(texel.x, 0));\
color += texture(target, fTexCoord + vec2(texel.x, -texel.y));\
return color / 9;\n\
};\
vec4 Distort()\
{\
vec2 mid = vec2(0.5f);\
float distanceFromCentre = distance(fTexCoord,mid);\
vec2 normalizedCoord = normalize(fTexCoord - mid);\
float bias = distanceFromCentre + sin(distanceFromCentre * 15) * 0.02f;\
vec2 newCoord = mid + bias * normalizedCoord;\
return texture(target,newCoord);\
}\
void main()\n\
{\
fragColor = Distort();\n\
}\
";

	GLuint PPVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(PPVertexShader, 1, &PPVertShader, 0);
	glCompileShader(PPVertexShader);

	GLuint PPFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(PPFragmentShader, 1, &PPFragShader, 0);
	glCompileShader(PPFragmentShader);

	m_PostProcessingShader = glCreateProgram();

	glAttachShader(m_PostProcessingShader, PPVertexShader);
	glAttachShader(m_PostProcessingShader, PPFragmentShader);

	glBindAttribLocation(m_PostProcessingShader, 0, "position");
	glBindAttribLocation(m_PostProcessingShader, 1, "texCoord");

	glLinkProgram(m_PostProcessingShader);

	successs = GL_TRUE;
	glGetProgramiv(m_PostProcessingShader, GL_LINK_STATUS, &successs);
	if (successs == GL_FALSE)
	{
		int infoLogLength = 0;
		glGetProgramiv(m_PostProcessingShader, GL_INFO_LOG_LENGTH, &infoLogLength);

		char* error = new char[infoLogLength + 1];
		glGetProgramInfoLog(m_PostProcessingShader, infoLogLength, 0, error);

		delete[] error;
	}

	glDeleteShader(PPVertexShader);
	glDeleteShader(PPFragmentShader);
#pragma endregion

}

void PROJECTApp::UnloadShader()
{
	glDeleteProgram(m_shader);
	glDeleteProgram(m_ModelShader);
	glDeleteProgram(m_ParticleShader);
	glDeleteProgram(m_ParticleShaderImage);
	glDeleteProgram(m_AnimationShader);
	glDeleteProgram(m_PostProcessingShader);
}

void PROJECTApp::CreateLandScape()
{
	std::vector<Vertex>verts;
	std::vector<glm::vec4>a_Normal;
	std::vector<unsigned int>indices;

	const unsigned char *pixels = m_MapList.at(0)->getPixels();

	// Create a grid of vertices
	for (int z = 0; z < m_LandLength; z++)
	{
		for (int x = 0; x < m_LandWidth; x++)
		{
			int sampleX = (int)((float)x / m_LandWidth  *  m_MapList.at(0)->getWidth());
			int sampleZ = (int)((float)z / m_LandLength *  m_MapList.at(0)->getHeight());
			int i = (int)(sampleZ * 					   m_MapList.at(0)->getWidth() + sampleX);

			//Position of vertex
			float xPos = (x * m_vertSeperation) - (m_LandWidth	* m_vertSeperation * 0.5f);
			float yPos = (pixels[i * 3] / 255.0f) * m_maxHeight;	// *3 is so we sample only one colour from the RGB bitmap
			float zPos = (z *m_vertSeperation) - (m_LandLength	* m_vertSeperation * 0.5f);

			float u = (float)x / (m_LandWidth - 1);
			float v = (float)z / (m_LandLength - 1);

			Vertex vert
			{
				{ xPos, yPos, zPos, 1.0f },	//Position
				{ u, v },					//Texture Coordinates(UV)
				{ 0.0f, 1.0f, 0.0f, 0.0f }	//normal
			};
			verts.push_back(vert);
		}

	}

	

	// calculate indices for verts
	for (int z = 0; z < m_LandLength - 1; z++)
	{
		for (int x = 0; x < m_LandWidth - 1; x++)
		{
			int i = z * m_LandLength + x; // The address of the vertices in the single dimension vector


										  //Triangulate
			indices.push_back(i + 1);				//  b--a
			indices.push_back(i);					//  | /
			indices.push_back(i + m_LandWidth);		//  c

													// B A C A
			glm::vec3 BACA = glm::cross((glm::vec3)verts.at(i).pos - (glm::vec3)verts.at(i + 1).pos,
				(glm::vec3)verts.at(i + m_LandWidth).pos - (glm::vec3)verts.at(i + 1).pos);

			// C B A B
			glm::vec3 CBAB = glm::cross((glm::vec3)verts[i + m_LandWidth].pos - (glm::vec3)verts.at(i).pos,
				(glm::vec3)verts.at(i + 1).pos - (glm::vec3)verts.at(i).pos);

			verts.at(i + 1).normal = glm::vec4(BACA, 0);
			verts.at(i).normal += glm::vec4(CBAB, 0);

			indices.push_back(i + 1);				//     a
			indices.push_back(i + m_LandWidth);		//   / |
			indices.push_back(i + m_LandWidth + 1); //  b--c

													// A C B C
			glm::vec3 ACBC = glm::cross((glm::vec3)verts.at(i + 1).pos - (glm::vec3)verts.at(i + m_LandWidth).pos,
				(glm::vec3)verts.at(i).pos - (glm::vec3)verts.at(i + m_LandWidth).pos);

			// A D C D
			glm::vec3 ADCD = glm::cross((glm::vec3)verts.at(i + 1).pos - (glm::vec3)verts.at(i + m_LandWidth + 1).pos,
				(glm::vec3)verts.at(i + m_LandWidth).pos - (glm::vec3)verts.at(i + m_LandWidth + 1).pos);

			verts.at(i + m_LandWidth).normal += glm::vec4(ACBC, 0);
			verts.at(i + m_LandWidth + 1).normal += glm::vec4(ADCD, 0);

		}
	}



	for (unsigned int i = 0; i < verts.size(); i++)
	{
		verts.at(i).normal = glm::normalize(verts.at(i).normal);
	}

	m_VertCount = verts.size();
	m_IndicesCount = indices.size();
	// Generate the VAO and Bind bind it.
	// Our VBO (vertex buffer object) and IBO (Index Buffer Object) will be "grouped" with this VAO
	// other settings will also be grouped with the VAO. this is used so we can reduce draw calls in the render method.
	glGenVertexArrays(1, &m_Vao);
	glBindVertexArray(m_Vao);

	// Create our VBO and IBO.
	// Then tell Opengl what type of buffer they are used for
	// VBO a buffer in graphics memory to contains our vertices
	// IBO a buffer in graphics memory to contain our indices.
	// Then Fill the buffers with our generated data.
	// This is taking our verts and indices from ram, and sending them to the graphics card
	glGenBuffers(1, &m_Vbo);
	glGenBuffers(1, &m_Ibo);

	glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ibo);

	glBufferData(GL_ARRAY_BUFFER, m_VertCount * sizeof(Vertex), &verts[0], GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_IndicesCount * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	Vertex::SetupVertexAttribPointers();

	// Unbind things after we've finished using them
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void PROJECTApp::Vertex::SetupVertexAttribPointers()
{
	// enable vertex position element
	// notice when we loaded the shader, we described the "position" element to be location 0.
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,                  // attribute 0 (position)
		4,                  // size - how many floats make up the position (x, y, z, w)
		GL_FLOAT,           // type - our x,y,z, w are float values
		GL_FALSE,           // normalized? - not used
		sizeof(Vertex),     // stride - size of an entire vertex
		(void*)0            // offset - bytes from the beginning of the vertex
	);

	// enable vertex UV element
	// notice when we loaded the shader, we described the "color" element to be location 1.
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,							// attribute 1 (Texture coordinates)
		2,							// size - how many floats make up the color (r, g, b, a)
		GL_FLOAT,					// type - our x,y,z are float values
		GL_FALSE,					// normalized? - not used
		sizeof(Vertex),				// stride - size of an entire vertex
		(void*)(sizeof(float) * 4)  // offset - bytes from the beginning of the vertex
	);

	//Enable vertex normal element
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2,							// attribute 2 (normal)
		4,							// size - how many floats make up the normal (x, y, z, w)
		GL_FLOAT,					// type - our x,y,z are float values
		GL_FALSE,					// normalized? - not used
		sizeof(Vertex),				// stride - size of an entire vertex
		(void*)(sizeof(float) * 6)  // offset - bytes from the beginning of the vertex
	);

	//Enable vertex color element
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(
		3,							// attribute 3 (color)
		4,							// size - how many floats make up the normal (x, y, z, w)
		GL_FLOAT,					// type - our x,y,z are float values
		GL_FALSE,					// normalized? - not used
		sizeof(Vertex),				// stride - size of an entire vertex
		(void*)(sizeof(float) * 8)  // offset - bytes from the beginning of the vertex
	);
}

void PROJECTApp::DestroyLandScape()
{
	// When We're Done, destroy the geometry
	glDeleteBuffers(1, &m_Ibo);
	glDeleteBuffers(1, &m_Vbo);
	glDeleteVertexArrays(1, &m_Vao);
}

void PROJECTApp::LoadTex(char* Location)
{	
	aie::Texture* TempTex;
	TempTex = new aie::Texture();
	TempTex->load(Location);

	m_TexList.push_back(TempTex);
	
}

void PROJECTApp::LoadMap(char * Location)
{
	aie::Texture* TempTex;
	TempTex = new aie::Texture();
	TempTex->load(Location);

	m_MapList.push_back(TempTex);
}

void PROJECTApp::SetupTex(GLchar* a_handle, int a_index, unsigned int a_shader)
{
	//Setup textures inside of the m_TexlList vector.
	int GLTEX = a_index > 31 ? 33984 : 33984 + a_index;
	glActiveTexture(GLTEX);
	glBindTexture(GL_TEXTURE_2D, m_TexList.at(a_index)->getHandle());
	glUniform1i(glGetUniformLocation(a_shader, a_handle), a_index);
}

void PROJECTApp::LoadFBX(char* Location)
{
	FBXFile* TempFBX;
	TempFBX = new FBXFile();
	TempFBX->load(Location, FBXFile::UNITS_CENTIMETER);
	m_FBXList.push_back(TempFBX);
	CreateFBXOpenGLBuffers(m_FBXList.at(m_FBXList.size()-1));
}

void PROJECTApp::LoadFBX(char * Location, bool anim)
{
	FBXFile* TempFBX;
	TempFBX = new FBXFile();
	TempFBX->load(Location, FBXFile::UNITS_CENTIMETER);
	m_FBXList.push_back(TempFBX);
	CreateFBXOpenGLBuffers(m_FBXList.at(m_FBXList.size() - 1),true);
}

void PROJECTApp::UnloadTex()
{
	m_TexList.clear();
	//GET REKt
}

void PROJECTApp::UnloadMap()
{
	m_MapList.clear();
	// DEStRoyED
}

void PROJECTApp::UnloadFBX()
{
	for (unsigned int i = 0; i < m_FBXList.size(); i++)
	{
		CleanupFBXOpenGLBuffers(m_FBXList.at(i));
		m_FBXList.at(i)->unload();
		delete m_FBXList.at(i);
	}
	m_FBXList.clear();
}

void PROJECTApp::FBXLoop(unsigned int a_Shader, FBXFile& a_Model, float a_scale)
{
	glm::mat4 model = glm::mat4
	(
		a_scale, 0, 0, 0,
		0, a_scale, 0, 0,
		0, 0, a_scale, 0,
		0, 0, 0, 1
	);

	glm::mat4 modelViewProjection = m_projectionMatrix * m_Camera->GetView() * model;

	glUseProgram(a_Shader);

	//Send Uniform variables, in this case the "projection view matrix"
	unsigned int mvpLoc = glGetUniformLocation(a_Shader, "ProjectionViewWorldMatrix");
	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &modelViewProjection[0][0]);

	// Loop through each mesh within the fbx file.
	for (unsigned int i = 0; i < a_Model.getMeshCount(); ++i)
	{
		FBXMeshNode *mesh = a_Model.getMeshByIndex(i);
		GLMesh* glData = (GLMesh*)mesh->m_userData;

		//get the texture from the model
		unsigned int diffuseTexture = mesh->m_material->textureIDs[mesh->m_material->DiffuseTexture];

		//Bind the texture and send it to our shader
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseTexture);
		glUniform1i(glGetUniformLocation(a_Shader, "diffuseTexture"), 0);

		//Draw the Mesh
		glBindVertexArray(glData->vao);
		glDrawElements(GL_TRIANGLES, mesh->m_indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
	glUseProgram(0);
}

void PROJECTApp::FBXLoop(unsigned int a_Shader, FBXFile & a_Model, float a_scale, bool a_Skeleton)
{
	glm::mat4 model = glm::mat4
		(
			a_scale, 0, 0, 0,
			0, a_scale, 0, 0,
			0, 0, a_scale, 0,
			0, 0, 0, 1
			);

	glm::mat4 modelViewProjection = m_projectionMatrix * m_Camera->GetView();

	if (m_renderwireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	glUseProgram(a_Shader);

	//Send Uniform variables, in this case the "projection view matrix"
	glUniformMatrix4fv(glGetUniformLocation(a_Shader, "AnimProjectionViewWorldMatrix"), 1, GL_FALSE, &modelViewProjection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(a_Shader, "AnimModel"), 1, GL_FALSE, &model[0][0]);

	//Grab the skeleton and animation we want to use.
	FBXSkeleton* skeleton = a_Model.getSkeletonByIndex(0);
	skeleton->updateBones();
	
	int bones_location = glGetUniformLocation(a_Shader, "bones");
	glUniformMatrix4fv(bones_location, skeleton->m_boneCount, GL_FALSE, (float*)skeleton->m_bones);

	if (m_renderbones)
	{
		for (unsigned int i = 0; i < skeleton->m_boneCount; i++)
		{
			glm::vec3 scale;
			glm::quat rotation;
			glm::vec3 translation;
			glm::vec3 skew;
			glm::vec4 perspective;

			glm::decompose(skeleton->m_nodes[i]->m_globalTransform * model, scale, rotation, translation, skew, perspective);

			Gizmos::addAABBFilled(translation * a_scale, scale, glm::vec4(1, 0, 0, 0.5f));
		}
	}

	// Loop through each mesh within the fbx file.
	for (unsigned int i = 0; i < a_Model.getMeshCount(); ++i)
	{
		FBXMeshNode *mesh = a_Model.getMeshByIndex(i);
		GLMesh* glData = (GLMesh*)mesh->m_userData;

		//get the texture from the model
		unsigned int TdiffuseTexture = mesh->m_material->textureIDs[mesh->m_material->DiffuseTexture];

		//Bind the texture and send it to our shader
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TdiffuseTexture);
		glUniform1i(glGetUniformLocation(a_Shader, "diffuseTexture"), 0);

		//Draw the Mesh
		glBindVertexArray(glData->vao);
		glDrawElements(GL_TRIANGLES, mesh->m_indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
	glUseProgram(0);
	Gizmos::draw(m_projectionMatrix * m_Camera->GetView());
}

void PROJECTApp::LoadFBXAnimations( std::string a_String[])
{
	static const int Max_anims = 100;
	FBXFile* TempFBX[Max_anims];
	int numfiles = sizeof(a_String) / sizeof(std::string);

	for (int i = 0; i < numfiles; i++)
	{
		TempFBX[i] = new FBXFile();
		TempFBX[i]->loadAnimationsOnly(a_String[i].c_str(), FBXFile::UNITS_CENTIMETER);
	}

	m_AnimationList.push_back(TempFBX);
}

void PROJECTApp::UpdateFBXAnimation(FBXFile* a_model, FBXFile* a_anims)
{

	// Spooky scary Skeletons
	// Grab the skeleton and animation we want to use
	FBXSkeleton* skeleton = a_model->getSkeletonByIndex(0);
	FBXAnimation* animation = a_anims->getAnimationByIndex(0);

	skeleton->evaluate(animation, m_AnimationTimer);

	// Evaluate the animation to update bones
	for (unsigned int bone_index = 0; bone_index < skeleton->m_boneCount;
		bone_index++)
	{
		skeleton->m_nodes[bone_index]->updateGlobalTransform();
	}

}

void PROJECTApp::LoadEmitter(int EmitRate, int MaxParticles, float LifeTimeMin, float LifetimeMax,
	float VelocityMin, float VelocityMax, float StartSize, float EndSize, glm::vec4 StartColor, 
	glm::vec4 EndColor)
{
	ParticleEmitter* TempEmit;
	TempEmit = new ParticleEmitter();
	TempEmit->Initialise
	(
		EmitRate, MaxParticles,
		LifeTimeMin, LifetimeMax,
		VelocityMin, VelocityMax,
		StartSize, EndSize,
		StartColor, EndColor
	);

	m_EmitterList.push_back(TempEmit);

}

void PROJECTApp::UnloadEmitter()
{
	m_EmitterList.clear();
}