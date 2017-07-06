#include <math.h>
#include <stdlib.h>
#include "except.h"
#include "vector.h"
#include "circle.h"
#include "plane.h"
#include "calc_norm.h"

#define DELTA 7.7018
#define WAVELENGTH .123989

void calc_norm(Vector* emitters, size_t elen, Vector* middles, size_t mlen, Vector* detectors, size_t dlen, double* qs, size_t qlen, double* norms, size_t nlen, double sphere_radius, double ref_index) {

	double q_norms[dlen * 2 + 1];
	for(int d = 0; d < dlen; d++) {
		double complex detector_sum = 0;
		for(int m = 0; m < mlen; m++) {
			for(int e = 0; e < elen; e++) {

				Plane cart = p_from_points(emitters[e], middles[m], detectors[d]);

				Vector e_proj = p_project(cart, emitters[e]);
				Vector m_proj = p_project(cart, middles[m]);
				Vector d_proj = p_project(cart, detectors[d]);

				// This should be done somewhere else
				if (v_collinear(e_proj, m_proj, d_proj)) {
					raise_error(INVALID_ARGUMENT, "Points are collinear");
					return;
				}

				Circle large = c_from_points(e_proj, m_proj, d_proj);

				double dist = p_dist(cart, Vec3(0, 0, 0));
				double angle = acos(dist / sphere_radius);

				// Center of new cart plane
				Circle small = c_new(Vec2(0, 0), sphere_radius * sin(angle));

				Vector inter_pts[2];
				c_intersection(small, large, inter_pts);
				if(check_exception() != CLEAR) {
					raise_error(INVALID_ARGUMENT, "Circles did not intersect?");
					return;
				}

				double dists[2];
				for(int i = 0; i < 2; i++) {
					dists[i] = v_dist(inter_pts[i], e_proj);
				}
				// closer pt
				Vector e_pt = (dists[0] < dists[1]) ? inter_pts[0] : inter_pts[1];
				//further pt
				Vector r_pt = (dists[0] > dists[1]) ? inter_pts[0] : inter_pts[1];

				double complex l1 = c_arc_length(large, e_proj, e_pt) * 1e9 + 0 * I;
				double complex l2 = c_arc_length(large, e_pt, r_pt) * 1e9 + 0 * I;
				double complex l3 = c_arc_length(large, r_pt, d_proj) * 1e9 + 0 * I;

				double phase1 = carg(cexp(2 * M_PI * DELTA *  I * l1)) + DELTA * M_PI;
				double phase2 = carg(cexp(2 * M_PI * DELTA * ref_index * I * l2)) + DELTA * M_PI + phase1;
				double phase3 = carg(cexp(2 * M_PI * DELTA * I * l3)) + phase2;

				double complex wave = cexp(phase3 * I);

				detector_sum += wave;
			}
		}
		double angle_to_detector = tan(detectors[d].z / detectors[d].y) / 2;
		double q = 4 * M_PI * sin(angle_to_detector) / WAVELENGTH;
		double norm = log(pow(cabs(detector_sum), 2));
		qs[d] = q;
		norms[d] = norm;
	}
}


