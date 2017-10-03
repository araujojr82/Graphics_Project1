#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>						// C++ cin, cout, etc.
#include <glm/vec3.hpp>					// glm::vec3
#include <glm/vec4.hpp>					// glm::vec4
#include <glm/mat4x4.hpp>				// glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>			// glm::value_ptr
#include <stdlib.h>
#include <stdio.h>
// Add the file stuff library (file stream>
#include <fstream>
#include <sstream>						// "String stream"
#include <istream>
#include <string>
#include <vector>						// smart array, "array" in most languages

#include "Utilities.h"
#include "ModelUtilities.h"
#include "cMesh.h"
#include "cShaderManager.h" 
#include "cGameObject.h"
#include "cVAOMeshManager.h"

// Euclides: Control selected object for movement
int g_GameObjNumber = 0;				// game object vector position number 
glm::vec3 CAMERASPEED = glm::vec3( 0.0f, 0.0f, 0.0f );

// Remember to #include <vector>...
std::vector< cGameObject* > g_vecGameObjects;


glm::vec3 g_cameraXYZ = glm::vec3( 0.0f, 0.0f, 15.0f );	// 5 units "down" z
glm::vec3 g_cameraTarget_XYZ = glm::vec3( 0.0f, 0.0f, 0.0f );

// TODO include camera new code

cVAOMeshManager* g_pVAOManager = 0;		// or NULL or nullptr

cShaderManager*		g_pShaderManager;		// Heap, new (and delete)

struct windowConfig
{
public:
	int height = 480;
	int width = 640;
	std::string title = "Graphics 101 is Awesome!";
};

struct sGOparameters
{
	std::string meshname;
	int nObjects;
	float x, y, z, scale;
	std::string random;
	float rangeX, rangeY, rangeZ, rangeScale;
};

void loadConfigFile( std::string fileName, windowConfig& wConfig );
sGOparameters parseLine( std::ifstream &source );
void loadObjectsFile( std::string fileName );

static void error_callback( int error, const char* description )
{
	fprintf( stderr, "Error: %s\n", description );
}

static void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
		glfwSetWindowShouldClose( window, GLFW_TRUE );

	// Change object in g_GameObject
	if( key == GLFW_KEY_SPACE && action == GLFW_PRESS )
	{
		//::g_GameObjects[1]->position.y += 0.01f;
		if( g_GameObjNumber < ( ::g_vecGameObjects.size() - 1 ) ) {
			g_GameObjNumber++;
		}
		else
		{
			g_GameObjNumber = 0;
		}
	}

	// Change colour
	if( key == GLFW_KEY_C && action == GLFW_PRESS )
	{
		::g_vecGameObjects[g_GameObjNumber]->diffuseColour.r = getRandInRange<float>( 0.0f, 1.0f );
		::g_vecGameObjects[g_GameObjNumber]->diffuseColour.g = getRandInRange<float>( 0.0f, 1.0f );
		::g_vecGameObjects[g_GameObjNumber]->diffuseColour.b = getRandInRange<float>( 0.0f, 1.0f );
	}

	// Change position
	switch( key )
	{
	case GLFW_KEY_UP:		// Up arrow
		::g_vecGameObjects[g_GameObjNumber]->position.y += 0.10f;
		break;
	case GLFW_KEY_DOWN:		// Down arrow
		::g_vecGameObjects[g_GameObjNumber]->position.y -= 0.10f;
		break;
	case GLFW_KEY_LEFT:		// Left arrow
		::g_vecGameObjects[g_GameObjNumber]->position.x -= 0.10f;
		break;
	case GLFW_KEY_RIGHT:	// Right arrow
		::g_vecGameObjects[g_GameObjNumber]->position.x += 0.10f;
		break;
	case GLFW_KEY_I:		// I key
		::g_vecGameObjects[g_GameObjNumber]->position.z += 0.10f;
		break;
	case GLFW_KEY_O:		// O key
		::g_vecGameObjects[g_GameObjNumber]->position.z -= 0.10f;
		break;
	}

	//const float CAMERASPEED = 0.1f;

	switch( key )
	{
	case GLFW_KEY_A:		// Left
		//g_cameraXYZ.x -= CAMERASPEED;
		CAMERASPEED.x -= 0.00001f;
		break;
	case GLFW_KEY_D:		// Right
		//g_cameraXYZ.x += CAMERASPEED;
		CAMERASPEED.x += 0.00001f;
		break;
	case GLFW_KEY_W:		// Forward (along z)
		//g_cameraXYZ.z += CAMERASPEED;
		CAMERASPEED.z += 0.00001f;
		break;
	case GLFW_KEY_S:		// Backwards (along z)
		//g_cameraXYZ.z -= CAMERASPEED;
		CAMERASPEED.z -= 0.00001f;
		break;
	case GLFW_KEY_Q:		// "Down" (along y axis)
		//g_cameraXYZ.y -= CAMERASPEED;
		CAMERASPEED.y -= 0.00001f;
		break;
	case GLFW_KEY_E:		// "Up" (along y axis)
		//g_cameraXYZ.y += CAMERASPEED;
		CAMERASPEED.y += 0.00001f;
		break;

	}
	return;
}

