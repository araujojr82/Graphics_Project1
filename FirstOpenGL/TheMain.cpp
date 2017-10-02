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

// Remember to #include <vector>...
std::vector< cGameObject* > g_vecGameObjects;


glm::vec3 g_cameraXYZ = glm::vec3( 0.0f, 0.0f, 5.0f );	// 5 units "down" z
// TODO include camera new code

cVAOMeshManager* g_pVAOManager = 0;		// or NULL or nullptr

cShaderManager*		g_pShaderManager;		// Heap, new (and delete)


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

	const float CAMERASPEED = 0.1f;
	switch( key )
	{
	case GLFW_KEY_A:		// Left
		g_cameraXYZ.x -= CAMERASPEED;
		break;
	case GLFW_KEY_D:		// Right
		g_cameraXYZ.x += CAMERASPEED;
		break;
	case GLFW_KEY_W:		// Forward (along z)
		g_cameraXYZ.z += CAMERASPEED;
		break;
	case GLFW_KEY_S:		// Backwards (along z)
		g_cameraXYZ.z -= CAMERASPEED;
		break;
	case GLFW_KEY_Q:		// "Down" (along y axis)
		g_cameraXYZ.y -= CAMERASPEED;
		break;
	case GLFW_KEY_E:		// "Up" (along y axis)
		g_cameraXYZ.y += CAMERASPEED;
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


	int height = 480;	/* default */
	int width = 640;	// default
	std::string title = "OpenGL Rocks";

	std::ifstream infoFile( "config.txt" );
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

		infoFile >> width;	// 1080

		infoFile >> a;	// "height"

		infoFile >> height;	// 768

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
				title = ssTitle.str();
			}
		} while( bKeepReading );


	}//if ( ! infoFile.is_open() )

	{
		cGameObject* pTempGO = new cGameObject();
		pTempGO->position.x = 0.5f;
		pTempGO->orientation.z = glm::degrees( 0.0f );	// Degrees
		pTempGO->orientation2.x = glm::degrees( 45.0f );	// Degrees
		pTempGO->orientation2.z = glm::degrees( 0.0f );	// Degrees
		pTempGO->scale = 1.0f;
		pTempGO->diffuseColour = glm::vec4( 0.5f, 0.5f, 0.5f, 1.0f );
		pTempGO->meshName = "spacefighter";
		::g_vecGameObjects.push_back( pTempGO );		// Fastest way to add
	}

	{
		cGameObject* pTempGO = new cGameObject();
		pTempGO->position.x = -0.5f;
		pTempGO->orientation.z = glm::degrees( 0.0f );	// Degrees
		pTempGO->orientation2.z = glm::degrees( 0.0f );	// Degrees
		pTempGO->scale = 1.0f;
		pTempGO->diffuseColour = glm::vec4( 0.0f, 1.0f, 0.0f, 1.0f );
		pTempGO->meshName = "virus";
		::g_vecGameObjects.push_back( pTempGO );		// Fastest way to add
	}

	{
		cGameObject* pTempGO = new cGameObject();
		pTempGO->position.x = -0.5f;
		pTempGO->orientation.z = glm::degrees( 0.0f );	// Degrees
		pTempGO->orientation2.x = glm::degrees( 90.0f );	// Degrees
		pTempGO->orientation2.z = glm::degrees( 0.0f );	// Degrees
		pTempGO->scale = 1.0f;
		pTempGO->diffuseColour = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );
		pTempGO->meshName = "cell";
		::g_vecGameObjects.push_back( pTempGO );		// Fastest way to add
	}

	// Add a bunch more rabbits
	const float SIZEOFWORLD = 4.0f;
	//for ( int index = 2; index < MAXNUMBEROFGAMEOBJECTS; index++ )
	for( int index = 3; index < 20; index++ )
	{
		cGameObject* pTempGO = new cGameObject();
		pTempGO->position.x = getRandInRange<float>( -SIZEOFWORLD, SIZEOFWORLD );
		pTempGO->position.y = getRandInRange<float>( -SIZEOFWORLD, SIZEOFWORLD );
		pTempGO->position.z = 0.0f;
		pTempGO->diffuseColour.r = getRandInRange<float>( 0.0f, 1.0f );
		pTempGO->diffuseColour.g = 1.0f;
		pTempGO->diffuseColour.b = getRandInRange<float>( 0.0f, 1.0f );
		pTempGO->meshName = "virus";
		::g_vecGameObjects.push_back( pTempGO );		// Fastest way to add
	}

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );

	// C++ string
	// C no strings. Sorry. char    char name[7] = "Michael\0";
	window = glfwCreateWindow( width, height,
		title.c_str(),
		NULL, NULL );
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
		//if (!LoadPlyFileIntoMesh("bun_zipper_res2_xyz.ply", testMesh))
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
		testMesh.name = "spacefighter";
		//if (!LoadPlyFileIntoMesh("Utah_Teapot_1Unit_xyz.ply", testMesh))
		if( !LoadPlyFileIntoMesh( "SmallSpaceFighter.ply", testMesh ) )
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
		testMesh.name = "cell";
		if( !LoadPlyFileIntoMesh( "blood_cell.ply", testMesh ) )
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

			// Do not rotate Fighter (IF)
			if( index != 0 )
			{
				::g_vecGameObjects[index]->orientation2.y += 0.01f;
			}

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

			// View or "camera" matrix
			glm::mat4 v = glm::mat4( 1.0f );	// identity

			v = glm::lookAt( g_cameraXYZ,						// "eye" or "camera" position
				glm::vec3( 0.0f, 0.0f, 0.0f ),		// "At" or "target" 
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