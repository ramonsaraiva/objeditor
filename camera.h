#include <math.h>
#include <GL/gl.h>
#include <GL/glut.h>

#define PI 3.141592653589

#define SPEED 1.05

class Camera
{
	private:
		float angle;
		void refreshDirection(void);

	public:
		float* i;
		float* d;

		Camera();
		Camera(float init_angle);

		float getSin(void);
		float getCos(void);

		void changeAngle(float angle2);
		void setDirectionY(float y);
		void setEye(int x, int y, int z);
		void move(int direction);
		void moveSide(int direction);

		void refreshLookAt();
		void resetView(int width, int height);
};
