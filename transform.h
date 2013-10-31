class Transform
{
    private:
        float* translate;
        float* rotate;
        float* scale;

    public:
        Transform(float* trans, float* rot, float* sca);

        float* get_translate();
        float* get_rotate();
        float* get_scale();
};
