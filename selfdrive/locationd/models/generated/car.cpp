#include "car.h"

namespace {
#define DIM 9
#define EDIM 9
#define MEDIM 9
typedef void (*Hfun)(double *, double *, double *);

double mass;

void set_mass(double x){ mass = x;}

double rotational_inertia;

void set_rotational_inertia(double x){ rotational_inertia = x;}

double center_to_front;

void set_center_to_front(double x){ center_to_front = x;}

double center_to_rear;

void set_center_to_rear(double x){ center_to_rear = x;}

double stiffness_front;

void set_stiffness_front(double x){ stiffness_front = x;}

double stiffness_rear;

void set_stiffness_rear(double x){ stiffness_rear = x;}
const static double MAHA_THRESH_25 = 3.8414588206941227;
const static double MAHA_THRESH_24 = 5.991464547107981;
const static double MAHA_THRESH_30 = 3.8414588206941227;
const static double MAHA_THRESH_26 = 3.8414588206941227;
const static double MAHA_THRESH_27 = 3.8414588206941227;
const static double MAHA_THRESH_29 = 3.8414588206941227;
const static double MAHA_THRESH_28 = 3.8414588206941227;
const static double MAHA_THRESH_31 = 3.8414588206941227;

/******************************************************************************
 *                       Code generated with SymPy 1.12                       *
 *                                                                            *
 *              See http://www.sympy.org/ for more information.               *
 *                                                                            *
 *                         This file is part of 'ekf'                         *
 ******************************************************************************/
void err_fun(double *nom_x, double *delta_x, double *out_6724727011957861295) {
   out_6724727011957861295[0] = delta_x[0] + nom_x[0];
   out_6724727011957861295[1] = delta_x[1] + nom_x[1];
   out_6724727011957861295[2] = delta_x[2] + nom_x[2];
   out_6724727011957861295[3] = delta_x[3] + nom_x[3];
   out_6724727011957861295[4] = delta_x[4] + nom_x[4];
   out_6724727011957861295[5] = delta_x[5] + nom_x[5];
   out_6724727011957861295[6] = delta_x[6] + nom_x[6];
   out_6724727011957861295[7] = delta_x[7] + nom_x[7];
   out_6724727011957861295[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_3167941242115716840) {
   out_3167941242115716840[0] = -nom_x[0] + true_x[0];
   out_3167941242115716840[1] = -nom_x[1] + true_x[1];
   out_3167941242115716840[2] = -nom_x[2] + true_x[2];
   out_3167941242115716840[3] = -nom_x[3] + true_x[3];
   out_3167941242115716840[4] = -nom_x[4] + true_x[4];
   out_3167941242115716840[5] = -nom_x[5] + true_x[5];
   out_3167941242115716840[6] = -nom_x[6] + true_x[6];
   out_3167941242115716840[7] = -nom_x[7] + true_x[7];
   out_3167941242115716840[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_5315706494890311132) {
   out_5315706494890311132[0] = 1.0;
   out_5315706494890311132[1] = 0;
   out_5315706494890311132[2] = 0;
   out_5315706494890311132[3] = 0;
   out_5315706494890311132[4] = 0;
   out_5315706494890311132[5] = 0;
   out_5315706494890311132[6] = 0;
   out_5315706494890311132[7] = 0;
   out_5315706494890311132[8] = 0;
   out_5315706494890311132[9] = 0;
   out_5315706494890311132[10] = 1.0;
   out_5315706494890311132[11] = 0;
   out_5315706494890311132[12] = 0;
   out_5315706494890311132[13] = 0;
   out_5315706494890311132[14] = 0;
   out_5315706494890311132[15] = 0;
   out_5315706494890311132[16] = 0;
   out_5315706494890311132[17] = 0;
   out_5315706494890311132[18] = 0;
   out_5315706494890311132[19] = 0;
   out_5315706494890311132[20] = 1.0;
   out_5315706494890311132[21] = 0;
   out_5315706494890311132[22] = 0;
   out_5315706494890311132[23] = 0;
   out_5315706494890311132[24] = 0;
   out_5315706494890311132[25] = 0;
   out_5315706494890311132[26] = 0;
   out_5315706494890311132[27] = 0;
   out_5315706494890311132[28] = 0;
   out_5315706494890311132[29] = 0;
   out_5315706494890311132[30] = 1.0;
   out_5315706494890311132[31] = 0;
   out_5315706494890311132[32] = 0;
   out_5315706494890311132[33] = 0;
   out_5315706494890311132[34] = 0;
   out_5315706494890311132[35] = 0;
   out_5315706494890311132[36] = 0;
   out_5315706494890311132[37] = 0;
   out_5315706494890311132[38] = 0;
   out_5315706494890311132[39] = 0;
   out_5315706494890311132[40] = 1.0;
   out_5315706494890311132[41] = 0;
   out_5315706494890311132[42] = 0;
   out_5315706494890311132[43] = 0;
   out_5315706494890311132[44] = 0;
   out_5315706494890311132[45] = 0;
   out_5315706494890311132[46] = 0;
   out_5315706494890311132[47] = 0;
   out_5315706494890311132[48] = 0;
   out_5315706494890311132[49] = 0;
   out_5315706494890311132[50] = 1.0;
   out_5315706494890311132[51] = 0;
   out_5315706494890311132[52] = 0;
   out_5315706494890311132[53] = 0;
   out_5315706494890311132[54] = 0;
   out_5315706494890311132[55] = 0;
   out_5315706494890311132[56] = 0;
   out_5315706494890311132[57] = 0;
   out_5315706494890311132[58] = 0;
   out_5315706494890311132[59] = 0;
   out_5315706494890311132[60] = 1.0;
   out_5315706494890311132[61] = 0;
   out_5315706494890311132[62] = 0;
   out_5315706494890311132[63] = 0;
   out_5315706494890311132[64] = 0;
   out_5315706494890311132[65] = 0;
   out_5315706494890311132[66] = 0;
   out_5315706494890311132[67] = 0;
   out_5315706494890311132[68] = 0;
   out_5315706494890311132[69] = 0;
   out_5315706494890311132[70] = 1.0;
   out_5315706494890311132[71] = 0;
   out_5315706494890311132[72] = 0;
   out_5315706494890311132[73] = 0;
   out_5315706494890311132[74] = 0;
   out_5315706494890311132[75] = 0;
   out_5315706494890311132[76] = 0;
   out_5315706494890311132[77] = 0;
   out_5315706494890311132[78] = 0;
   out_5315706494890311132[79] = 0;
   out_5315706494890311132[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_8134846415775312522) {
   out_8134846415775312522[0] = state[0];
   out_8134846415775312522[1] = state[1];
   out_8134846415775312522[2] = state[2];
   out_8134846415775312522[3] = state[3];
   out_8134846415775312522[4] = state[4];
   out_8134846415775312522[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8000000000000007*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_8134846415775312522[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_8134846415775312522[7] = state[7];
   out_8134846415775312522[8] = state[8];
}
void F_fun(double *state, double dt, double *out_2041777415663342842) {
   out_2041777415663342842[0] = 1;
   out_2041777415663342842[1] = 0;
   out_2041777415663342842[2] = 0;
   out_2041777415663342842[3] = 0;
   out_2041777415663342842[4] = 0;
   out_2041777415663342842[5] = 0;
   out_2041777415663342842[6] = 0;
   out_2041777415663342842[7] = 0;
   out_2041777415663342842[8] = 0;
   out_2041777415663342842[9] = 0;
   out_2041777415663342842[10] = 1;
   out_2041777415663342842[11] = 0;
   out_2041777415663342842[12] = 0;
   out_2041777415663342842[13] = 0;
   out_2041777415663342842[14] = 0;
   out_2041777415663342842[15] = 0;
   out_2041777415663342842[16] = 0;
   out_2041777415663342842[17] = 0;
   out_2041777415663342842[18] = 0;
   out_2041777415663342842[19] = 0;
   out_2041777415663342842[20] = 1;
   out_2041777415663342842[21] = 0;
   out_2041777415663342842[22] = 0;
   out_2041777415663342842[23] = 0;
   out_2041777415663342842[24] = 0;
   out_2041777415663342842[25] = 0;
   out_2041777415663342842[26] = 0;
   out_2041777415663342842[27] = 0;
   out_2041777415663342842[28] = 0;
   out_2041777415663342842[29] = 0;
   out_2041777415663342842[30] = 1;
   out_2041777415663342842[31] = 0;
   out_2041777415663342842[32] = 0;
   out_2041777415663342842[33] = 0;
   out_2041777415663342842[34] = 0;
   out_2041777415663342842[35] = 0;
   out_2041777415663342842[36] = 0;
   out_2041777415663342842[37] = 0;
   out_2041777415663342842[38] = 0;
   out_2041777415663342842[39] = 0;
   out_2041777415663342842[40] = 1;
   out_2041777415663342842[41] = 0;
   out_2041777415663342842[42] = 0;
   out_2041777415663342842[43] = 0;
   out_2041777415663342842[44] = 0;
   out_2041777415663342842[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_2041777415663342842[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_2041777415663342842[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_2041777415663342842[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_2041777415663342842[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_2041777415663342842[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_2041777415663342842[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_2041777415663342842[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_2041777415663342842[53] = -9.8000000000000007*dt;
   out_2041777415663342842[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_2041777415663342842[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_2041777415663342842[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_2041777415663342842[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_2041777415663342842[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_2041777415663342842[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_2041777415663342842[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_2041777415663342842[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_2041777415663342842[62] = 0;
   out_2041777415663342842[63] = 0;
   out_2041777415663342842[64] = 0;
   out_2041777415663342842[65] = 0;
   out_2041777415663342842[66] = 0;
   out_2041777415663342842[67] = 0;
   out_2041777415663342842[68] = 0;
   out_2041777415663342842[69] = 0;
   out_2041777415663342842[70] = 1;
   out_2041777415663342842[71] = 0;
   out_2041777415663342842[72] = 0;
   out_2041777415663342842[73] = 0;
   out_2041777415663342842[74] = 0;
   out_2041777415663342842[75] = 0;
   out_2041777415663342842[76] = 0;
   out_2041777415663342842[77] = 0;
   out_2041777415663342842[78] = 0;
   out_2041777415663342842[79] = 0;
   out_2041777415663342842[80] = 1;
}
void h_25(double *state, double *unused, double *out_4947360145717879899) {
   out_4947360145717879899[0] = state[6];
}
void H_25(double *state, double *unused, double *out_6495236860240024280) {
   out_6495236860240024280[0] = 0;
   out_6495236860240024280[1] = 0;
   out_6495236860240024280[2] = 0;
   out_6495236860240024280[3] = 0;
   out_6495236860240024280[4] = 0;
   out_6495236860240024280[5] = 0;
   out_6495236860240024280[6] = 1;
   out_6495236860240024280[7] = 0;
   out_6495236860240024280[8] = 0;
}
void h_24(double *state, double *unused, double *out_4183217639544912511) {
   out_4183217639544912511[0] = state[4];
   out_4183217639544912511[1] = state[5];
}
void H_24(double *state, double *unused, double *out_4322587261234524714) {
   out_4322587261234524714[0] = 0;
   out_4322587261234524714[1] = 0;
   out_4322587261234524714[2] = 0;
   out_4322587261234524714[3] = 0;
   out_4322587261234524714[4] = 1;
   out_4322587261234524714[5] = 0;
   out_4322587261234524714[6] = 0;
   out_4322587261234524714[7] = 0;
   out_4322587261234524714[8] = 0;
   out_4322587261234524714[9] = 0;
   out_4322587261234524714[10] = 0;
   out_4322587261234524714[11] = 0;
   out_4322587261234524714[12] = 0;
   out_4322587261234524714[13] = 0;
   out_4322587261234524714[14] = 1;
   out_4322587261234524714[15] = 0;
   out_4322587261234524714[16] = 0;
   out_4322587261234524714[17] = 0;
}
void h_30(double *state, double *unused, double *out_3496728753088996732) {
   out_3496728753088996732[0] = state[4];
}
void H_30(double *state, double *unused, double *out_9013569818747272907) {
   out_9013569818747272907[0] = 0;
   out_9013569818747272907[1] = 0;
   out_9013569818747272907[2] = 0;
   out_9013569818747272907[3] = 0;
   out_9013569818747272907[4] = 1;
   out_9013569818747272907[5] = 0;
   out_9013569818747272907[6] = 0;
   out_9013569818747272907[7] = 0;
   out_9013569818747272907[8] = 0;
}
void h_26(double *state, double *unused, double *out_2146001117991960103) {
   out_2146001117991960103[0] = state[7];
}
void H_26(double *state, double *unused, double *out_2753733541365968056) {
   out_2753733541365968056[0] = 0;
   out_2753733541365968056[1] = 0;
   out_2753733541365968056[2] = 0;
   out_2753733541365968056[3] = 0;
   out_2753733541365968056[4] = 0;
   out_2753733541365968056[5] = 0;
   out_2753733541365968056[6] = 0;
   out_2753733541365968056[7] = 1;
   out_2753733541365968056[8] = 0;
}
void h_27(double *state, double *unused, double *out_9074023012693440184) {
   out_9074023012693440184[0] = state[3];
}
void H_27(double *state, double *unused, double *out_6838806506946847996) {
   out_6838806506946847996[0] = 0;
   out_6838806506946847996[1] = 0;
   out_6838806506946847996[2] = 0;
   out_6838806506946847996[3] = 1;
   out_6838806506946847996[4] = 0;
   out_6838806506946847996[5] = 0;
   out_6838806506946847996[6] = 0;
   out_6838806506946847996[7] = 0;
   out_6838806506946847996[8] = 0;
}
void h_29(double *state, double *unused, double *out_2486195086217145021) {
   out_2486195086217145021[0] = state[1];
}
void H_29(double *state, double *unused, double *out_8922942910647886525) {
   out_8922942910647886525[0] = 0;
   out_8922942910647886525[1] = 1;
   out_8922942910647886525[2] = 0;
   out_8922942910647886525[3] = 0;
   out_8922942910647886525[4] = 0;
   out_8922942910647886525[5] = 0;
   out_8922942910647886525[6] = 0;
   out_8922942910647886525[7] = 0;
   out_8922942910647886525[8] = 0;
}
void h_28(double *state, double *unused, double *out_3431293141612483349) {
   out_3431293141612483349[0] = state[0];
}
void H_28(double *state, double *unused, double *out_4441402145992134517) {
   out_4441402145992134517[0] = 1;
   out_4441402145992134517[1] = 0;
   out_4441402145992134517[2] = 0;
   out_4441402145992134517[3] = 0;
   out_4441402145992134517[4] = 0;
   out_4441402145992134517[5] = 0;
   out_4441402145992134517[6] = 0;
   out_4441402145992134517[7] = 0;
   out_4441402145992134517[8] = 0;
}
void h_31(double *state, double *unused, double *out_6312431023533613388) {
   out_6312431023533613388[0] = state[8];
}
void H_31(double *state, double *unused, double *out_2127525439132616580) {
   out_2127525439132616580[0] = 0;
   out_2127525439132616580[1] = 0;
   out_2127525439132616580[2] = 0;
   out_2127525439132616580[3] = 0;
   out_2127525439132616580[4] = 0;
   out_2127525439132616580[5] = 0;
   out_2127525439132616580[6] = 0;
   out_2127525439132616580[7] = 0;
   out_2127525439132616580[8] = 1;
}
#include <eigen3/Eigen/Dense>
#include <iostream>

typedef Eigen::Matrix<double, DIM, DIM, Eigen::RowMajor> DDM;
typedef Eigen::Matrix<double, EDIM, EDIM, Eigen::RowMajor> EEM;
typedef Eigen::Matrix<double, DIM, EDIM, Eigen::RowMajor> DEM;

void predict(double *in_x, double *in_P, double *in_Q, double dt) {
  typedef Eigen::Matrix<double, MEDIM, MEDIM, Eigen::RowMajor> RRM;

  double nx[DIM] = {0};
  double in_F[EDIM*EDIM] = {0};

  // functions from sympy
  f_fun(in_x, dt, nx);
  F_fun(in_x, dt, in_F);


  EEM F(in_F);
  EEM P(in_P);
  EEM Q(in_Q);

  RRM F_main = F.topLeftCorner(MEDIM, MEDIM);
  P.topLeftCorner(MEDIM, MEDIM) = (F_main * P.topLeftCorner(MEDIM, MEDIM)) * F_main.transpose();
  P.topRightCorner(MEDIM, EDIM - MEDIM) = F_main * P.topRightCorner(MEDIM, EDIM - MEDIM);
  P.bottomLeftCorner(EDIM - MEDIM, MEDIM) = P.bottomLeftCorner(EDIM - MEDIM, MEDIM) * F_main.transpose();

  P = P + dt*Q;

  // copy out state
  memcpy(in_x, nx, DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
}

// note: extra_args dim only correct when null space projecting
// otherwise 1
template <int ZDIM, int EADIM, bool MAHA_TEST>
void update(double *in_x, double *in_P, Hfun h_fun, Hfun H_fun, Hfun Hea_fun, double *in_z, double *in_R, double *in_ea, double MAHA_THRESHOLD) {
  typedef Eigen::Matrix<double, ZDIM, ZDIM, Eigen::RowMajor> ZZM;
  typedef Eigen::Matrix<double, ZDIM, DIM, Eigen::RowMajor> ZDM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, EDIM, Eigen::RowMajor> XEM;
  //typedef Eigen::Matrix<double, EDIM, ZDIM, Eigen::RowMajor> EZM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, 1> X1M;
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> XXM;

  double in_hx[ZDIM] = {0};
  double in_H[ZDIM * DIM] = {0};
  double in_H_mod[EDIM * DIM] = {0};
  double delta_x[EDIM] = {0};
  double x_new[DIM] = {0};


  // state x, P
  Eigen::Matrix<double, ZDIM, 1> z(in_z);
  EEM P(in_P);
  ZZM pre_R(in_R);

  // functions from sympy
  h_fun(in_x, in_ea, in_hx);
  H_fun(in_x, in_ea, in_H);
  ZDM pre_H(in_H);

  // get y (y = z - hx)
  Eigen::Matrix<double, ZDIM, 1> pre_y(in_hx); pre_y = z - pre_y;
  X1M y; XXM H; XXM R;
  if (Hea_fun){
    typedef Eigen::Matrix<double, ZDIM, EADIM, Eigen::RowMajor> ZAM;
    double in_Hea[ZDIM * EADIM] = {0};
    Hea_fun(in_x, in_ea, in_Hea);
    ZAM Hea(in_Hea);
    XXM A = Hea.transpose().fullPivLu().kernel();


    y = A.transpose() * pre_y;
    H = A.transpose() * pre_H;
    R = A.transpose() * pre_R * A;
  } else {
    y = pre_y;
    H = pre_H;
    R = pre_R;
  }
  // get modified H
  H_mod_fun(in_x, in_H_mod);
  DEM H_mod(in_H_mod);
  XEM H_err = H * H_mod;

  // Do mahalobis distance test
  if (MAHA_TEST){
    XXM a = (H_err * P * H_err.transpose() + R).inverse();
    double maha_dist = y.transpose() * a * y;
    if (maha_dist > MAHA_THRESHOLD){
      R = 1.0e16 * R;
    }
  }

  // Outlier resilient weighting
  double weight = 1;//(1.5)/(1 + y.squaredNorm()/R.sum());

  // kalman gains and I_KH
  XXM S = ((H_err * P) * H_err.transpose()) + R/weight;
  XEM KT = S.fullPivLu().solve(H_err * P.transpose());
  //EZM K = KT.transpose(); TODO: WHY DOES THIS NOT COMPILE?
  //EZM K = S.fullPivLu().solve(H_err * P.transpose()).transpose();
  //std::cout << "Here is the matrix rot:\n" << K << std::endl;
  EEM I_KH = Eigen::Matrix<double, EDIM, EDIM>::Identity() - (KT.transpose() * H_err);

  // update state by injecting dx
  Eigen::Matrix<double, EDIM, 1> dx(delta_x);
  dx  = (KT.transpose() * y);
  memcpy(delta_x, dx.data(), EDIM * sizeof(double));
  err_fun(in_x, delta_x, x_new);
  Eigen::Matrix<double, DIM, 1> x(x_new);

  // update cov
  P = ((I_KH * P) * I_KH.transpose()) + ((KT.transpose() * R) * KT);

  // copy out state
  memcpy(in_x, x.data(), DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
  memcpy(in_z, y.data(), y.rows() * sizeof(double));
}




}
extern "C" {

void car_update_25(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_25, H_25, NULL, in_z, in_R, in_ea, MAHA_THRESH_25);
}
void car_update_24(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<2, 3, 0>(in_x, in_P, h_24, H_24, NULL, in_z, in_R, in_ea, MAHA_THRESH_24);
}
void car_update_30(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_30, H_30, NULL, in_z, in_R, in_ea, MAHA_THRESH_30);
}
void car_update_26(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_26, H_26, NULL, in_z, in_R, in_ea, MAHA_THRESH_26);
}
void car_update_27(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_27, H_27, NULL, in_z, in_R, in_ea, MAHA_THRESH_27);
}
void car_update_29(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_29, H_29, NULL, in_z, in_R, in_ea, MAHA_THRESH_29);
}
void car_update_28(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_28, H_28, NULL, in_z, in_R, in_ea, MAHA_THRESH_28);
}
void car_update_31(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_31, H_31, NULL, in_z, in_R, in_ea, MAHA_THRESH_31);
}
void car_err_fun(double *nom_x, double *delta_x, double *out_6724727011957861295) {
  err_fun(nom_x, delta_x, out_6724727011957861295);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_3167941242115716840) {
  inv_err_fun(nom_x, true_x, out_3167941242115716840);
}
void car_H_mod_fun(double *state, double *out_5315706494890311132) {
  H_mod_fun(state, out_5315706494890311132);
}
void car_f_fun(double *state, double dt, double *out_8134846415775312522) {
  f_fun(state,  dt, out_8134846415775312522);
}
void car_F_fun(double *state, double dt, double *out_2041777415663342842) {
  F_fun(state,  dt, out_2041777415663342842);
}
void car_h_25(double *state, double *unused, double *out_4947360145717879899) {
  h_25(state, unused, out_4947360145717879899);
}
void car_H_25(double *state, double *unused, double *out_6495236860240024280) {
  H_25(state, unused, out_6495236860240024280);
}
void car_h_24(double *state, double *unused, double *out_4183217639544912511) {
  h_24(state, unused, out_4183217639544912511);
}
void car_H_24(double *state, double *unused, double *out_4322587261234524714) {
  H_24(state, unused, out_4322587261234524714);
}
void car_h_30(double *state, double *unused, double *out_3496728753088996732) {
  h_30(state, unused, out_3496728753088996732);
}
void car_H_30(double *state, double *unused, double *out_9013569818747272907) {
  H_30(state, unused, out_9013569818747272907);
}
void car_h_26(double *state, double *unused, double *out_2146001117991960103) {
  h_26(state, unused, out_2146001117991960103);
}
void car_H_26(double *state, double *unused, double *out_2753733541365968056) {
  H_26(state, unused, out_2753733541365968056);
}
void car_h_27(double *state, double *unused, double *out_9074023012693440184) {
  h_27(state, unused, out_9074023012693440184);
}
void car_H_27(double *state, double *unused, double *out_6838806506946847996) {
  H_27(state, unused, out_6838806506946847996);
}
void car_h_29(double *state, double *unused, double *out_2486195086217145021) {
  h_29(state, unused, out_2486195086217145021);
}
void car_H_29(double *state, double *unused, double *out_8922942910647886525) {
  H_29(state, unused, out_8922942910647886525);
}
void car_h_28(double *state, double *unused, double *out_3431293141612483349) {
  h_28(state, unused, out_3431293141612483349);
}
void car_H_28(double *state, double *unused, double *out_4441402145992134517) {
  H_28(state, unused, out_4441402145992134517);
}
void car_h_31(double *state, double *unused, double *out_6312431023533613388) {
  h_31(state, unused, out_6312431023533613388);
}
void car_H_31(double *state, double *unused, double *out_2127525439132616580) {
  H_31(state, unused, out_2127525439132616580);
}
void car_predict(double *in_x, double *in_P, double *in_Q, double dt) {
  predict(in_x, in_P, in_Q, dt);
}
void car_set_mass(double x) {
  set_mass(x);
}
void car_set_rotational_inertia(double x) {
  set_rotational_inertia(x);
}
void car_set_center_to_front(double x) {
  set_center_to_front(x);
}
void car_set_center_to_rear(double x) {
  set_center_to_rear(x);
}
void car_set_stiffness_front(double x) {
  set_stiffness_front(x);
}
void car_set_stiffness_rear(double x) {
  set_stiffness_rear(x);
}
}

const EKF car = {
  .name = "car",
  .kinds = { 25, 24, 30, 26, 27, 29, 28, 31 },
  .feature_kinds = {  },
  .f_fun = car_f_fun,
  .F_fun = car_F_fun,
  .err_fun = car_err_fun,
  .inv_err_fun = car_inv_err_fun,
  .H_mod_fun = car_H_mod_fun,
  .predict = car_predict,
  .hs = {
    { 25, car_h_25 },
    { 24, car_h_24 },
    { 30, car_h_30 },
    { 26, car_h_26 },
    { 27, car_h_27 },
    { 29, car_h_29 },
    { 28, car_h_28 },
    { 31, car_h_31 },
  },
  .Hs = {
    { 25, car_H_25 },
    { 24, car_H_24 },
    { 30, car_H_30 },
    { 26, car_H_26 },
    { 27, car_H_27 },
    { 29, car_H_29 },
    { 28, car_H_28 },
    { 31, car_H_31 },
  },
  .updates = {
    { 25, car_update_25 },
    { 24, car_update_24 },
    { 30, car_update_30 },
    { 26, car_update_26 },
    { 27, car_update_27 },
    { 29, car_update_29 },
    { 28, car_update_28 },
    { 31, car_update_31 },
  },
  .Hes = {
  },
  .sets = {
    { "mass", car_set_mass },
    { "rotational_inertia", car_set_rotational_inertia },
    { "center_to_front", car_set_center_to_front },
    { "center_to_rear", car_set_center_to_rear },
    { "stiffness_front", car_set_stiffness_front },
    { "stiffness_rear", car_set_stiffness_rear },
  },
  .extra_routines = {
  },
};

ekf_lib_init(car)
