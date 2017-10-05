#include "cGameObject.h"

cGameObject::cGameObject()
{
	this->scale = 1.0f;	// (not zero)
	this->position = glm::vec3(0.0f);
	this->orientation = glm::vec3(0.0f);
	this->orientation2 = glm::vec3(0.0f);

	// If you aren't sure what the 4th value should be,
	// make it a 1.0f ("alpha" or transparecy)
	this->diffuseColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	
	this->bIsLight = false;
	this->myLight = NULL;

	return;
}

cGameObject::~cGameObject()
{
	return;
}