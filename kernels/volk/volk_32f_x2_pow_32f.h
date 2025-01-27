/* -*- c++ -*- */
/*
 * Copyright 2014 Free Software Foundation, Inc.
 *
 * This file is part of VOLK
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*!
 * \page volk_32f_x2_pow_32f
 *
 * \b Overview
 *
 * Raises the sample in aVector to the power of the number in bVector.
 *
 * c[i] = pow(a[i], b[i])
 *
 * <b>Dispatcher Prototype</b>
 * \code
 * void volk_32f_x2_pow_32f(float* cVector, const float* bVector, const float* aVector,
 * unsigned int num_points) \endcode
 *
 * \b Inputs
 * \li bVector: The input vector of indices (power values).
 * \li aVector: The input vector of base values.
 * \li num_points: The number of values in both input vectors.
 *
 * \b Outputs
 * \li cVector: The output vector.
 *
 * \b Example
 * Calculate the first two powers of two (2^x).
 * \code
 *   int N = 10;
 *   unsigned int alignment = volk_get_alignment();
 *   float* increasing = (float*)volk_malloc(sizeof(float)*N, alignment);
 *   float* twos = (float*)volk_malloc(sizeof(float)*N, alignment);
 *   float* out = (float*)volk_malloc(sizeof(float)*N, alignment);
 *
 *   for(unsigned int ii = 0; ii < N; ++ii){
 *       increasing[ii] = (float)ii;
 *       twos[ii] = 2.f;
 *   }
 *
 *   volk_32f_x2_pow_32f(out, increasing, twos, N);
 *
 *   for(unsigned int ii = 0; ii < N; ++ii){
 *       printf("out[%u] = %1.2f\n", ii, out[ii]);
 *   }
 *
 *   volk_free(increasing);
 *   volk_free(twos);
 *   volk_free(out);
 * \endcode
 */

#ifndef INCLUDED_volk_32f_x2_pow_32f_a_H
#define INCLUDED_volk_32f_x2_pow_32f_a_H

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define POW_POLY_DEGREE 3

#if LV_HAVE_AVX2 && LV_HAVE_FMA
#include <immintrin.h>

#define POLY0_AVX2_FMA(x, c0) _mm256_set1_ps(c0)
#define POLY1_AVX2_FMA(x, c0, c1) \
    _mm256_fmadd_ps(POLY0_AVX2_FMA(x, c1), x, _mm256_set1_ps(c0))
#define POLY2_AVX2_FMA(x, c0, c1, c2) \
    _mm256_fmadd_ps(POLY1_AVX2_FMA(x, c1, c2), x, _mm256_set1_ps(c0))
#define POLY3_AVX2_FMA(x, c0, c1, c2, c3) \
    _mm256_fmadd_ps(POLY2_AVX2_FMA(x, c1, c2, c3), x, _mm256_set1_ps(c0))
#define POLY4_AVX2_FMA(x, c0, c1, c2, c3, c4) \
    _mm256_fmadd_ps(POLY3_AVX2_FMA(x, c1, c2, c3, c4), x, _mm256_set1_ps(c0))
#define POLY5_AVX2_FMA(x, c0, c1, c2, c3, c4, c5) \
    _mm256_fmadd_ps(POLY4_AVX2_FMA(x, c1, c2, c3, c4, c5), x, _mm256_set1_ps(c0))

