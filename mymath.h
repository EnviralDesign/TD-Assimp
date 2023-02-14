#pragma once
#include <math.h>
#include <iostream>
#include <cmath>

void cross(const float a[3], const float b[3], float result[3]) {
	result[0] = a[1] * b[2] - a[2] * b[1];
	result[1] = a[2] * b[0] - a[0] * b[2];
	result[2] = a[0] * b[1] - a[1] * b[0];
}

float dot_old(float vector_a[], float vector_b[]) {
	/* given two 3 component vectors, this function returns the dot product. */
	float product = 0;
	for (int i = 0; i < 3; i++)
		product = product + vector_a[i] * vector_b[i];
	return product;
}

float dot(const float a[3], const float b[3]) {
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

float length(const float a[3]) {
	return std::sqrt(dot(a, a));
}

void normalize(float a[3]) {
	float len = length(a);
	a[0] /= len;
	a[1] /= len;
	a[2] /= len;
}


#define CHAR_BIT 8

void tbn_to_quat(float tx, float ty, float tz, float tw, float bx, float by, float bz, float nx, float ny, float nz, float out[4]) {
	/**
	 * Packs the tangent frame represented by the specified matrix into a quaternion.
	 * Reflection is preserved by encoding it as the sign of the w component in the
	 * resulting quaternion. Since -0 cannot always be represented on the GPU, this
	 * function computes a bias to ensure values are always either positive or negative,
	 * never 0. The bias is computed based on the specified storageSize, which defaults
	 * to 2 bytes, making the resulting quaternion suitable for storage into an SNORM16
	 * vector.
	 *
	 * https://github.com/google/filament/blob/main/libs/math/include/math/mat3.h
	 *
	 * Note that the inverse-transpose of a matrix is equal to its cofactor matrix divided by its
	 * determinant:
	 *
	 *     transpose(inverse(M)) = cof(M) / det(M)
	 *
	 * The cofactor matrix is faster to compute than the inverse-transpose, and it can be argued
	 * that it is a more correct way of transforming normals anyway. Some references from Dale
	 * Weiler, Nathan Reed, Inigo Quilez, and Eric Lengyel:
	 *
	 *   - https://github.com/graphitemaster/normals_revisited
	 *   - http://www.reedbeta.com/blog/normals-inverse-transpose-part-1/
	 *   - https://www.shadertoy.com/view/3s33zj
	 *   - FGED Volume 1, section 1.7.5 "Inverses of Small Matrices"
	 *   - FGED Volume 1, section 3.2.2 "Transforming Normal Vectors"
	 *
	 * In "Transforming Normal Vectors", Lengyel notes that there are two types of transformed
	 * normals: one that uses the transposed adjugate (aka cofactor matrix) and one that uses the
	 * transposed inverse. He goes on to say that this difference is inconsequential, except when
	 * mirroring is involved.
	 *
	 * warning normals transformed by this matrix must be normalized
	 *
	 * tbn mat to quat code reference:
	 * https://github.com/toji/gl-matrix/blob/master/dist/esm/quat.js
	 * ~line 400 - fromMat3(out, m)
	*/

	// init the 4 component variable to hold the quaternion data.
	//float out[4];
	out[0] = 0; out[1] = 0; out[2] = 0; out[3] = 0;

	// init the 9 component array to hold intermediary tbn matrix.
	float m[9];
	m[0] = tx; m[1] = ty; m[2] = tz;
	m[3] = bx; m[4] = by; m[5] = bz;
	m[6] = nx; m[7] = ny; m[8] = nz;

	// init some other params.
	float fTrace = m[0] + m[4] + m[8];
	float fRoot;

	if (fTrace > 0.0) {
		// |w| > 1/2, may as well choose w > 1/2
		fRoot = pow(fTrace + 1.0, 0.5); // same as square root.

		out[3] = 0.5 * fRoot;
		fRoot = 0.5 / fRoot; // 1 / (4w)

		out[0] = (m[5] - m[7]) * fRoot;
		out[1] = (m[6] - m[2]) * fRoot;
		out[2] = (m[1] - m[3]) * fRoot;
	}
	else
	{
		// |w| <= 1/2
		int i = 0;
		if (m[4] > m[0]) {
			i = 1;
		}

		if (m[8] > m[i * 3 + i]) {
			i = 2;
		}

		int j = (i + 1) % 3;
		int k = (i + 2) % 3;
		fRoot = pow(m[i * 3 + i] - m[j * 3 + j] - m[k * 3 + k] + 1.0, 0.5); // same as square root.
		out[i] = 0.5 * fRoot;
		fRoot = 0.5 / fRoot;
		out[3] = (m[j * 3 + k] - m[k * 3 + j]) * fRoot;
		out[j] = (m[j * 3 + i] + m[i * 3 + j]) * fRoot;
		out[k] = (m[k * 3 + i] + m[i * 3 + k]) * fRoot;

		// calculate length/magnitude of quaternion.
		float quatMag = pow(
			(
				(out[0] * out[0]) +
				(out[1] * out[1]) +
				(out[2] * out[2]) +
				(out[3] * out[3])),
			0.5);

		// normalize quaternion. normalizing quats works like normalizing regular vec3's,
		// except with a 4th component.
		out[0] = out[0] / quatMag;
		out[1] = out[1] / quatMag;
		out[2] = out[2] / quatMag;
		out[3] = out[3] / quatMag;
	}

	// positivity check - flip signs if w is negative.
	if (out[3] < 0) {
		out[0] = -out[0];
		out[1] = -out[1];
		out[2] = -out[2];
		out[3] = -out[3];
	}

	// bias the quat, ensure w is never 0.0
	int storageSize = sizeof(int16_t);
	//const int char_bit = 8;
	float bias = 1 / (float)((1 << (storageSize * CHAR_BIT - 1)) - 1);

	if (out[3] < bias) {
		out[3] = bias;
		float factor = pow(1.0 - bias * bias, 0.5); // same as square root.
		out[0] *= factor;
		out[1] *= factor;
		out[2] *= factor;
	}

	float b[3];
	float n[3] = { nx,ny,nz };
	float t[3] = { tx,ty,tz };
	
	if (tw > 0) {
		cross(t, n, b);
	}
	else {
		cross(n, t, b);
	}

	// If there's a reflection ((n x t) . b <= 0), make sure w is negative
	float cc[3];
	cross(t, n, cc);
	if (dot(cc, b) < 0.0) {
		out[0] *= -out[0];
		out[1] *= -out[1];
		out[2] *= -out[2];
	}
}
