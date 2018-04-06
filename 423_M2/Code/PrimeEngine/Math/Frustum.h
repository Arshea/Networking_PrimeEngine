#ifndef __PYENGINE_2_0_FRUSTUM_H__
#define __PYENGINE_2_0_FRUSTUM_H__


// APIAbstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
// Windows
#if APIABSTRACTION_D3D9 | APIABSTRACTION_D3D11
#define _USE_MATH_DEFINES
#include <math.h>
#endif
// Inter-Engine includes
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/../../GlobalConfig/GlobalConfig.h"

// Matrices
#include "PrimeEngine/Math/Matrix4x4.h"

#if PE_PLAT_IS_WIN32
#include <xmmintrin.h>
#include <pmmintrin.h>
#endif

// Plane information, constructed manually or using 3 points
struct Plane {
	Vector3 n; // Normal
	float d;

	Plane::Plane() {}

	void setParameters(Vector3 &A, Vector3 &B, Vector3 &C);

	void normalise() {
		float length = sqrt(pow(n.m_x, 2) + pow(n.m_y, 2) + pow(n.m_z, 2));
		n.m_x /= length;
		n.m_y /= length;
		n.m_z /= length;
		d /= length;
	}


};

//Frustum information, including 6 planes for camera projection
struct Frustum {
	Plane frustPlanes[6];
	Vector3 ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;
	float nearD, farD, ratio, angle, tang;
	float nw, nh, fw, fh; // Width and height of near and far planes

	enum planeEnum {
		NEAR_PLANE = 0, FAR_PLANE,
		LEFT_PLANE, RIGHT_PLANE,
		TOP_PLANE, BOTTOM_PLANE
	};

	Frustum::Frustum() {}
	Frustum::Frustum(float, float, float, float);

	void setCamDef(Vector3 &pos, Vector3 &target, Vector3 &up);

	bool isAabbInFrustum(float *min, float *max);

	/*
	void debugPrintPlane(Plane p) {
		char buffer[128];
		sprintf(buffer, "--PlaneDebug-- Normal: %f, %f, %f; d: %f\n", p.n.m_x, p.n.m_y, p.n.m_z, p.d);
		OutputDebugStringA(buffer);
	}

	void debugPrintVector(Vector3 &v) {
		char buffer[128];
		sprintf(buffer, "--VectorDebug-- %f, %f, %f\n", v.m_x, v.m_y, v.m_z);
		OutputDebugStringA(buffer);
	}
	*/

	/*
	Vector3 * debugPrintVisual() {
		Vector3 colorFront = { 1.0f, 0.0f, 1.0f };
		Vector3 colorBack = { 1.0f, 0.5f, 0.5f };
		Vector3 testPrint[24];

		for (int i = 1; i < 20; i += 2) {
			testPrint[i] = colorFront;
		}
		testPrint[21] = colorBack;
		testPrint[23] = colorBack;

		testPrint[0] = ntl;
		testPrint[2] = nbr;
		testPrint[4] = ntl;
		testPrint[6] = ntr;
		testPrint[8] = ntr;
		testPrint[10] = nbr;
		testPrint[12] = nbr;
		testPrint[14] = nbl;
		testPrint[16] = nbl;
		testPrint[18] = ntl;
		testPrint[20] = ftr;
		testPrint[22] = fbl;


		return(testPrint);
	}
	*/
};

#endif