static inline void volk_32f_x2_pow_32f_a_avx2_fma(float* cVector,
                                                  const float* bVector,
                                                  const float* aVector,
                                                  unsigned int num_points)
{
    float* cPtr = cVector;
    const float* bPtr = bVector;
    const float* aPtr = aVector;

    unsigned int number = 0;
    const unsigned int eighthPoints = num_points / 8;

    __m256 aVal, bVal, cVal, logarithm, mantissa, frac, leadingOne;
    __m256 tmp, fx, mask, pow2n, z, y;
    __m256 one, exp_hi, exp_lo, ln2, log2EF, half, exp_C1, exp_C2;
    __m256 exp_p0, exp_p1, exp_p2, exp_p3, exp_p4, exp_p5;
    __m256i bias, exp, emm0, pi32_0x7f;

    one = _mm256_set1_ps(1.0);
    exp_hi = _mm256_set1_ps(88.3762626647949);
    exp_lo = _mm256_set1_ps(-88.3762626647949);
    ln2 = _mm256_set1_ps(0.6931471805);
    log2EF = _mm256_set1_ps(1.44269504088896341);
    half = _mm256_set1_ps(0.5);
    exp_C1 = _mm256_set1_ps(0.693359375);
    exp_C2 = _mm256_set1_ps(-2.12194440e-4);
    pi32_0x7f = _mm256_set1_epi32(0x7f);

    exp_p0 = _mm256_set1_ps(1.9875691500e-4);
    exp_p1 = _mm256_set1_ps(1.3981999507e-3);
    exp_p2 = _mm256_set1_ps(8.3334519073e-3);
    exp_p3 = _mm256_set1_ps(4.1665795894e-2);
    exp_p4 = _mm256_set1_ps(1.6666665459e-1);
    exp_p5 = _mm256_set1_ps(5.0000001201e-1);

    for (; number < eighthPoints; number++) {
        // First compute the logarithm
        aVal = _mm256_load_ps(aPtr);
        bias = _mm256_set1_epi32(127);
        leadingOne = _mm256_set1_ps(1.0f);
        exp = _mm256_sub_epi32(
            _mm256_srli_epi32(_mm256_and_si256(_mm256_castps_si256(aVal),
                                               _mm256_set1_epi32(0x7f800000)),
                              23),
            bias);
        logarithm = _mm256_cvtepi32_ps(exp);

        frac = _mm256_or_ps(
            leadingOne,
            _mm256_and_ps(aVal, _mm256_castsi256_ps(_mm256_set1_epi32(0x7fffff))));

#if POW_POLY_DEGREE == 6
        mantissa = POLY5_AVX2_FMA(frac,
                                  3.1157899f,
                                  -3.3241990f,
                                  2.5988452f,
                                  -1.2315303f,
                                  3.1821337e-1f,
                                  -3.4436006e-2f);
#elif POW_POLY_DEGREE == 5
        mantissa = POLY4_AVX2_FMA(frac,
                                  2.8882704548164776201f,
                                  -2.52074962577807006663f,
                                  1.48116647521213171641f,
                                  -0.465725644288844778798f,
                                  0.0596515482674574969533f);
#elif POW_POLY_DEGREE == 4
        mantissa = POLY3_AVX2_FMA(frac,
                                  2.61761038894603480148f,
                                  -1.75647175389045657003f,
                                  0.688243882994381274313f,
                                  -0.107254423828329604454f);
#elif POW_POLY_DEGREE == 3
        mantissa = POLY2_AVX2_FMA(frac,
                                  2.28330284476918490682f,
                                  -1.04913055217340124191f,
                                  0.204446009836232697516f);
#else
#error
#endif

        logarithm = _mm256_fmadd_ps(mantissa, _mm256_sub_ps(frac, leadingOne), logarithm);
        logarithm = _mm256_mul_ps(logarithm, ln2);

        // Now calculate b*lna
        bVal = _mm256_load_ps(bPtr);
        bVal = _mm256_mul_ps(bVal, logarithm);

        // Now compute exp(b*lna)
        bVal = _mm256_max_ps(_mm256_min_ps(bVal, exp_hi), exp_lo);

        fx = _mm256_fmadd_ps(bVal, log2EF, half);

        emm0 = _mm256_cvttps_epi32(fx);
        tmp = _mm256_cvtepi32_ps(emm0);

        mask = _mm256_and_ps(_mm256_cmp_ps(tmp, fx, _CMP_GT_OS), one);
        fx = _mm256_sub_ps(tmp, mask);

        tmp = _mm256_fnmadd_ps(fx, exp_C1, bVal);
        bVal = _mm256_fnmadd_ps(fx, exp_C2, tmp);
        z = _mm256_mul_ps(bVal, bVal);

        y = _mm256_fmadd_ps(exp_p0, bVal, exp_p1);
        y = _mm256_fmadd_ps(y, bVal, exp_p2);
        y = _mm256_fmadd_ps(y, bVal, exp_p3);
        y = _mm256_fmadd_ps(y, bVal, exp_p4);
        y = _mm256_fmadd_ps(y, bVal, exp_p5);
        y = _mm256_fmadd_ps(y, z, bVal);
        y = _mm256_add_ps(y, one);

        emm0 =
            _mm256_slli_epi32(_mm256_add_epi32(_mm256_cvttps_epi32(fx), pi32_0x7f), 23);

        pow2n = _mm256_castsi256_ps(emm0);
        cVal = _mm256_mul_ps(y, pow2n);

        _mm256_store_ps(cPtr, cVal);

        aPtr += 8;
        bPtr += 8;
        cPtr += 8;
    }

    number = eighthPoints * 8;
    for (; number < num_points; number++) {
        *cPtr++ = pow(*aPtr++, *bPtr++);
    }
}

#endif /* LV_HAVE_AVX2 && LV_HAVE_FMA for aligned */

#ifdef LV_HAVE_AVX2
#include <immintrin.h>

#define POLY0_AVX2(x, c0) _mm256_set1_ps(c0)
#define POLY1_AVX2(x, c0, c1) \
    _mm256_add_ps(_mm256_mul_ps(POLY0_AVX2(x, c1), x), _mm256_set1_ps(c0))
#define POLY2_AVX2(x, c0, c1, c2) \
    _mm256_add_ps(_mm256_mul_ps(POLY1_AVX2(x, c1, c2), x), _mm256_set1_ps(c0))
#define POLY3_AVX2(x, c0, c1, c2, c3) \
    _mm256_add_ps(_mm256_mul_ps(POLY2_AVX2(x, c1, c2, c3), x), _mm256_set1_ps(c0))
#define POLY4_AVX2(x, c0, c1, c2, c3, c4) \
    _mm256_add_ps(_mm256_mul_ps(POLY3_AVX2(x, c1, c2, c3, c4), x), _mm256_set1_ps(c0))
#define POLY5_AVX2(x, c0, c1, c2, c3, c4, c5) \
    _mm256_add_ps(_mm256_mul_ps(POLY4_AVX2(x, c1, c2, c3, c4, c5), x), _mm256_set1_ps(c0))

