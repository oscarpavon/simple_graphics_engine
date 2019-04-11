#include "ThirdPerson.hpp"
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> 

void ThirdPerson::move_forward(){
	//this->engine->main_camera.MoveForward();
	glm::mat4 mat = glm::translate(glm::mat4(1.0),glm::vec3(0,0,0));
	
	glm::mat4 rotated = glm::rotate(mat,glm::radians(90.f),glm::vec3(0,1,0));
	glm::mat4 rotated2 = glm::rotate(rotated,glm::radians(90.f),glm::vec3(0,1,0));
	glm::mat4 rotated3 = glm::rotate(rotated2,glm::radians(90.f),glm::vec3(-1,0,0));

	glm::mat4 translated =glm::translate(glm::mat4(1.0),glm::vec3(0,-3,-10));



	this->engine->main_camera.View = translated * rotated3 * glm::inverse(this->mesh->model_matrix);
	
}
void ThirdPerson::update(){
	move_forward();
	if(engine->input.W.bIsPressed){
		
		this->mesh->model_matrix = glm::translate(mesh->model_matrix,glm::vec3(0,-0.001,0));
		engine->print_debug("moving around",10,0);
	}
}