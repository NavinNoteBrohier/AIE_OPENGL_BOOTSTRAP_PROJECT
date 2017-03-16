#include "FBXUtility.h"
#include <Gizmos.h>



using aie::Gizmos;


FBXFILES::FBXFILES()
{
}

FBXFILES::~FBXFILES()
{
}

void FBXFILES::CreateFBXOpenGLBuffers(FBXFile * file)
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

void FBXFILES::CreateFBXOpenGLBuffers(FBXFile * file, bool additionalAtribs)
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

void FBXFILES::CleanupFBXOpenGLBuffers(FBXFile * file)
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

void FBXFILES::LoadFBX(char * Location)
{
	FBXFile* TempFBX;
	TempFBX = new FBXFile();
	TempFBX->load(Location, FBXFile::UNITS_CENTIMETER);
	m_FBXList.push_back(TempFBX);
	CreateFBXOpenGLBuffers(m_FBXList.at(m_FBXList.size() - 1));
}

void FBXFILES::LoadFBX(char * Location, bool anim)
{
	FBXFile* TempFBX;
	TempFBX = new FBXFile();
	TempFBX->load(Location, FBXFile::UNITS_CENTIMETER);
	m_FBXList.push_back(TempFBX);
	CreateFBXOpenGLBuffers(m_FBXList.at(m_FBXList.size() - 1), true);
}

void FBXFILES::UnloadFBX()
{
	for (unsigned int i = 0; i < m_FBXList.size(); i++)
	{
		CleanupFBXOpenGLBuffers(m_FBXList.at(i));
		m_FBXList.at(i)->unload();
		delete m_FBXList.at(i);
	}
	m_FBXList.clear();
}

void FBXFILES::FBXLoop(unsigned int a_Shader, FBXFile & a_Model, float a_scale, glm::mat4 m_projectionMat, glm::mat4 Cameraview)
{
	glm::mat4 model = glm::mat4
	(
		a_scale, 0, 0, 0,
		0, a_scale, 0, 0,
		0, 0, a_scale, 0,
		0, 0, 0, 1
	);

	glm::mat4 modelViewProjection = m_projectionMat * Cameraview * model;

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

void FBXFILES::FBXLoop(unsigned int a_Shader, FBXFile & a_Model, float a_scale, bool a_Skeleton, glm::mat4 m_projectionMat, glm::mat4 Cameraview)
{
	glm::mat4 model = glm::mat4
	(
		a_scale, 0, 0, 0,
		0, a_scale, 0, 0,
		0, 0, a_scale, 0,
		0, 0, 0, 1
	);

	glm::mat4 modelViewProjection = m_projectionMat * Cameraview;

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
	Gizmos::draw(m_projectionMat * Cameraview);
}

void FBXFILES::LoadFBXAnimations(std::string a_String[])
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

void FBXFILES::UpdateFBXAnimation(FBXFile * a_model, FBXFile * a_anims)
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