static inline void volk_32f_x2_pow_32f_a_avx2(float* cVector,
                                              const float* bVector,
                                              const float* aVector,
                                              unsigned int num_points)
{
    float* cPtr = cVector;
    const float* bPtr = bVector;
    const float* aPtr = aVector;

    unsigned int number = 0;
    const unsigned int eighthPoints = num_points / 8;

    __m256 aVal, bVal, cVal, logarithm, mantissa, frac, leadingOne;
    __m256 tmp, fx, mask, pow2n, z, y;
    __m256 one, exp_hi, exp_lo, ln2, log2EF, half, exp_C1, exp_C2;
    __m256 exp_p0, exp_p1, exp_p2, exp_p3, exp_p4, exp_p5;
    __m256i bias, exp, emm0, pi32_0x7f;

    one = _mm256_set1_ps(1.0);
    exp_hi = _mm256_set1_ps(88.3762626647949);
    exp_lo = _mm256_set1_ps(-88.3762626647949);
    ln2 = _mm256_set1_ps(0.6931471805);
    log2EF = _mm256_set1_ps(1.44269504088896341);
    half = _mm256_set1_ps(0.5);
    exp_C1 = _mm256_set1_ps(0.693359375);
    exp_C2 = _mm256_set1_ps(-2.12194440e-4);
    pi32_0x7f = _mm256_set1_epi32(0x7f);

    exp_p0 = _mm256_set1_ps(1.9875691500e-4);
    exp_p1 = _mm256_set1_ps(1.3981999507e-3);
    exp_p2 = _mm256_set1_ps(8.3334519073e-3);
    exp_p3 = _mm256_set1_ps(4.1665795894e-2);
    exp_p4 = _mm256_set1_ps(1.6666665459e-1);
    exp_p5 = _mm256_set1_ps(5.0000001201e-1);

    for (; number < eighthPoints; number++) {
        // First compute the logarithm
        aVal = _mm256_load_ps(aPtr);
        bias = _mm256_set1_epi32(127);
        leadingOne = _mm256_set1_ps(1.0f);
        exp = _mm256_sub_epi32(
            _mm256_srli_epi32(_mm256_and_si256(_mm256_castps_si256(aVal),
                                               _mm256_set1_epi32(0x7f800000)),
                              23),
            bias);
        logarithm = _mm256_cvtepi32_ps(exp);

        frac = _mm256_or_ps(
            leadingOne,
            _mm256_and_ps(aVal, _mm256_castsi256_ps(_mm256_set1_epi32(0x7fffff))));

#if POW_POLY_DEGREE == 6
        mantissa = POLY5_AVX2(frac,
                              3.1157899f,
                              -3.3241990f,
                              2.5988452f,
                              -1.2315303f,
                              3.1821337e-1f,
                              -3.4436006e-2f);
#elif POW_POLY_DEGREE == 5
        mantissa = POLY4_AVX2(frac,
                              2.8882704548164776201f,
                              -2.52074962577807006663f,
                              1.48116647521213171641f,
                              -0.465725644288844778798f,
                              0.0596515482674574969533f);
#elif POW_POLY_DEGREE == 4
        mantissa = POLY3_AVX2(frac,
                              2.61761038894603480148f,
                              -1.75647175389045657003f,
                              0.688243882994381274313f,
                              -0.107254423828329604454f);
#elif POW_POLY_DEGREE == 3
        mantissa = POLY2_AVX2(frac,
                              2.28330284476918490682f,
                              -1.04913055217340124191f,
                              0.204446009836232697516f);
#else
#error
#endif

        logarithm = _mm256_add_ps(
            _mm256_mul_ps(mantissa, _mm256_sub_ps(frac, leadingOne)), logarithm);
        logarithm = _mm256_mul_ps(logarithm, ln2);

        // Now calculate b*lna
        bVal = _mm256_load_ps(bPtr);
        bVal = _mm256_mul_ps(bVal, logarithm);

        // Now compute exp(b*lna)
        bVal = _mm256_max_ps(_mm256_min_ps(bVal, exp_hi), exp_lo);

        fx = _mm256_add_ps(_mm256_mul_ps(bVal, log2EF), half);

        emm0 = _mm256_cvttps_epi32(fx);
        tmp = _mm256_cvtepi32_ps(emm0);

        mask = _mm256_and_ps(_mm256_cmp_ps(tmp, fx, _CMP_GT_OS), one);
        fx = _mm256_sub_ps(tmp, mask);

        tmp = _mm256_sub_ps(bVal, _mm256_mul_ps(fx, exp_C1));
        bVal = _mm256_sub_ps(tmp, _mm256_mul_ps(fx, exp_C2));
        z = _mm256_mul_ps(bVal, bVal);

        y = _mm256_add_ps(_mm256_mul_ps(exp_p0, bVal), exp_p1);
        y = _mm256_add_ps(_mm256_mul_ps(y, bVal), exp_p2);
        y = _mm256_add_ps(_mm256_mul_ps(y, bVal), exp_p3);
        y = _mm256_add_ps(_mm256_mul_ps(y, bVal), exp_p4);
        y = _mm256_add_ps(_mm256_mul_ps(y, bVal), exp_p5);
        y = _mm256_add_ps(_mm256_mul_ps(y, z), bVal);
        y = _mm256_add_ps(y, one);

        emm0 =
            _mm256_slli_epi32(_mm256_add_epi32(_mm256_cvttps_epi32(fx), pi32_0x7f), 23);

        pow2n = _mm256_castsi256_ps(emm0);
        cVal = _mm256_mul_ps(y, pow2n);

        _mm256_store_ps(cPtr, cVal);

        aPtr += 8;
        bPtr += 8;
        cPtr += 8;
    }

    number = eighthPoints * 8;
    for (; number < num_points; number++) {
        *cPtr++ = pow(*aPtr++, *bPtr++);
    }
}

#endif /* LV_HAVE_AVX2 for aligned */


#ifdef LV_HAVE_SSE4_1
#include <smmintrin.h>

#define POLY0(x, c0) _mm_set1_ps(c0)
#define POLY1(x, c0, c1) _mm_add_ps(_mm_mul_ps(POLY0(x, c1), x), _mm_set1_ps(c0))
#define POLY2(x, c0, c1, c2) _mm_add_ps(_mm_mul_ps(POLY1(x, c1, c2), x), _mm_set1_ps(c0))
#define POLY3(x, c0, c1, c2, c3) \
    _mm_add_ps(_mm_mul_ps(POLY2(x, c1, c2, c3), x), _mm_set1_ps(c0))
