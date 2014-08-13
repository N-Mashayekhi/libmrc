
#include "mhd_riemann_private.h"
#include "ggcm_mhd_defs.h"
#include "ggcm_mhd_private.h"

#include <mrc_fld_as_double_aos.h>
#define F1(f, m, i) MRC_D2(f, m, i)

#include <math.h>

// ----------------------------------------------------------------------
// fluxes_cc

static void
fluxes_cc(mrc_fld_data_t F[5], mrc_fld_data_t U[5], mrc_fld_data_t W[5], 
	  mrc_fld_data_t gamma)
{
  F[RR]  = W[RR] * W[VX];
  F[RVX] = W[RR] * W[VX] * W[VX] + W[PP];
  F[RVY] = W[RR] * W[VY] * W[VX];
  F[RVZ] = W[RR] * W[VZ] * W[VX];
  F[UU]  = (U[UU] + W[PP]) * W[VX];
}

// ----------------------------------------------------------------------
// fluxes_hll

static void
fluxes_hydro_hll(mrc_fld_data_t F[5], mrc_fld_data_t Ul[5], mrc_fld_data_t Ur[5],
	       mrc_fld_data_t Wl[5], mrc_fld_data_t Wr[5], mrc_fld_data_t gamma)
{
  mrc_fld_data_t Fl[5], Fr[5];

  fluxes_cc(Fl, Ul, Wl, gamma);
  fluxes_cc(Fr, Ur, Wr, gamma);

  mrc_fld_data_t vv, cs2;

  cs2 = gamma * Wl[PP] / Wl[RR];
  mrc_fld_data_t cpv_l = Wl[VX] + sqrtf(cs2);
  mrc_fld_data_t cmv_l = Wl[VX] - sqrtf(cs2); 

  cs2 = gamma * Wr[PP] / Wr[RR];
  mrc_fld_data_t cpv_r = Wr[VX] + sqrtf(cs2);
  mrc_fld_data_t cmv_r = Wr[VX] - sqrtf(cs2); 

  mrc_fld_data_t SR =  fmaxf(fmaxf(cpv_l, cpv_r), 0.); 
  mrc_fld_data_t SL =  fminf(fminf(cmv_l, cmv_r), 0.); 

  //  mrc_fld_data_t lambda = .5 * (cmsv_l + cmsv_r);  
  for (int m = 0; m < 5; m++) {
    F[m] = ((SR * Fl[m] - SL * Fr[m]) + (SR * SL * (Ur[m] - Ul[m]))) / (SR - SL);
  }
}

// ----------------------------------------------------------------------
// mhd_riemann_hll_run

static void
mhd_riemann_hydro_hll_run(struct mhd_riemann *riemann, struct mrc_fld *F,
			struct mrc_fld *U_l, struct mrc_fld *U_r,
			struct mrc_fld *W_l, struct mrc_fld *W_r,
			int ldim, int l, int r, int dim)
{
  mrc_fld_data_t gamma = riemann->mhd->par.gamm;
  for (int i = -l; i < ldim + r; i++) {
    fluxes_hydro_hll(&F1(F, 0, i), &F1(U_l, 0, i), &F1(U_r, 0, i),
		   &F1(W_l, 0, i), &F1(W_r, 0, i), gamma);
  }
}

// ----------------------------------------------------------------------
// mhd_riemann_hydro_hll_ops

struct mhd_riemann_ops mhd_riemann_hydro_hll_ops = {
  .name             = "hydro_hll",
  .run              = mhd_riemann_hydro_hll_run,
};

