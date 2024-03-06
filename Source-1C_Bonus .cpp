//********************************
//Αυτό το αρχείο θα το χρησιμοποιήσετε
// για να υλοποιήσετε την άσκηση 1Β της OpenGL
//
//ΑΜ:                         Όνομα:
//ΑΜ:                         Όνομα:

//*********************************
#define _CRT_SECURE_NO_WARNINGS

#define STB_IMAGE_IMPLEMENTATION

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include "stb_image.h"

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;





//************************
// Βοηθητικές συναρτήσεις
glm::mat4 View;
glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix() {
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}





//*******************************************************************************
// Η παρακάτω συνάρτηση είναι από http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
// H συνάρτηση loadOBJ φορτώνει ένα αντικείμενο από το obj αρχείο του και φορτώνει και normals kai uv συντεταγμένες
// Την χρησιμοποιείτε όπως το παράδειγμα που έχω στην main
// Very, VERY simple OBJ loader.
// 

bool loadOBJ(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec2>& out_uvs,
	std::vector<glm::vec3>& out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				fclose(file);
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}
	fclose(file);
	return true;
}



//************************************
// Η LoadShaders είναι black box για σας
//************************************
GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}


///****************************************************************
//  Εδω θα υλοποιήσετε την συνάρτηση της κάμερας
//****************************************************************

// Initial Field of View
float initialFoV = 45.0f;


// For Camera Movement 
// Initialize camera's Coordinates
float cctvX = 40.0f;
float cctvY = 40.0f;
float cctvZ = 40.0f;


// Initial camera's Position (10, 50, 0)
glm::vec3 position = glm::vec3(cctvX, cctvY, cctvZ);

// Initial camera's Target : P(0, 0, 0)
glm::vec3 target = glm::vec3(10.0f, 10.0f, 0.0f);

// Up Vector
glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);


//	For Rotation

float angleZ = 0.0f;
float angleX = 0.0f;
float angleY = 0.0f;