#define POLY4(x, c0, c1, c2, c3, c4) \
    _mm_add_ps(_mm_mul_ps(POLY3(x, c1, c2, c3, c4), x), _mm_set1_ps(c0))
#define POLY5(x, c0, c1, c2, c3, c4, c5) \
    _mm_add_ps(_mm_mul_ps(POLY4(x, c1, c2, c3, c4, c5), x), _mm_set1_ps(c0))

static inline void volk_32f_x2_pow_32f_a_sse4_1(float* cVector,
                                                const float* bVector,
                                                const float* aVector,
                                                unsigned int num_points)
{
    float* cPtr = cVector;
    const float* bPtr = bVector;
    const float* aPtr = aVector;

    unsigned int number = 0;
    const unsigned int quarterPoints = num_points / 4;

    __m128 aVal, bVal, cVal, logarithm, mantissa, frac, leadingOne;
    __m128 tmp, fx, mask, pow2n, z, y;
    __m128 one, exp_hi, exp_lo, ln2, log2EF, half, exp_C1, exp_C2;
    __m128 exp_p0, exp_p1, exp_p2, exp_p3, exp_p4, exp_p5;
    __m128i bias, exp, emm0, pi32_0x7f;

    one = _mm_set1_ps(1.0);
    exp_hi = _mm_set1_ps(88.3762626647949);
    exp_lo = _mm_set1_ps(-88.3762626647949);
    ln2 = _mm_set1_ps(0.6931471805);
    log2EF = _mm_set1_ps(1.44269504088896341);
    half = _mm_set1_ps(0.5);
    exp_C1 = _mm_set1_ps(0.693359375);
    exp_C2 = _mm_set1_ps(-2.12194440e-4);
    pi32_0x7f = _mm_set1_epi32(0x7f);

    exp_p0 = _mm_set1_ps(1.9875691500e-4);
    exp_p1 = _mm_set1_ps(1.3981999507e-3);
    exp_p2 = _mm_set1_ps(8.3334519073e-3);
    exp_p3 = _mm_set1_ps(4.1665795894e-2);
    exp_p4 = _mm_set1_ps(1.6666665459e-1);
    exp_p5 = _mm_set1_ps(5.0000001201e-1);

    for (; number < quarterPoints; number++) {
        // First compute the logarithm
        aVal = _mm_load_ps(aPtr);
        bias = _mm_set1_epi32(127);
        leadingOne = _mm_set1_ps(1.0f);
        exp = _mm_sub_epi32(
            _mm_srli_epi32(
                _mm_and_si128(_mm_castps_si128(aVal), _mm_set1_epi32(0x7f800000)), 23),
            bias);
        logarithm = _mm_cvtepi32_ps(exp);

        frac = _mm_or_ps(leadingOne,
                         _mm_and_ps(aVal, _mm_castsi128_ps(_mm_set1_epi32(0x7fffff))));

#if POW_POLY_DEGREE == 6
        mantissa = POLY5(frac,
                         3.1157899f,
                         -3.3241990f,
                         2.5988452f,
                         -1.2315303f,
                         3.1821337e-1f,
                         -3.4436006e-2f);
#elif POW_POLY_DEGREE == 5
        mantissa = POLY4(frac,
                         2.8882704548164776201f,
                         -2.52074962577807006663f,
                         1.48116647521213171641f,
                         -0.465725644288844778798f,
                         0.0596515482674574969533f);
#elif POW_POLY_DEGREE == 4
        mantissa = POLY3(frac,
                         2.61761038894603480148f,
                         -1.75647175389045657003f,
                         0.688243882994381274313f,
                         -0.107254423828329604454f);
#elif POW_POLY_DEGREE == 3
        mantissa = POLY2(frac,
                         2.28330284476918490682f,
                         -1.04913055217340124191f,
                         0.204446009836232697516f);
#else
#error
#endif

        logarithm =
            _mm_add_ps(logarithm, _mm_mul_ps(mantissa, _mm_sub_ps(frac, leadingOne)));
        logarithm = _mm_mul_ps(logarithm, ln2);


        // Now calculate b*lna
        bVal = _mm_load_ps(bPtr);
        bVal = _mm_mul_ps(bVal, logarithm);

        // Now compute exp(b*lna)
        bVal = _mm_max_ps(_mm_min_ps(bVal, exp_hi), exp_lo);

        fx = _mm_add_ps(_mm_mul_ps(bVal, log2EF), half);

        emm0 = _mm_cvttps_epi32(fx);
        tmp = _mm_cvtepi32_ps(emm0);

        mask = _mm_and_ps(_mm_cmpgt_ps(tmp, fx), one);
        fx = _mm_sub_ps(tmp, mask);

        tmp = _mm_mul_ps(fx, exp_C1);
        z = _mm_mul_ps(fx, exp_C2);
        bVal = _mm_sub_ps(_mm_sub_ps(bVal, tmp), z);
        z = _mm_mul_ps(bVal, bVal);

        y = _mm_mul_ps(_mm_add_ps(_mm_mul_ps(exp_p0, bVal), exp_p1), bVal);
        y = _mm_add_ps(_mm_mul_ps(_mm_add_ps(y, exp_p2), bVal), exp_p3);
        y = _mm_mul_ps(_mm_add_ps(_mm_mul_ps(y, bVal), exp_p4), bVal);
        y = _mm_add_ps(_mm_mul_ps(_mm_add_ps(y, exp_p5), z), bVal);
        y = _mm_add_ps(y, one);

        emm0 = _mm_slli_epi32(_mm_add_epi32(_mm_cvttps_epi32(fx), pi32_0x7f), 23);

        pow2n = _mm_castsi128_ps(emm0);
        cVal = _mm_mul_ps(y, pow2n);

        _mm_store_ps(cPtr, cVal);

        aPtr += 4;
        bPtr += 4;
        cPtr += 4;
    }

    number = quarterPoints * 4;
    for (; number < num_points; number++) {
        *cPtr++ = powf(*aPtr++, *bPtr++);
    }
}

