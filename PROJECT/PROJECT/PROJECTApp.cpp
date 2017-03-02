#include "PROJECTApp.h"
#include "Particle.h"

using glm::vec3;
using glm::vec4;
using glm::mat4;
using aie::Gizmos;

PROJECTApp::PROJECTApp() {

}

PROJECTApp::~PROJECTApp() {

}

bool PROJECTApp::startup() {
	
	setBackgroundColour(0.0f, 0.2f, 0.5f);

	//Camera
	m_Camera = new Camera();
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
	m_SpecStrength = 0.5f;
	m_LightSpecColor = glm::vec3(0.5f, 0.5f, 0.5f);

	//Load texture and heightmap
	//Textures
	LoadTex("textures/Tile.png");
	LoadTex("textures/grass.png");
	LoadTex("textures/sand.png");
	LoadTex("textures/snow.png");
	LoadTex("textures/rock.png");
	// Heightmap
	LoadMap("textures/HM.bmp");

	//Load models



	m_myFbxModel = new FBXFile();
	m_myFbxModel->load("./models/Golden Oaks Library/Golden Oaks Library.Fbx", FBXFile::UNITS_CENTIMETER);
	CreateFBXOpenGLBuffers(m_myFbxModel);

	//Load emitters 

	m_Emitter = new ParticleEmitter();
	m_Emitter->Initialise
	(
		1000, 500,
		0.1f, 5.0f,
		1, 5,
		1, 0.1f,
		glm::vec4(1, 0, 0, 1), glm::vec4(1, 1, 0, 1)
	);
	 //

	LoadShader();
	CreateLandScape();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}

void PROJECTApp::shutdown()
{
	UnloadTex();
	UnloadMap();
	CleanupFBXOpenGLBuffers(m_myFbxModel);
	m_myFbxModel->unload();
	delete m_myFbxModel;
	Gizmos::destroy();
}

void PROJECTApp::update(float deltaTime)
{


	// query time since application started
	float time = getTime();

	// rotate camera
	m_Camera->Update(deltaTime);

	m_Emitter->Update(deltaTime,*m_Camera);
	
	// wipe the gizmos clean for this frame
	Gizmos::clear();

	// quit if we press escape
	aie::Input* input = aie::Input::getInstance();

	if (input->isKeyDown(aie::INPUT_KEY_ESCAPE))
		quit();
}

void PROJECTApp::draw()
{
	// wipe the screen to the background colour
	clearScreen();

	// update perspective in case window resized
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f,
		getWindowWidth() / (float)getWindowHeight(),
		0.1f, 1000.f);

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

	Gizmos::draw(m_projectionMatrix * m_Camera->GetView());
#pragma endregion

#pragma region //FBX LOADING
	// FBX loading
	float s = 1.0f;
	glm::mat4 model = glm::mat4
	(
		s, 0, 0, 0,
		0, s, 0, 0,
		0, 0, s, 0,
		0, 0, 0, 1
	);

	glm::mat4 modelViewProjection = m_projectionMatrix * m_Camera->GetView() * model;

	glUseProgram(m_ModelShader);

	//Send Uniform variables, in this case the "projection view matrix"
	unsigned int mvpLoc = glGetUniformLocation(m_ModelShader, "ProjectionViewWorldMatrix");
	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &modelViewProjection[0][0]);

	// Loop through each mesh within the fbx file.
	for (unsigned int i = 0; i < m_myFbxModel->getMeshCount(); ++i)
	{
		FBXMeshNode *mesh = m_myFbxModel->getMeshByIndex(i);
		GLMesh* glData = (GLMesh*)mesh->m_userData;

		//get the texture from the model
		unsigned int diffuseTexture = mesh->m_material->textureIDs[mesh->m_material->DiffuseTexture];

		//Bind the texture and send it to our shader
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseTexture);
		glUniform1i(glGetUniformLocation(m_ModelShader, "diffuseTexture"), 0);

		//Draw the Mesh
		glBindVertexArray(glData->vao);
		glDrawElements(GL_TRIANGLES, mesh->m_indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
	glUseProgram(0);
#pragma endregion

#pragma region //Emitters

	glUseProgram(m_ParticleShader);
	int loc = glGetUniformLocation(m_ParticleShader, "ProjectionView");
	glUniformMatrix4fv(loc, 1, GL_FALSE,
		glm::value_ptr(projectionView));
	m_Emitter->Draw();
	glUseProgram(0);


#pragma endregion

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

void PROJECTApp::LoadShader()
{
#pragma region //m_shader
	std::cout << "Started Loadshader" << std::endl;

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
		tempOne = mix(texture2D(grass,fUv),texture2D(rock,fUv),(fPos.y));\n						\
		tempTwo = mix(texture2D(sand,fUv),texture2D(snow,fUv),(fPos.y));\n						\
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
	std::cout << "after shader" << std::endl;

	// Step 1:
	// Load the vertex shader, provide it with the source code and compile it.
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, NULL);
	glCompileShader(vs);
	std::cout << "Finished Step 1" << std::endl;
	// Step 2:
	// Load the fragment shader, provide it with the source code and compile it.
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader, NULL);
	glCompileShader(fs);
	std::cout << "Finished Step 2" << std::endl;
	// step 3:
	// Create the shader program
	m_shader = glCreateProgram();
	std::cout << "Finished Step 3" << std::endl;
	// Step 4:
	// attach the vertex and fragment shaders to the m_shader program
	glAttachShader(m_shader, vs);
	glAttachShader(m_shader, fs);
	std::cout << "Finished Step 4" << std::endl;
	// Step 5:
	// describe the location of the shader inputs the link the program
	glBindAttribLocation(m_shader, 0, "vPosition");
	glBindAttribLocation(m_shader, 1, "vUv");
	glBindAttribLocation(m_shader, 2, "vNormal");
	glLinkProgram(m_shader);
	std::cout << "Finished Step 5" << std::endl;
	// step 6:
	// delete the vs and fs shaders
	glDeleteShader(vs);
	glDeleteShader(fs);
	std::cout << "Finished Step 6" << std::endl;
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

#pragma region //Particle Shader

	const char* ParticleVertexShader =
		"#version 410\n								\
		in vec4 Position;\n								\
		in vec4 inColor;\n								\
														\
		out vec4 Color;\n								\
														\
		uniform mat4 ProjectionView;\n					\
														\
		void main()\n										\
		{\n												\
			Color = inColor;\n							\
			gl_Position = ProjectionView * Position;\n		\
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
														\
														\
														\
														\
														\
			FragColor = Color;\n						\
		};\n											";

	GLuint PaVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(PaVertexShader, 1, &ParticleVertexShader, 0);
	glCompileShader(PaVertexShader);

	GLuint PaFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(PaFragmentShader, 1, &ParticleFragmentShader, 0);

	m_ParticleShader = glCreateProgram();

	glAttachShader(m_ParticleShader, PaVertexShader);
	glAttachShader(m_ParticleShader, PaFragmentShader);

	glBindAttribLocation(m_ParticleShader, 0, "Position");
	glBindAttribLocation(m_ParticleShader, 1, "inColor");

	glLinkProgram(m_ParticleShader);

	glDeleteShader(PaVertexShader);
	glDeleteShader(PaFragmentShader);

#pragma endregion

}

void PROJECTApp::UnloadShader()
{
	glDeleteProgram(m_shader);
	glDeleteProgram(m_ModelShader);
	glDeleteProgram(m_ParticleShader);
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

	std::cout << verts.size() << std::endl;

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
	CreateFBXOpenGLBuffers(m_FBXList.at(m_FBXList.size()));
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
	m_FBXList.clear();
}
