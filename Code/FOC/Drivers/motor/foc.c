//foc.c
#include "foc.h"
#include "arm_math.h"
#include "motor_runtime_param.h"
#include <stdbool.h>
#define rad60 deg2rad(60)
#define SQRT3 1.73205080756887729353
#define deg2rad(a) (PI * (a) / 180)
#define rad2deg(a) (180 * (a) / PI)
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

static void svpwm(float phi, float d, float q, float *d_u, float *d_v, float *d_w)
{
    d = min(d, 1);
    d = max(d, -1);
    q = min(q, 1);
    q = max(q, -1);
    const int v[6][3] = {{1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 1, 1}, {0, 0, 1}, {1, 0, 1}};
    const int K_to_sector[] = {4, 6, 5, 5, 3, 1, 2, 2};
    float sin_phi = arm_sin_f32(phi);
    float cos_phi = arm_cos_f32(phi);
    float alpha = 0;
    float beta = 0;
    arm_inv_park_f32(d, q, &alpha, &beta, sin_phi, cos_phi);

    bool A = beta > 0;
    bool B = fabs(beta) > SQRT3 * fabs(alpha);
    bool C = alpha > 0;

    int K = 4 * A + 2 * B + C;
    int sector = K_to_sector[K];

    float t_m = arm_sin_f32(sector * rad60) * alpha - arm_cos_f32(sector * rad60) * beta;
    float t_n = beta * arm_cos_f32(sector * rad60 - rad60) - alpha * arm_sin_f32(sector * rad60 - rad60);
    float t_0 = 1 - t_m - t_n;

    *d_u = t_m * v[sector - 1][0] + t_n * v[sector % 6][0] + t_0 / 2;
    *d_v = t_m * v[sector - 1][1] + t_n * v[sector % 6][1] + t_0 / 2;
    *d_w = t_m * v[sector - 1][2] + t_n * v[sector % 6][2] + t_0 / 2;
}

//foc.c
//......
__attribute__((weak)) void set_pwm_duty(float d_u, float d_v, float d_w)
{
    while (1)
        ;
}
void foc_forward(float d, float q, float rotor_rad)
{
    float d_u = 0;
    float d_v = 0;
    float d_w = 0;
    svpwm(rotor_rad, d, q, &d_u, &d_v, &d_w);
    set_pwm_duty(d_u, d_v, d_w);
}
//......