#endif /* LV_HAVE_SSE4_1 for aligned */

#endif /* INCLUDED_volk_32f_x2_pow_32f_a_H */

#ifndef INCLUDED_volk_32f_x2_pow_32f_u_H
#define INCLUDED_volk_32f_x2_pow_32f_u_H

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define POW_POLY_DEGREE 3

#ifdef LV_HAVE_GENERIC

static inline void volk_32f_x2_pow_32f_generic(float* cVector,
                                               const float* bVector,
                                               const float* aVector,
                                               unsigned int num_points)
{
    float* cPtr = cVector;
    const float* bPtr = bVector;
    const float* aPtr = aVector;
    unsigned int number = 0;

    for (number = 0; number < num_points; number++) {
        *cPtr++ = powf(*aPtr++, *bPtr++);
    }
}
#endif /* LV_HAVE_GENERIC */


#ifdef LV_HAVE_SSE4_1
#include <smmintrin.h>

#define POLY0(x, c0) _mm_set1_ps(c0)
#define POLY1(x, c0, c1) _mm_add_ps(_mm_mul_ps(POLY0(x, c1), x), _mm_set1_ps(c0))
#define POLY2(x, c0, c1, c2) _mm_add_ps(_mm_mul_ps(POLY1(x, c1, c2), x), _mm_set1_ps(c0))
#define POLY3(x, c0, c1, c2, c3) \
    _mm_add_ps(_mm_mul_ps(POLY2(x, c1, c2, c3), x), _mm_set1_ps(c0))
#define POLY4(x, c0, c1, c2, c3, c4) \
    _mm_add_ps(_mm_mul_ps(POLY3(x, c1, c2, c3, c4), x), _mm_set1_ps(c0))
#define POLY5(x, c0, c1, c2, c3, c4, c5) \
    _mm_add_ps(_mm_mul_ps(POLY4(x, c1, c2, c3, c4, c5), x), _mm_set1_ps(c0))

static inline void volk_32f_x2_pow_32f_u_sse4_1(float* cVector,
                                                const float* bVector,
                                                const float* aVector,
                                                unsigned int num_points)
{
    float* cPtr = cVector;
    const float* bPtr = bVector;
    const float* aPtr = aVector;

    unsigned int number = 0;
    const unsigned int quarterPoints = num_points / 4;

    __m128 aVal, bVal, cVal, logarithm, mantissa, frac, leadingOne;
    __m128 tmp, fx, mask, pow2n, z, y;
    __m128 one, exp_hi, exp_lo, ln2, log2EF, half, exp_C1, exp_C2;
    __m128 exp_p0, exp_p1, exp_p2, exp_p3, exp_p4, exp_p5;
    __m128i bias, exp, emm0, pi32_0x7f;

    one = _mm_set1_ps(1.0);
    exp_hi = _mm_set1_ps(88.3762626647949);
    exp_lo = _mm_set1_ps(-88.3762626647949);
    ln2 = _mm_set1_ps(0.6931471805);
    log2EF = _mm_set1_ps(1.44269504088896341);
    half = _mm_set1_ps(0.5);
    exp_C1 = _mm_set1_ps(0.693359375);
    exp_C2 = _mm_set1_ps(-2.12194440e-4);
    pi32_0x7f = _mm_set1_epi32(0x7f);

    exp_p0 = _mm_set1_ps(1.9875691500e-4);
    exp_p1 = _mm_set1_ps(1.3981999507e-3);
    exp_p2 = _mm_set1_ps(8.3334519073e-3);
    exp_p3 = _mm_set1_ps(4.1665795894e-2);
    exp_p4 = _mm_set1_ps(1.6666665459e-1);
    exp_p5 = _mm_set1_ps(5.0000001201e-1);

    for (; number < quarterPoints; number++) {
        // First compute the logarithm
        aVal = _mm_loadu_ps(aPtr);
        bias = _mm_set1_epi32(127);
        leadingOne = _mm_set1_ps(1.0f);
        exp = _mm_sub_epi32(
            _mm_srli_epi32(
                _mm_and_si128(_mm_castps_si128(aVal), _mm_set1_epi32(0x7f800000)), 23),
            bias);
        logarithm = _mm_cvtepi32_ps(exp);

        frac = _mm_or_ps(leadingOne,
                         _mm_and_ps(aVal, _mm_castsi128_ps(_mm_set1_epi32(0x7fffff))));

#if POW_POLY_DEGREE == 6
        mantissa = POLY5(frac,
                         3.1157899f,
                         -3.3241990f,
                         2.5988452f,
                         -1.2315303f,
                         3.1821337e-1f,
                         -3.4436006e-2f);
#elif POW_POLY_DEGREE == 5
        mantissa = POLY4(frac,
                         2.8882704548164776201f,
                         -2.52074962577807006663f,
                         1.48116647521213171641f,
                         -0.465725644288844778798f,
                         0.0596515482674574969533f);
#elif POW_POLY_DEGREE == 4
        mantissa = POLY3(frac,
                         2.61761038894603480148f,
                         -1.75647175389045657003f,
                         0.688243882994381274313f,
                         -0.107254423828329604454f);
#elif POW_POLY_DEGREE == 3
        mantissa = POLY2(frac,
                         2.28330284476918490682f,
                         -1.04913055217340124191f,
                         0.204446009836232697516f);
#else
#error
#endif

        logarithm =
            _mm_add_ps(logarithm, _mm_mul_ps(mantissa, _mm_sub_ps(frac, leadingOne)));
        logarithm = _mm_mul_ps(logarithm, ln2);


        // Now calculate b*lna
        bVal = _mm_loadu_ps(bPtr);
        bVal = _mm_mul_ps(bVal, logarithm);

        // Now compute exp(b*lna)
        bVal = _mm_max_ps(_mm_min_ps(bVal, exp_hi), exp_lo);

        fx = _mm_add_ps(_mm_mul_ps(bVal, log2EF), half);

        emm0 = _mm_cvttps_epi32(fx);
        tmp = _mm_cvtepi32_ps(emm0);

        mask = _mm_and_ps(_mm_cmpgt_ps(tmp, fx), one);
        fx = _mm_sub_ps(tmp, mask);

        tmp = _mm_mul_ps(fx, exp_C1);
        z = _mm_mul_ps(fx, exp_C2);
        bVal = _mm_sub_ps(_mm_sub_ps(bVal, tmp), z);
        z = _mm_mul_ps(bVal, bVal);

        y = _mm_mul_ps(_mm_add_ps(_mm_mul_ps(exp_p0, bVal), exp_p1), bVal);
        y = _mm_add_ps(_mm_mul_ps(_mm_add_ps(y, exp_p2), bVal), exp_p3);
        y = _mm_mul_ps(_mm_add_ps(_mm_mul_ps(y, bVal), exp_p4), bVal);
        y = _mm_add_ps(_mm_mul_ps(_mm_add_ps(y, exp_p5), z), bVal);
        y = _mm_add_ps(y, one);

        emm0 = _mm_slli_epi32(_mm_add_epi32(_mm_cvttps_epi32(fx), pi32_0x7f), 23);

        pow2n = _mm_castsi128_ps(emm0);
        cVal = _mm_mul_ps(y, pow2n);

        _mm_storeu_ps(cPtr, cVal);

        aPtr += 4;
        bPtr += 4;
        cPtr += 4;
    }

    number = quarterPoints * 4;
    for (; number < num_points; number++) {
        *cPtr++ = powf(*aPtr++, *bPtr++);
    }
}

