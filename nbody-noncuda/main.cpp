#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"

#define window_width 800
#define window_height 800

static float G = 1200; //change to calibrated value
static float timestep = 1.0f;

const float velMin = 1.0f;
const float velMax = 1000000.0f;
const float sizeMin = 1.0f;		//not in any scale - just used for display
const float sizeMax = 10.0f;
const float massMin = 1.0f;		//in terms of 10^24
const float massMax = 100000.0f;
const float posMin = 0;
const float posMax = 10000000000000.0f; //meters

#define num_particles 100
#define pi 3.14159265

typedef struct vector2
{
	float x, y;

	vector2(float _x, float _y){ x = _x; y = _y; }
	vector2(){ x = 0.0f; y = 0.0f; }
	void operator+=(const vector2 &other){
		x += other.x;
		y += other.y;
	}
	void operator-=(const vector2 &other){
		x -= other.x;
		y -= other.y;
	}
	void operator*=(const vector2 &other){
		x *= other.x;
		y *= other.y;
	}
	void operator*=(const float &var){
		x *= var;
		y *= var;
	}
	void operator/=(const float &var){
		x /= var;
		y /= var;
	}
	void operator/=(const vector2 &other){
		x /= other.x;
		y /= other.y;
	}
	vector2 operator+(const vector2 &other){
		vector2 copy = *this;
		copy += other;
		return copy;
	}
	vector2 operator*(const vector2 &other){
		vector2 copy = *this;
		copy *= other;
		return copy;
	}
	vector2 operator-(const vector2 &other){
		vector2 copy = *this;
		copy -= other;
		return copy;
	}
	vector2 operator*(const float &var){
		vector2 copy = *this;
		copy *= var;
		return copy;
	}
	vector2 operator/(const float &var){
		vector2 copy = *this;
		copy /= var;
		return copy;
	}

	float vlength(){
		return sqrtf(x*x + y*y);
	}
}vector2;

float randFloat(float maxVal){
	return ((float) rand() / (float) RAND_MAX) * maxVal;
}
float randFloat(float minVal, float maxVal){
	return (((float) rand() / (float) RAND_MAX) * (maxVal-minVal) + minVal);
}

typedef struct particle{
	vector2 pos, vel, acc;
	float diameter, mass;

	particle(){
		pos = vector2(randFloat(posMin,posMax),randFloat(posMin,posMax)); 
		vel = vector2(randFloat(-1 * velMax, velMax), randFloat(-1 * velMax, velMax));
		acc = vector2();
		diameter = randFloat(sizeMin, sizeMax);
		mass = randFloat(massMin, massMax);
	}

	void applyForce(vector2 force){
		acc += force / mass;
	}

	//calculates and applies the interaction between particles
	void interaction(particle *other){
		float dist = (pos - other->pos).vlength();
		vector2 force = (pos - other->pos)*(G * powf(10,19) * (mass * other->mass) / (dist * dist * dist));
		acc += force / mass;
		other->acc += force / other->mass;
	}

	//update the particle's position/velocity
	void update(){
		pos += vel*(timestep) +(acc) * 0.5f *(timestep*timestep);
		vel += acc*(timestep);
	}
	vector2* pointList(){
		vector2 * list = (vector2*) malloc(360 * sizeof(vector2));
		for (int i = 0; i < 360; i++){
			float heading = i * 180.0f / pi;
			list[i] = vector2(cosf(heading)*diameter / 2.0f, sinf(heading)*diameter / 2.0f);
		}
	}
}particle;

particle *particles;

void initParticles(){
	for (int i = 0; i < num_particles; i++){
		particles[i] = particle();
	}
}

void applyGravity(){
	for (int i = 0; i < num_particles; i++){
		for (int j = i + 1; j < num_particles; j++){
			particles[i].interaction(&particles[j]);
		}
	}
	for (int i = 0; i < num_particles; i++){
		particles[i].update();
	}
}

static void display(void)
{
	applyGravity();
	//printf("called\n");

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//View matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();   
	gluLookAt(5.75, 5.75, 10.0, 5.75, 5.75, 0.0, 0.0, 1.0, 0.0);

	/*glRotatef(90.0, 1.0, 0.0, 0.0);
	*/

	glEnable(GL_POINT_SPRITE);


	for (int i = 0; i < num_particles; i++){
		glPointSize((particles[i].mass/massMax)*10.0f);
		float velc = particles[i].vel.vlength();
		velc = (velc / 100.0f) * 255.0f;
		glColor3f(velc>255.0f ? 255.0f : velc, 0.0f, 255.0f - (velc>255.0f ? 255.0f : velc));
		glBegin(GL_POINTS);
		glVertex3f((particles[i].pos.x / posMax)*12.0f, (particles[i].pos.y / posMax)*12.0f, 0.0f);
		glEnd();
		//printf("%f, %f, %f ~ %f, %f, %f ~ %f\n\n", (particles[i].pos.x / posMax)*12.0f, (particles[i].pos.y / posMax)*12.0f, 0.0f, particles[i].vel.x, particles[i].vel.y, 0.0f, particles[i].diameter);

	}

	glDisable(GL_POINT_SPRITE);
	/*
	glEnable(GL_POINT_SPRITE);
	glPointSize(6.0f);
	glBegin(GL_POINTS);

	glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();   // lower left vertex

	glPointSize(3.0f);
	glBegin(GL_POINTS);
	glVertex3f(5.5f, -5.5f, 0.0f);    // lower right vertex
	glVertex3f(5.5f, 5.5f, 0.0f);    // upper vertex


	glVertex3f(-5.5f, -5.5f, 0.0f);    // lower right vertex
	glVertex3f(-5.5f, 5.5f, 0.0f);    // upper vertex
	glEnd();
	glDisable(GL_POINT_SPRITE);
	*/
	glutSwapBuffers();
	glutPostRedisplay();

}

void initGL(int argc, char **argv)
{
	glutInit(&argc, argv);

	//Setup window
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(window_width, window_height);

	glutCreateWindow("Million particles");

	//Register callbacks
	glutDisplayFunc(display);
	//glutKeyboardFunc(keyboard);
	//glutMouseFunc(mouse);
	//glutMotionFunc(motion);

	//GLEW initialization
	glewInit();
	if (!glewIsSupported("GL_VERSION_4_0 ")) {
		fprintf(stderr, "ERROR: Support for necessary OpenGL extensions missing.");
		fflush(stderr);

		exit(0);
	}

	//Clear
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glDisable(GL_DEPTH_TEST);

	//Viewport
	glViewport(0, 0, window_width, window_height);

	//Projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat) window_width / (GLfloat) window_height, 0.1, 10.0);
}

int main(int argc, char **argv)
{

	initGL(argc, argv);
	particles = (particle*) malloc(num_particles*sizeof(particle));
	initParticles();

	glutMainLoop();

	free(particles);
	return 0;
}