void camera_function() {


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// For Zoom Factor  (Multipied with camera X, Y, Z coordinates)

	float zoomFactor = 1.0f;




	// X Rotation : when pressing W button
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {

		angleX = angleX + 0.5f;
	}

	// X' Rotation : when pressing X button
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {

		angleX = angleX - 0.5f;
	}




	// Z Rotation : when pressing Q button
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {

		angleZ = angleZ + 0.5f;
	}

	// Z' Roation : when pressing Z button
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {

		angleZ = angleZ - 0.5f;
	}



	// Extra code for rotation around Y axis, You can use Rotation around Y axis with Up Row and Down Row

	// Y Rotation : when pressing Up row button
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {

		angleY = angleY + 0.5f;
	}

	// Y' Rotation : when pressing Down Row button
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {

		angleY = angleY - 0.5f;
	}




	// You can change initialFov in order to see better the inside of our object while Zooming In


	// Zoom In
	if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {


		// Slowly decrease the zoom factor and multiply it with our current X, Y, Z coordinates in order to move our camera closer to the object.
		// The distance between our camera and the object becomes smaller.

		zoomFactor = zoomFactor - 0.01f;

		cctvX = cctvX * zoomFactor;
		cctvY = cctvY * zoomFactor;
		cctvZ = cctvZ * zoomFactor;

		position = glm::vec3(cctvX, cctvY, cctvZ);
	}

	// Zoom Out
	if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {

		// Slowly increase the zoom factor and multiply it with our current X, Y, Z coordinates in order to move our camera closer to the object. 
		// The distance between our camera and the object becomes larger

		zoomFactor = zoomFactor + 0.01f;

		cctvX = cctvX * zoomFactor;
		cctvY = cctvY * zoomFactor;
		cctvZ = cctvZ * zoomFactor;

		position = glm::vec3(cctvX, cctvY, cctvZ);
	}


	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.



	glm::mat4 RotationMatrixX = glm::rotate(mat4(1.0f), glm::radians(angleX), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 RotationMatrixY = glm::rotate(mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 RotationMatrixZ = glm::rotate(mat4(1.0f), glm::radians(angleZ), glm::vec3(0.0f, 0.0f, 1.0f));
	//glm::mat4 zoomMatrixCamera = glm::scale(mat4(1.0f), glm::vec3(zoomFactor, zoomFactor, zoomFactor));





		// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 4.0f, 0.1f, 100.0f);



	// Camera matrix
	View = glm::lookAt(
		position,
		target,
		up
	);



	//ViewMatrix = View * RotationMatrixZ * RotationMatrixY * RotationMatrixX * zoomMatrixCamera ;		// We had an Error Scaling our Camera based on Depth Test... 
																										// One side was not trasparent(check README)
	ViewMatrix = View * RotationMatrixZ * RotationMatrixY * RotationMatrixX;

}



int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1000, 1000, u8"Εργασία 1Γ – Καταστροφή", NULL, NULL);


	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark green background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders

	GLuint programID = LoadShaders("ProjCVertexShader.vertexshader", "ProjCFragmentShader.fragmentshader");

	GLuint MatrixID = glGetUniformLocation(programID, "MVP");




	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// Load the texture: Grid

	int width_grid, height_grid, nrChannels_grid;
	unsigned char* data_grid = stbi_load("ground2.jpg", &width_grid, &height_grid, &nrChannels_grid, 0);

	if (data_grid)
	{

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}



	GLuint textureGrid;
	glGenTextures(1, &textureGrid);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureGrid);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_grid, height_grid, 0, GL_RGB, GL_UNSIGNED_BYTE, data_grid);

	// When MAGnifying the image (no bigger mipmap available), use LINEAR filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureGrid = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> vertices_grid;
	std::vector<glm::vec3> normals_grid;
	std::vector<glm::vec2> uvs_grid;
	bool res_grid = loadOBJ("gridOG.obj", vertices_grid, uvs_grid, normals_grid);


	// Load it into a VBO

	GLuint vertexbuffer_grid;
	glGenBuffers(1, &vertexbuffer_grid);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_grid);
	glBufferData(GL_ARRAY_BUFFER, vertices_grid.size() * sizeof(glm::vec3), &vertices_grid[0], GL_STATIC_DRAW);

	GLuint uvbuffer_grid;
	glGenBuffers(1, &uvbuffer_grid);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_grid);
	glBufferData(GL_ARRAY_BUFFER, uvs_grid.size() * sizeof(glm::vec2), &uvs_grid[0], GL_STATIC_DRAW);





	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





	// Load the texture: Ball

	int width_ball, height_ball, nrChannels_ball;
	unsigned char* data_ball = stbi_load("fire.jpg", &width_ball, &height_ball, &nrChannels_ball, 0);

	if (data_ball)
	{

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}



	GLuint textureBall;
	glGenTextures(1, &textureBall);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureBall);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_ball, height_ball, 0, GL_RGB, GL_UNSIGNED_BYTE, data_ball);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureBall = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> vertices_ball;
	std::vector<glm::vec3> normals_ball;
	std::vector<glm::vec2> uvs_ball;
	bool res_ball = loadOBJ("ball.obj", vertices_ball, uvs_ball, normals_ball);


	// Load it into a VBO

	GLuint vertexbuffer_ball;
	glGenBuffers(1, &vertexbuffer_ball);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_ball);
	glBufferData(GL_ARRAY_BUFFER, vertices_ball.size() * sizeof(glm::vec3), &vertices_ball[0], GL_STATIC_DRAW);

	GLuint uvbuffer_ball;
	glGenBuffers(1, &uvbuffer_ball);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_ball);
	glBufferData(GL_ARRAY_BUFFER, uvs_ball.size() * sizeof(glm::vec2), &uvs_ball[0], GL_STATIC_DRAW);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////







	// For Translate Factor : Creates the movement of the sphere //
	float tfx = 0.0f;
	float tfy = 0.0f;
	float tfz = 0.0f;

	/*if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
		//float tf;
		srand(time(NULL));										// srand(time(NULL)) to initialize our random number generator each time the program is executed for a different value)
		tf = (rand() % 2 + 0.5);									// rand() % Max + Min, Max = 10, Min = 2, in the range 2 to 10
		//srand(1);												// srand(0) in order to not save the initial value of h due to rand() generator
	}*/
	/*// For Translate Factor : Creates the movement of the sphere
	float tf;
	tf = 0.0f;*/

	//************************************************
	// **Προσθέστε κώδικα για την κάμερα
	//*************************************************
	int i = 0.0f;




	do {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);



		//		Model Matrices For Each Object		: Grid / Ball		//
		glm::mat4 ModelMatrixGrid = glm::mat4(1.0);
		glm::mat4 ModelMatrixBall = glm::mat4(1.0f);



		//*************************************************
		// Να προστεθεί κώδικας για τον υπολογισμό του νέο MVP

		// Compute the MVP matrix from keyboard input
		camera_function();												// Call the camera function to be able to rotate/zoom with our camera.
		glm::mat4 ProjectionMatrix = getProjectionMatrix();				// Get the Projection Matrix 
		glm::mat4 ViewMatrix = getViewMatrix();							// Get the ViewMatrix




		// Create our MVP matrix 
		//glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrixGrid2; // Remember, matrix multiplication is the other way around		

		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrixGrid; // Remember, matrix multiplication is the other way around		


		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);



		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureGrid);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureGrid, 0);

		//
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_grid);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_grid);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		//MVP = ProjectionMatrix * ViewMatrix * ModelMatrixGrid2; // Remember, matrix multiplication is the other way around	
		//MVP = ProjectionMatrix * ViewMatrix * ModelMatrixGrid; // Remember, matrix multiplication is the other way around	
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices_grid.size());

		//
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





		if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {

			// So as the sphere can't go to the negeative ( pass through grid )
			if (tfz >= -0.01f) {

				tfz = tfz + 0.01f;
			}
		}



		if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {

			srand(time(NULL));										// srand(time(NULL)) to initialize our random number generator each time the program is executed for a different value)
			tfx = (rand() % 20);									// rand() % Max + Min, Max = 10, Min = 2, in the range 2 to 10
			//srand(0);
			tfy = (rand() % 20);											// rand() % Max + Min, Max = 10, Min = 2, in the range 2 to 10
			//srand(0);												// srand(0) in order to not save the initial value of h due to rand() generator
			tfz = 20.0f;
		}


		if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {

			
			while (tfz >= 0.0f) {



				//*************************************************
				// Να προστεθεί κώδικας για τον υπολογισμό του νέο MVP

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


				// Compute the MVP matrix from keyboard input
				camera_function();												// Call the camera function to be able to rotate/zoom with our camera.
				glm::mat4 ProjectionMatrix = getProjectionMatrix();				// Get the Projection Matrix 
				glm::mat4 ViewMatrix = getViewMatrix();							// Get the ViewMatrix




				tfz = tfz - 0.05f;


				// Speed Up
				if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
				
					tfz = tfz - 0.03;
				}


				// Slow Down
				else if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {

					tfz = tfz + 0.03;
				}


				//		MATRICES FOR : BALL			//
				//Create Translation Matrix
				glm::mat4 TranslationMatrixBall = glm::translate(mat4(1.0f), vec3(tfx, tfy, tfz));

				// NEW MODEL MATRIX FOR : BALL		//
				glm::mat4 ModelMatrixBall2 = TranslationMatrixBall * ModelMatrixBall;


				//tfz = tfz - 0.01f;

				//glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrixGrid2; // Remember, matrix multiplication is the other way around
				glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrixGrid; // Remember, matrix multiplication is the other way around	

				// Bind our texture in Texture Unit 0
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textureGrid);
				// Set our "myTextureSampler" sampler to use Texture Unit 0
				glUniform1i(TextureGrid, 0);

				//
				// 1rst attribute buffer : vertices
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_grid);
				glVertexAttribPointer(
					0,                  // attribute
					3,                  // size
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0            // array buffer offset
				);

				// 2nd attribute buffer : UVs
				glEnableVertexAttribArray(1);
				glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_grid);
				glVertexAttribPointer(
					1,                                // attribute
					2,                                // size
					GL_FLOAT,                         // type
					GL_FALSE,                         // normalized?
					0,                                // stride
					(void*)0                          // array buffer offset
				);



				//glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrixGrid; // Remember, matrix multiplication is the other way around	
				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

				// Draw the triangle !
				glDrawArrays(GL_TRIANGLES, 0, vertices_grid.size());

				//
				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);




			

				// Bind our texture in Texture Unit 0
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textureBall);
				// Set our "myTextureSampler" sampler to use Texture Unit 0
				glUniform1i(TextureBall, 0);



				//
				// 1rst attribute buffer : vertices
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_ball);
				glVertexAttribPointer(
					0,                  // attribute
					3,                  // size
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0            // array buffer offset
				);

				// 2nd attribute buffer : UVs
				glEnableVertexAttribArray(1);
				glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_ball);
				glVertexAttribPointer(
					1,                                // attribute
					2,                                // size
					GL_FLOAT,                         // type
					GL_FALSE,                         // normalized?
					0,                                // stride
					(void*)0                          // array buffer offset
				);



				// Create our MVP matrix 
				MVP = ProjectionMatrix * ViewMatrix * ModelMatrixBall2; // Remember, matrix multiplication is the other way around	

				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
				// Draw the triangle !
				glDrawArrays(GL_TRIANGLES, 0, vertices_ball.size());




				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);



				if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {

					break;
				}

				// Swap buffers
				glfwSwapBuffers(window);
				glfwPollEvents();


			}
		}




						// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();


	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO

	// Cleanup Ball Buffers
	glDeleteBuffers(1, &vertexbuffer_ball);
	glDeleteBuffers(1, &uvbuffer_ball);

	//Cleanup Grid Buffers
	glDeleteBuffers(1, &vertexbuffer_grid);
	glDeleteBuffers(1, &uvbuffer_grid);


	glDeleteProgram(programID);

	// Cleanup Ball Texture
	glDeleteTextures(1, &textureBall);

	//Cleanup Grid Texture
	glDeleteTextures(1, &textureGrid);

	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;

}

