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

        void set_translate(float* trans);
        void set_rotate(float* rot);
        void set_scale(float* sca);
        void set_angle_inc(float ai);

        float* get_translate();
        float* get_rotate();
        float* get_scale();
        float get_rotation_angle();

        void toggle_rotation();
        bool is_rotating();
        void inc_rotation_angle();
};