int main( void )
{
	GLFWwindow* window;
	GLuint vertex_buffer, vertex_shader, fragment_shader, program;
	GLint mvp_location, vpos_location, vcol_location;
	glfwSetErrorCallback( error_callback );
	
	if( !glfwInit() )
		exit( EXIT_FAILURE );

	windowConfig wConfig;
	
	loadConfigFile( "config.txt", wConfig );
	loadObjectsFile( "objects.txt" );

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );

	window = glfwCreateWindow( wConfig.width, wConfig.height,
								wConfig.title.c_str(),
								glfwGetPrimaryMonitor(), //NULL,
								NULL );
	if( !window )
	{
		glfwTerminate();
		exit( EXIT_FAILURE );
	}

	glfwSetKeyCallback( window, key_callback );
	glfwMakeContextCurrent( window );
	gladLoadGLLoader( ( GLADloadproc )glfwGetProcAddress );
	glfwSwapInterval( 1 );

	std::cout << glGetString( GL_VENDOR ) << " "
		<< glGetString( GL_RENDERER ) << ", "
		<< glGetString( GL_VERSION ) << std::endl;
	std::cout << "Shader language version: " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << std::endl;

	::g_pShaderManager = new cShaderManager();

	cShaderManager::cShader vertShader;
	cShaderManager::cShader fragShader;

	vertShader.fileName = "simpleVert.glsl";
	fragShader.fileName = "simpleFrag.glsl";

	::g_pShaderManager->setBasePath( "assets//shaders//" );

	// Shader objects are passed by reference so that
	//	we can look at the results if we wanted to. 
	if( !::g_pShaderManager->createProgramFromFile(
		"mySexyShader", vertShader, fragShader ) )
	{
		std::cout << "Oh no! All is lost!!! Blame Loki!!!" << std::endl;
		std::cout << ::g_pShaderManager->getLastError() << std::endl;
		// Should we exit?? 
		return -1;
		//		exit(
	}
	std::cout << "The shaders compiled and linked OK" << std::endl;

	//Load models
	::g_pVAOManager = new cVAOMeshManager();

	GLint sexyShaderID = ::g_pShaderManager->getIDFromFriendlyName( "mySexyShader" );

	{
		cMesh testMesh;
		testMesh.name = "virus";
		if( !LoadPlyFileIntoMesh( "virus_super_low_res_XYZ.ply", testMesh ) )
		{
			std::cout << "Didn't load model" << std::endl;
			// do something??
		}
		if( !::g_pVAOManager->loadMeshIntoVAO( testMesh, sexyShaderID ) )
		{
			std::cout << "Could not load mesh into VAO" << std::endl;
		}
	}

	{
		cMesh testMesh;
		testMesh.name = "bacteria1";
		if( !LoadPlyFileIntoMesh( "bacillus_electronscope_xyz.ply", testMesh ) )
		{
			std::cout << "Didn't load model" << std::endl;
			// do something??
		}
		if( !::g_pVAOManager->loadMeshIntoVAO( testMesh, sexyShaderID ) )
		{
			std::cout << "Could not load mesh into VAO" << std::endl;
		}
	}

	{
		cMesh testMesh;
		testMesh.name = "bacteria2";
		if( !LoadPlyFileIntoMesh( "Flagellate_bacteria_xyz.ply", testMesh ) )
		{
			std::cout << "Didn't load model" << std::endl;
			// do something??
		}
		if( !::g_pVAOManager->loadMeshIntoVAO( testMesh, sexyShaderID ) )
		{
			std::cout << "Could not load mesh into VAO" << std::endl;
		}
	}

	{
		cMesh testMesh;
		testMesh.name = "bloodcell";
		if( !LoadPlyFileIntoMesh( "erythro_for_obj_xyz.ply", testMesh ) )
		{
			std::cout << "Didn't load model" << std::endl;
			// do something??
		}
		if( !::g_pVAOManager->loadMeshIntoVAO( testMesh, sexyShaderID ) )
		{
			std::cout << "Could not load mesh into VAO" << std::endl;
		}
	}

	GLint currentProgID = ::g_pShaderManager->getIDFromFriendlyName( "mySexyShader" );

	mvp_location = glGetUniformLocation( currentProgID, "MVP" );		// program, "MVP");

	// Main game or application loop
	while( !glfwWindowShouldClose( window ) )
	{
		float ratio;
		int width, height;
		glm::mat4x4 m, p, mvp;			//  mat4x4 m, p, mvp;

		glfwGetFramebufferSize( window, &width, &height );
		ratio = width / ( float )height;
		glViewport( 0, 0, width, height );

		glClear( GL_COLOR_BUFFER_BIT );

		// "Draw scene" loop
		//for ( int index = 0; index != MAXNUMBEROFGAMEOBJECTS; index++ )

		unsigned int sizeOfVector = ::g_vecGameObjects.size();
		for( int index = 0; index != sizeOfVector; index++ )
		{
			// Is there a game object? 
			if( ::g_vecGameObjects[index] == 0 )
			{	// Nothing to draw
				continue;		// Skip all for loop code and go to next
			}

			// Was near the draw call, but we need the mesh name
			std::string meshToDraw = ::g_vecGameObjects[index]->meshName;		//::g_GameObjects[index]->meshName;

			sVAOInfo VAODrawInfo;
			if( ::g_pVAOManager->lookupVAOFromName( meshToDraw, VAODrawInfo ) == false )
			{	// Didn't find mesh
				continue;
			}

			// There IS something to draw
			m = glm::mat4x4( 1.0f );	//		mat4x4_identity(m);

			::g_vecGameObjects[index]->orientation.z += 0.001f;

			glm::mat4 matRreRotZ = glm::mat4x4( 1.0f );
			matRreRotZ = glm::rotate( matRreRotZ, ::g_vecGameObjects[index]->orientation.z,
				glm::vec3( 0.0f, 0.0f, 1.0f ) );
			m = m * matRreRotZ;

			glm::mat4 trans = glm::mat4x4( 1.0f );
			trans = glm::translate( trans,
				::g_vecGameObjects[index]->position );
			m = m * trans;

			glm::mat4 matPostRotZ = glm::mat4x4( 1.0f );
			matPostRotZ = glm::rotate( matPostRotZ, ::g_vecGameObjects[index]->orientation2.z,
				glm::vec3( 0.0f, 0.0f, 1.0f ) );
			m = m * matPostRotZ;

			//// Do not rotate Fighter (IF)
			//if( index != 0 )
			//{
			//	::g_vecGameObjects[index]->orientation2.y += 0.01f;
			//}
			::g_vecGameObjects[index]->orientation2.y += 0.01f;

			glm::mat4 matPostRotY = glm::mat4x4( 1.0f );
			matPostRotY = glm::rotate( matPostRotY, ::g_vecGameObjects[index]->orientation2.y,
				glm::vec3( 0.0f, 1.0f, 0.0f ) );
			m = m * matPostRotY;


			glm::mat4 matPostRotX = glm::mat4x4( 1.0f );
			matPostRotX = glm::rotate( matPostRotX, ::g_vecGameObjects[index]->orientation2.x,
				glm::vec3( 1.0f, 0.0f, 0.0f ) );
			m = m * matPostRotX;
			// TODO: add the other rotation matrix (i.e. duplicate code above)

			float finalScale = VAODrawInfo.scaleForUnitBBox * ::g_vecGameObjects[index]->scale;

			glm::mat4 matScale = glm::mat4x4( 1.0f );
			matScale = glm::scale( matScale,
				glm::vec3( finalScale,
					finalScale,
					finalScale ) );
			m = m * matScale;

			// Change from an orthographic view to a perspective view
			p = glm::perspective( 0.6f,			// FOV
				ratio,		// Aspect ratio
				0.1f,			// Near (as big as possible)
				1000.0f );	// Far (as small as possible)

			// Set Camera Speed according to the user input
			//g_cameraXYZ.z -= 0.0001f;
			g_cameraXYZ.x += CAMERASPEED.x;
			g_cameraXYZ.z += CAMERASPEED.y;
			g_cameraXYZ.y += CAMERASPEED.z;

			// View or "camera" matrix
			glm::mat4 v = glm::mat4( 1.0f );	// identity

			v = glm::lookAt( g_cameraXYZ,			// "eye" or "camera" position
				g_cameraTarget_XYZ,					// "At" or "target"							 
				glm::vec3( 0.0f, 1.0f, 0.0f ) );	// "up" vector

			//mat4x4_mul(mvp, p, m);
			mvp = p * v * m;			// This way (sort of backwards)

			GLint shaderID = ::g_pShaderManager->getIDFromFriendlyName( "mySexyShader" );
			GLint diffuseColour_loc = glGetUniformLocation( shaderID, "diffuseColour" );

			glUniform4f( diffuseColour_loc,
				::g_vecGameObjects[index]->diffuseColour.r,
				::g_vecGameObjects[index]->diffuseColour.g,
				::g_vecGameObjects[index]->diffuseColour.b,
				::g_vecGameObjects[index]->diffuseColour.a );

			::g_pShaderManager->useShaderProgram( "mySexyShader" );

			glUniformMatrix4fv( mvp_location, 1, GL_FALSE,
				( const GLfloat* )glm::value_ptr( mvp ) );

			//		glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			//		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );	// Default


			glBindVertexArray( VAODrawInfo.VAO_ID );

			glDrawElements( GL_TRIANGLES,
				VAODrawInfo.numberOfIndices,
				GL_UNSIGNED_INT,	// 32 bit int
				0 ); // g_numberOfVertices

			glBindVertexArray( 0 );

		}//for ( int index = 0...


		std::stringstream ssTitle;
		ssTitle << "Camera (xyz): "
			<< g_cameraXYZ.x << ", "
			<< g_cameraXYZ.y << ", "
			<< g_cameraXYZ.z;
		glfwSetWindowTitle( window, ssTitle.str().c_str() );

		glfwSwapBuffers( window );
		glfwPollEvents();

	}// while ( ! glfwWindowShouldClose(window) )


	glfwDestroyWindow( window );
	glfwTerminate();

	// 
	delete ::g_pShaderManager;
	delete ::g_pVAOManager;

	//    exit(EXIT_SUCCESS);
	return 0;
}

