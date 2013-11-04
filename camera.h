#include <math.h>
#include <GL/glew.h>

#define PI 3.141592653589

class Camera
{
	private:
		float angle;
		void refreshDirection(void);
        float speed = 0.3;

	public:
		float* i;
		float* d;

		Camera();
		Camera(float init_angle);
        
		float getSin(void);
		float getCos(void);
        float* get_xyz();

		void changeAngle(float angle2);
		void setDirectionY(float y);
		void setEye(int x, int y, int z);
		void move(int direction);
		void moveSide(int direction);
        void set_speed(float s);

		void refreshLookAt();
		void resetView(int width, int height);
};
