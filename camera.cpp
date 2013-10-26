#include "camera.h"

Camera::Camera(float init_angle){
	i = new float[3];
	i[0] = 0;
	i[1] = 0;
	i[2] = 0;
	d = new float[3];
	d[1] = 0;
	angle = init_angle;
	refreshDirection();
	resetView(800, 600);
}

void Camera::refreshLookAt()
{
	// já deixa preparado para o modo transformaçòes e renderização de objetos
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(i[0], i[1], i[2],   d[0], i[1] + d[1], d[2],   0.0, 1.0, 0.0); 
}

void Camera::resetView(int width, int height){
	// define tamanho da viewport (porção visível da tela)
	glViewport(0, 0, width, height);

	// modo definição de matriz de visualziação
	glMatrixMode(GL_PROJECTION);
	// limpa matrizes
	glLoadIdentity();

	// define câmera com projeção perspectiva
	gluPerspective(45.0, width / (double)height, 0.2, 1000.0);

	refreshLookAt();
}

void Camera::changeAngle(float angle2){
	angle += angle2;
	if(angle > 360)
		angle -= 360;
	else if(angle < 0)
		angle += 360;
	refreshDirection();
	refreshLookAt();
}

void Camera::refreshDirection(){
	d[0] =  getCos() + i[0];
	d[2] =  getSin() + i[2];
}

void Camera::setDirectionY(float y){
	d[1] += y;
	refreshLookAt();
}

void Camera::move(int direction){// direction = 1 to front, direction = -1 to backwards
	i[0] += SPEED * direction * getCos();
	i[1] += SPEED * direction * d[1];
	i[2] += SPEED * direction * getSin();
	refreshDirection();
	refreshLookAt();
}

void Camera::moveSide(int direction){// direction = -1 to right, direction = 1 to left
	float auxAngle = angle / 180 * PI - (PI / 2);
	i[0] += SPEED * cos(auxAngle) * direction;
	i[2] += SPEED * sin(auxAngle) * direction;
	refreshDirection();
	refreshLookAt();
}

float Camera::getSin(){
	return sin(angle / 180 * PI);
}

float Camera::getCos(){
	return cos(angle / 180 * PI);
}