#endif /* LV_HAVE_SSE4_1 for unaligned */

#if LV_HAVE_AVX2 && LV_HAVE_FMA
#include <immintrin.h>

#define POLY0_AVX2_FMA(x, c0) _mm256_set1_ps(c0)
#define POLY1_AVX2_FMA(x, c0, c1) \
    _mm256_fmadd_ps(POLY0_AVX2_FMA(x, c1), x, _mm256_set1_ps(c0))
#define POLY2_AVX2_FMA(x, c0, c1, c2) \
    _mm256_fmadd_ps(POLY1_AVX2_FMA(x, c1, c2), x, _mm256_set1_ps(c0))
#define POLY3_AVX2_FMA(x, c0, c1, c2, c3) \
    _mm256_fmadd_ps(POLY2_AVX2_FMA(x, c1, c2, c3), x, _mm256_set1_ps(c0))
#define POLY4_AVX2_FMA(x, c0, c1, c2, c3, c4) \
    _mm256_fmadd_ps(POLY3_AVX2_FMA(x, c1, c2, c3, c4), x, _mm256_set1_ps(c0))
#define POLY5_AVX2_FMA(x, c0, c1, c2, c3, c4, c5) \
    _mm256_fmadd_ps(POLY4_AVX2_FMA(x, c1, c2, c3, c4, c5), x, _mm256_set1_ps(c0))

