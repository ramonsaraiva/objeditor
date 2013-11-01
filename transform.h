class Transform
{
    private:
        float* translate;
        float* rotate;
        float* scale;
        float angle_inc;

        bool rotation = false;
        float rotation_angle = 0.0;

    public:
        Transform(float* trans, float* rot, float* sca, float ai);

        float* get_translate();
        float* get_rotate();
        float* get_scale();

        void toggle_rotation();
        bool is_rotating();
        float get_rotation_angle();
        void inc_rotation_angle();
};