//Load Config.txt
void loadConfigFile( std::string fileName, windowConfig& wConfig )
{
	// TODO change this config formating
	std::ifstream infoFile( fileName );
	if( !infoFile.is_open() )
	{	// File didn't open...
		std::cout << "Can't find config file" << std::endl;
		std::cout << "Using defaults" << std::endl;
	}
	else
	{	// File DID open, so read it... 
		std::string a;

		infoFile >> a;	// "Game"	//std::cin >> a;
		infoFile >> a;	// "Config"
		infoFile >> a;	// "width"

		infoFile >> wConfig.width;	// 1080

		infoFile >> a;	// "height"

		infoFile >> wConfig.height;	// 768

		infoFile >> a;		// Title_Start

		std::stringstream ssTitle;		// Inside "sstream"
		bool bKeepReading = true;
		do
		{
			infoFile >> a;		// Title_End??
			if( a != "Title_End" )
			{
				ssTitle << a << " ";
			}
			else
			{	// it IS the end! 
				bKeepReading = false;
				wConfig.title = ssTitle.str();
			}
		} while( bKeepReading );
	}
}


//Load objects.txt
void loadObjectsFile( std::string fileName )
{
	//sGOparameters sGOpar;
	std::vector <sGOparameters> allObjects;

	std::ifstream objectsFile( fileName );
	if( !objectsFile.is_open() )
	{	// File didn't open...
		std::cout << "Can't find config file" << std::endl;
		std::cout << "Using defaults" << std::endl;
	}
	else
	{	// File DID open, so loop through the file and pushback to structure
		while( !objectsFile.eof() && objectsFile.is_open() ) {
			allObjects.push_back( parseLine( objectsFile ) );
		}
		objectsFile.close();  //Closing "costfile.txt"
	}

	for( int index = 0; index != allObjects.size(); index++ )
	{	
		// Check, Number of Objects must be at least 1
		if( allObjects[index].nObjects == 0 ) allObjects[index].nObjects = 1;
		
		// Create the number of gameObjects specified in the file for each line 
		for( int i = 0; i != allObjects[index].nObjects; i++ )
		{ 
			// Create a new GO
			cGameObject* pTempGO = new cGameObject();

			pTempGO->meshName = allObjects[index].meshname; // Set the name of the mesh
			if( allObjects[index].random == "true" ) 
			{   // position and the scale should be random
				pTempGO->position.x = getRandInRange<float>( -allObjects[index].rangeX, allObjects[index].rangeX );
				pTempGO->position.y = getRandInRange<float>( -allObjects[index].rangeY, allObjects[index].rangeY );
				pTempGO->position.z = getRandInRange<float>( -allObjects[index].rangeZ, allObjects[index].rangeZ );
				pTempGO->scale      = getRandInRange<float>( 0.0f, allObjects[index].rangeScale );
			}
			else
			{   // position and scale are fixed
				pTempGO->position.x = allObjects[index].x;
				pTempGO->position.y = allObjects[index].y;
				pTempGO->position.z = allObjects[index].z;
				pTempGO->scale      = allObjects[index].scale;
			}

			// HACK set color for each model
			// TODO add color to the config file
			if( allObjects[index].meshname == "bacteria1" )
			{
				pTempGO->diffuseColour = glm::vec4( 0.8f, 0.8f, 0.2f, 1.0f );
			}
			else if( allObjects[index].meshname == "bacteria2" ) 
			{
				pTempGO->diffuseColour = glm::vec4( 0.8f, 1.0f, 0.2f, 1.0f );
			}
			else if( allObjects[index].meshname == "virus" )
			{
				pTempGO->diffuseColour = glm::vec4( 0.1f, 0.9f, 0.1f, 1.0f );
			}
			else if( allObjects[index].meshname == "bloodcell" ) {
				pTempGO->diffuseColour = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );
			}
			::g_vecGameObjects.push_back( pTempGO );
		}
	}
		
}

// Parse the file line to fit into the structure
sGOparameters parseLine( std::ifstream &source ) {

	sGOparameters sGOpar;

	//Scanning a line from the file
	source >> sGOpar.meshname >> sGOpar.nObjects
		>> sGOpar.x >> sGOpar.y >> sGOpar.z >> sGOpar.scale
		>> sGOpar.random
		>> sGOpar.rangeX >> sGOpar.rangeY >> sGOpar.rangeZ
		>> sGOpar.rangeScale;


	return sGOpar;
}