static inline void volk_32f_x2_pow_32f_u_avx2_fma(float* cVector,
                                                  const float* bVector,
                                                  const float* aVector,
                                                  unsigned int num_points)
{
    float* cPtr = cVector;
    const float* bPtr = bVector;
    const float* aPtr = aVector;

    unsigned int number = 0;
    const unsigned int eighthPoints = num_points / 8;

    __m256 aVal, bVal, cVal, logarithm, mantissa, frac, leadingOne;
    __m256 tmp, fx, mask, pow2n, z, y;
    __m256 one, exp_hi, exp_lo, ln2, log2EF, half, exp_C1, exp_C2;
    __m256 exp_p0, exp_p1, exp_p2, exp_p3, exp_p4, exp_p5;
    __m256i bias, exp, emm0, pi32_0x7f;

    one = _mm256_set1_ps(1.0);
    exp_hi = _mm256_set1_ps(88.3762626647949);
    exp_lo = _mm256_set1_ps(-88.3762626647949);
    ln2 = _mm256_set1_ps(0.6931471805);
    log2EF = _mm256_set1_ps(1.44269504088896341);
    half = _mm256_set1_ps(0.5);
    exp_C1 = _mm256_set1_ps(0.693359375);
    exp_C2 = _mm256_set1_ps(-2.12194440e-4);
    pi32_0x7f = _mm256_set1_epi32(0x7f);

    exp_p0 = _mm256_set1_ps(1.9875691500e-4);
    exp_p1 = _mm256_set1_ps(1.3981999507e-3);
    exp_p2 = _mm256_set1_ps(8.3334519073e-3);
    exp_p3 = _mm256_set1_ps(4.1665795894e-2);
    exp_p4 = _mm256_set1_ps(1.6666665459e-1);
    exp_p5 = _mm256_set1_ps(5.0000001201e-1);

    for (; number < eighthPoints; number++) {
        // First compute the logarithm
        aVal = _mm256_loadu_ps(aPtr);
        bias = _mm256_set1_epi32(127);
        leadingOne = _mm256_set1_ps(1.0f);
        exp = _mm256_sub_epi32(
            _mm256_srli_epi32(_mm256_and_si256(_mm256_castps_si256(aVal),
                                               _mm256_set1_epi32(0x7f800000)),
                              23),
            bias);
        logarithm = _mm256_cvtepi32_ps(exp);

        frac = _mm256_or_ps(
            leadingOne,
            _mm256_and_ps(aVal, _mm256_castsi256_ps(_mm256_set1_epi32(0x7fffff))));

#if POW_POLY_DEGREE == 6
        mantissa = POLY5_AVX2_FMA(frac,
                                  3.1157899f,
                                  -3.3241990f,
                                  2.5988452f,
                                  -1.2315303f,
                                  3.1821337e-1f,
                                  -3.4436006e-2f);
#elif POW_POLY_DEGREE == 5
        mantissa = POLY4_AVX2_FMA(frac,
                                  2.8882704548164776201f,
                                  -2.52074962577807006663f,
                                  1.48116647521213171641f,
                                  -0.465725644288844778798f,
                                  0.0596515482674574969533f);
#elif POW_POLY_DEGREE == 4
        mantissa = POLY3_AVX2_FMA(frac,
                                  2.61761038894603480148f,
                                  -1.75647175389045657003f,
                                  0.688243882994381274313f,
                                  -0.107254423828329604454f);
#elif POW_POLY_DEGREE == 3
        mantissa = POLY2_AVX2_FMA(frac,
                                  2.28330284476918490682f,
                                  -1.04913055217340124191f,
                                  0.204446009836232697516f);
#else
#error
#endif

        logarithm = _mm256_fmadd_ps(mantissa, _mm256_sub_ps(frac, leadingOne), logarithm);
        logarithm = _mm256_mul_ps(logarithm, ln2);


        // Now calculate b*lna
        bVal = _mm256_loadu_ps(bPtr);
        bVal = _mm256_mul_ps(bVal, logarithm);

        // Now compute exp(b*lna)
        bVal = _mm256_max_ps(_mm256_min_ps(bVal, exp_hi), exp_lo);

        fx = _mm256_fmadd_ps(bVal, log2EF, half);

        emm0 = _mm256_cvttps_epi32(fx);
        tmp = _mm256_cvtepi32_ps(emm0);

        mask = _mm256_and_ps(_mm256_cmp_ps(tmp, fx, _CMP_GT_OS), one);
        fx = _mm256_sub_ps(tmp, mask);

        tmp = _mm256_fnmadd_ps(fx, exp_C1, bVal);
        bVal = _mm256_fnmadd_ps(fx, exp_C2, tmp);
        z = _mm256_mul_ps(bVal, bVal);

        y = _mm256_fmadd_ps(exp_p0, bVal, exp_p1);
        y = _mm256_fmadd_ps(y, bVal, exp_p2);
        y = _mm256_fmadd_ps(y, bVal, exp_p3);
        y = _mm256_fmadd_ps(y, bVal, exp_p4);
        y = _mm256_fmadd_ps(y, bVal, exp_p5);
        y = _mm256_fmadd_ps(y, z, bVal);
        y = _mm256_add_ps(y, one);

        emm0 =
            _mm256_slli_epi32(_mm256_add_epi32(_mm256_cvttps_epi32(fx), pi32_0x7f), 23);

        pow2n = _mm256_castsi256_ps(emm0);
        cVal = _mm256_mul_ps(y, pow2n);

        _mm256_storeu_ps(cPtr, cVal);

        aPtr += 8;
        bPtr += 8;
        cPtr += 8;
    }

    number = eighthPoints * 8;
    for (; number < num_points; number++) {
        *cPtr++ = pow(*aPtr++, *bPtr++);
    }
}

#endif /* LV_HAVE_AVX2 && LV_HAVE_FMA for unaligned */

#ifdef LV_HAVE_AVX2
#include <immintrin.h>

#define POLY0_AVX2(x, c0) _mm256_set1_ps(c0)
#define POLY1_AVX2(x, c0, c1) \
    _mm256_add_ps(_mm256_mul_ps(POLY0_AVX2(x, c1), x), _mm256_set1_ps(c0))
#define POLY2_AVX2(x, c0, c1, c2) \
    _mm256_add_ps(_mm256_mul_ps(POLY1_AVX2(x, c1, c2), x), _mm256_set1_ps(c0))
#define POLY3_AVX2(x, c0, c1, c2, c3) \
    _mm256_add_ps(_mm256_mul_ps(POLY2_AVX2(x, c1, c2, c3), x), _mm256_set1_ps(c0))
#define POLY4_AVX2(x, c0, c1, c2, c3, c4) \
    _mm256_add_ps(_mm256_mul_ps(POLY3_AVX2(x, c1, c2, c3, c4), x), _mm256_set1_ps(c0))
#define POLY5_AVX2(x, c0, c1, c2, c3, c4, c5) \
    _mm256_add_ps(_mm256_mul_ps(POLY4_AVX2(x, c1, c2, c3, c4, c5), x), _mm256_set1_ps(c0))

