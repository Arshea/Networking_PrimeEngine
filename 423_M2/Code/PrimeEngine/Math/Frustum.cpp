#include "PrimeEngine/Math/Frustum.h"

#include <stdio.h>

void Plane::setParameters(Vector3 &A, Vector3 &B, Vector3 &C) {
	Vector3 AB, BC;
	AB = B - A;
	BC = C - B;
	n = AB.crossProduct(BC);
	n.normalize();
	d = -n.dotProduct(A);
	//normalise();
}

Frustum::Frustum(float m_near, float m_far, float aspect, float verticalFov) {
	// Not sure how to set angle and ratio.
	angle = verticalFov / 2; // For testing reduced
	ratio = aspect;
	nearD = m_near;
	farD = /*m_far*/ /*For testing, reduced*/ 50.0f;

	tang = (float)tan(angle * 0.5);
	nh = nearD * tang;
	nw = nh * ratio;
	fh = farD  * tang;
	fw = fh * ratio;
}

void Frustum::setCamDef(Vector3 &pos, Vector3 &target, Vector3 &up) {
	Vector3 dir, nc, fc, X, Y, Z;

	// Calc Z axis of camera (opposite direction to looking)
	Z = pos - target;
	Z.normalize();

	// Calc X axis using "up" and z axis
	X = up.crossProduct(Z);
	X.normalize();

	// Y axis
	Y = Z.crossProduct(X);
	Y.normalize(); // Don't think this is necessary?

				   // Find centres of near and far planes
	nc = pos - (Z * nearD);
	fc = pos - (Z * farD);

	// Calc 4 corners of near plane
	ntl = nc + (Y * nh) - (X * nw);
	ntr = nc + (Y * nh) + (X * nw);
	nbl = nc - (Y * nh) - (X * nw);
	nbr = nc - (Y * nh) + (X * nw);

	// Calc 4 corners of far plane
	ftl = fc + (Y * fh) - (X * fw);
	ftr = fc + (Y * fh) + (X * fw);
	fbl = fc - (Y * fh) - (X * fw);
	fbr = fc - (Y * fh) + (X * fw);

	// Compute six planes (all clockwise order looking into frustum)
	frustPlanes[NEAR_PLANE].setParameters(ntl, ntr, nbr);
	frustPlanes[FAR_PLANE].setParameters(ftr, ftl, fbl);
	frustPlanes[LEFT_PLANE].setParameters(ftl, ntl, nbl);
	frustPlanes[RIGHT_PLANE].setParameters(ntr, ftr, fbr);
	frustPlanes[TOP_PLANE].setParameters(ftl, ftr, ntr);
	frustPlanes[BOTTOM_PLANE].setParameters(nbl, nbr, fbr);

	//debugPrintVector(ntl);
	//debugPrintPlane(frustPlanes[NEAR_PLANE]);

}

bool Frustum::isAabbInFrustum(float *min, float *max) {
	Vector3 positive; // Closest point to the plane
	
	for (int i = 0; i < 6; i++) {

		positive = { min[0], min[1], min[2] };
		if (frustPlanes[i].n.m_x >= 0) positive.m_x = max[0];
		if (frustPlanes[i].n.m_y >= 0) positive.m_y = max[1];
		if (frustPlanes[i].n.m_z >= 0) positive.m_z = max[2];

		float check = (positive.dotProduct(frustPlanes[i].n));
		if ((check + frustPlanes[i].d)<0) return false;

		/*
		char buffer[128];
		sprintf(buffer, "POSITIVE: %f\n", positive);
		OutputDebugStringA(buffer);
		*/
	}
	return true;
}