static inline void volk_32f_x2_pow_32f_u_avx2(float* cVector,
                                              const float* bVector,
                                              const float* aVector,
                                              unsigned int num_points)
{
    float* cPtr = cVector;
    const float* bPtr = bVector;
    const float* aPtr = aVector;

    unsigned int number = 0;
    const unsigned int eighthPoints = num_points / 8;

    __m256 aVal, bVal, cVal, logarithm, mantissa, frac, leadingOne;
    __m256 tmp, fx, mask, pow2n, z, y;
    __m256 one, exp_hi, exp_lo, ln2, log2EF, half, exp_C1, exp_C2;
    __m256 exp_p0, exp_p1, exp_p2, exp_p3, exp_p4, exp_p5;
    __m256i bias, exp, emm0, pi32_0x7f;

    one = _mm256_set1_ps(1.0);
    exp_hi = _mm256_set1_ps(88.3762626647949);
    exp_lo = _mm256_set1_ps(-88.3762626647949);
    ln2 = _mm256_set1_ps(0.6931471805);
    log2EF = _mm256_set1_ps(1.44269504088896341);
    half = _mm256_set1_ps(0.5);
    exp_C1 = _mm256_set1_ps(0.693359375);
    exp_C2 = _mm256_set1_ps(-2.12194440e-4);
    pi32_0x7f = _mm256_set1_epi32(0x7f);

    exp_p0 = _mm256_set1_ps(1.9875691500e-4);
    exp_p1 = _mm256_set1_ps(1.3981999507e-3);
    exp_p2 = _mm256_set1_ps(8.3334519073e-3);
    exp_p3 = _mm256_set1_ps(4.1665795894e-2);
    exp_p4 = _mm256_set1_ps(1.6666665459e-1);
    exp_p5 = _mm256_set1_ps(5.0000001201e-1);

    for (; number < eighthPoints; number++) {
        // First compute the logarithm
        aVal = _mm256_loadu_ps(aPtr);
        bias = _mm256_set1_epi32(127);
        leadingOne = _mm256_set1_ps(1.0f);
        exp = _mm256_sub_epi32(
            _mm256_srli_epi32(_mm256_and_si256(_mm256_castps_si256(aVal),
                                               _mm256_set1_epi32(0x7f800000)),
                              23),
            bias);
        logarithm = _mm256_cvtepi32_ps(exp);

        frac = _mm256_or_ps(
            leadingOne,
            _mm256_and_ps(aVal, _mm256_castsi256_ps(_mm256_set1_epi32(0x7fffff))));

#if POW_POLY_DEGREE == 6
        mantissa = POLY5_AVX2(frac,
                              3.1157899f,
                              -3.3241990f,
                              2.5988452f,
                              -1.2315303f,
                              3.1821337e-1f,
                              -3.4436006e-2f);
#elif POW_POLY_DEGREE == 5
        mantissa = POLY4_AVX2(frac,
                              2.8882704548164776201f,
                              -2.52074962577807006663f,
                              1.48116647521213171641f,
                              -0.465725644288844778798f,
                              0.0596515482674574969533f);
#elif POW_POLY_DEGREE == 4
        mantissa = POLY3_AVX2(frac,
                              2.61761038894603480148f,
                              -1.75647175389045657003f,
                              0.688243882994381274313f,
                              -0.107254423828329604454f);
#elif POW_POLY_DEGREE == 3
        mantissa = POLY2_AVX2(frac,
                              2.28330284476918490682f,
                              -1.04913055217340124191f,
                              0.204446009836232697516f);
#else
#error
#endif

        logarithm = _mm256_add_ps(
            _mm256_mul_ps(mantissa, _mm256_sub_ps(frac, leadingOne)), logarithm);
        logarithm = _mm256_mul_ps(logarithm, ln2);

        // Now calculate b*lna
        bVal = _mm256_loadu_ps(bPtr);
        bVal = _mm256_mul_ps(bVal, logarithm);

        // Now compute exp(b*lna)
        bVal = _mm256_max_ps(_mm256_min_ps(bVal, exp_hi), exp_lo);

        fx = _mm256_add_ps(_mm256_mul_ps(bVal, log2EF), half);

        emm0 = _mm256_cvttps_epi32(fx);
        tmp = _mm256_cvtepi32_ps(emm0);

        mask = _mm256_and_ps(_mm256_cmp_ps(tmp, fx, _CMP_GT_OS), one);
        fx = _mm256_sub_ps(tmp, mask);

        tmp = _mm256_sub_ps(bVal, _mm256_mul_ps(fx, exp_C1));
        bVal = _mm256_sub_ps(tmp, _mm256_mul_ps(fx, exp_C2));
        z = _mm256_mul_ps(bVal, bVal);

        y = _mm256_add_ps(_mm256_mul_ps(exp_p0, bVal), exp_p1);
        y = _mm256_add_ps(_mm256_mul_ps(y, bVal), exp_p2);
        y = _mm256_add_ps(_mm256_mul_ps(y, bVal), exp_p3);
        y = _mm256_add_ps(_mm256_mul_ps(y, bVal), exp_p4);
        y = _mm256_add_ps(_mm256_mul_ps(y, bVal), exp_p5);
        y = _mm256_add_ps(_mm256_mul_ps(y, z), bVal);
        y = _mm256_add_ps(y, one);

        emm0 =
            _mm256_slli_epi32(_mm256_add_epi32(_mm256_cvttps_epi32(fx), pi32_0x7f), 23);

        pow2n = _mm256_castsi256_ps(emm0);
        cVal = _mm256_mul_ps(y, pow2n);

        _mm256_storeu_ps(cPtr, cVal);

        aPtr += 8;
        bPtr += 8;
        cPtr += 8;
    }

    number = eighthPoints * 8;
    for (; number < num_points; number++) {
        *cPtr++ = pow(*aPtr++, *bPtr++);
    }
}

#endif /* LV_HAVE_AVX2 for unaligned */

#endif /* INCLUDED_volk_32f_x2_log2_32f_u_H */
