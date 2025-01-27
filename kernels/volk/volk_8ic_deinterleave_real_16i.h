/* -*- c++ -*- */
/*
 * Copyright 2012, 2014 Free Software Foundation, Inc.
 *
 * This file is part of VOLK
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*!
 * \page volk_8ic_deinterleave_real_16i
 *
 * \b Overview
 *
 * Deinterleaves the complex 8-bit char vector into just the I (real)
 * vector and converts it to 16-bit shorts.
 *
 * <b>Dispatcher Prototype</b>
 * \code
 * void volk_8ic_deinterleave_real_16i(int16_t* iBuffer, const lv_8sc_t* complexVector,
 * unsigned int num_points) \endcode
 *
 * \b Inputs
 * \li complexVector: The complex input vector.
 * \li num_points: The number of complex data values to be deinterleaved.
 *
 * \b Outputs
 * \li iBuffer: The I buffer output data.
 *
 * \b Example
 * \code
 * int N = 10000;
 *
 * volk_8ic_deinterleave_real_16i();
 *
 * volk_free(x);
 * \endcode
 */

#ifndef INCLUDED_volk_8ic_deinterleave_real_16i_a_H
#define INCLUDED_volk_8ic_deinterleave_real_16i_a_H

#include <inttypes.h>
#include <stdio.h>


#ifdef LV_HAVE_AVX2
#include <immintrin.h>

static inline void volk_8ic_deinterleave_real_16i_a_avx2(int16_t* iBuffer,
                                                         const lv_8sc_t* complexVector,
                                                         unsigned int num_points)
{
    unsigned int number = 0;
    const int8_t* complexVectorPtr = (int8_t*)complexVector;
    int16_t* iBufferPtr = iBuffer;
    __m256i moveMask = _mm256_set_epi8(0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       14,
                                       12,
                                       10,
                                       8,
                                       6,
                                       4,
                                       2,
                                       0,
                                       0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       14,
                                       12,
                                       10,
                                       8,
                                       6,
                                       4,
                                       2,
                                       0);
    __m256i complexVal, outputVal;
    __m128i outputVal0;

    unsigned int sixteenthPoints = num_points / 16;

    for (number = 0; number < sixteenthPoints; number++) {
        complexVal = _mm256_load_si256((__m256i*)complexVectorPtr);
        complexVectorPtr += 32;

        complexVal = _mm256_shuffle_epi8(complexVal, moveMask);
        complexVal = _mm256_permute4x64_epi64(complexVal, 0xd8);

        outputVal0 = _mm256_extractf128_si256(complexVal, 0);

        outputVal = _mm256_cvtepi8_epi16(outputVal0);
        outputVal = _mm256_slli_epi16(outputVal, 7);

        _mm256_store_si256((__m256i*)iBufferPtr, outputVal);

        iBufferPtr += 16;
    }

    number = sixteenthPoints * 16;
    for (; number < num_points; number++) {
        *iBufferPtr++ = ((int16_t)*complexVectorPtr++) * 128;
        complexVectorPtr++;
    }
}
#endif /* LV_HAVE_AVX2 */

#ifdef LV_HAVE_SSE4_1
#include <smmintrin.h>

static inline void volk_8ic_deinterleave_real_16i_a_sse4_1(int16_t* iBuffer,
                                                           const lv_8sc_t* complexVector,
                                                           unsigned int num_points)
{
    unsigned int number = 0;
    const int8_t* complexVectorPtr = (int8_t*)complexVector;
    int16_t* iBufferPtr = iBuffer;
    __m128i moveMask = _mm_set_epi8(
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 14, 12, 10, 8, 6, 4, 2, 0);
    __m128i complexVal, outputVal;

    unsigned int eighthPoints = num_points / 8;

    for (number = 0; number < eighthPoints; number++) {
        complexVal = _mm_load_si128((__m128i*)complexVectorPtr);
        complexVectorPtr += 16;

        complexVal = _mm_shuffle_epi8(complexVal, moveMask);

        outputVal = _mm_cvtepi8_epi16(complexVal);
        outputVal = _mm_slli_epi16(outputVal, 7);

        _mm_store_si128((__m128i*)iBufferPtr, outputVal);
        iBufferPtr += 8;
    }

    number = eighthPoints * 8;
    for (; number < num_points; number++) {
        *iBufferPtr++ = ((int16_t)*complexVectorPtr++) * 128;
        complexVectorPtr++;
    }
}
#endif /* LV_HAVE_SSE4_1 */


#ifdef LV_HAVE_AVX
#include <immintrin.h>

static inline void volk_8ic_deinterleave_real_16i_a_avx(int16_t* iBuffer,
                                                        const lv_8sc_t* complexVector,
                                                        unsigned int num_points)
{
    unsigned int number = 0;
    const int8_t* complexVectorPtr = (int8_t*)complexVector;
    int16_t* iBufferPtr = iBuffer;
    __m128i moveMask = _mm_set_epi8(
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 14, 12, 10, 8, 6, 4, 2, 0);
    __m256i complexVal, outputVal;
    __m128i complexVal1, complexVal0, outputVal1, outputVal0;

    unsigned int sixteenthPoints = num_points / 16;

    for (number = 0; number < sixteenthPoints; number++) {
        complexVal = _mm256_load_si256((__m256i*)complexVectorPtr);
        complexVectorPtr += 32;

        complexVal1 = _mm256_extractf128_si256(complexVal, 1);
        complexVal0 = _mm256_extractf128_si256(complexVal, 0);

        outputVal1 = _mm_shuffle_epi8(complexVal1, moveMask);
        outputVal0 = _mm_shuffle_epi8(complexVal0, moveMask);

        outputVal1 = _mm_cvtepi8_epi16(outputVal1);
        outputVal1 = _mm_slli_epi16(outputVal1, 7);
        outputVal0 = _mm_cvtepi8_epi16(outputVal0);
        outputVal0 = _mm_slli_epi16(outputVal0, 7);

        __m256i dummy = _mm256_setzero_si256();
        outputVal = _mm256_insertf128_si256(dummy, outputVal0, 0);
        outputVal = _mm256_insertf128_si256(outputVal, outputVal1, 1);
        _mm256_store_si256((__m256i*)iBufferPtr, outputVal);

        iBufferPtr += 16;
    }

    number = sixteenthPoints * 16;
    for (; number < num_points; number++) {
        *iBufferPtr++ = ((int16_t)*complexVectorPtr++) * 128;
        complexVectorPtr++;
    }
}
#endif /* LV_HAVE_AVX */


#ifdef LV_HAVE_GENERIC

static inline void volk_8ic_deinterleave_real_16i_generic(int16_t* iBuffer,
                                                          const lv_8sc_t* complexVector,
                                                          unsigned int num_points)
{
    unsigned int number = 0;
    const int8_t* complexVectorPtr = (const int8_t*)complexVector;
    int16_t* iBufferPtr = iBuffer;
    for (number = 0; number < num_points; number++) {
        *iBufferPtr++ = ((int16_t)(*complexVectorPtr++)) * 128;
        complexVectorPtr++;
    }
}
#endif /* LV_HAVE_GENERIC */


#endif /* INCLUDED_volk_8ic_deinterleave_real_16i_a_H */

#ifndef INCLUDED_volk_8ic_deinterleave_real_16i_u_H
#define INCLUDED_volk_8ic_deinterleave_real_16i_u_H

#include <inttypes.h>
#include <stdio.h>


#ifdef LV_HAVE_AVX2
#include <immintrin.h>

static inline void volk_8ic_deinterleave_real_16i_u_avx2(int16_t* iBuffer,
                                                         const lv_8sc_t* complexVector,
                                                         unsigned int num_points)
{
    unsigned int number = 0;
    const int8_t* complexVectorPtr = (int8_t*)complexVector;
    int16_t* iBufferPtr = iBuffer;
    __m256i moveMask = _mm256_set_epi8(0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       14,
                                       12,
                                       10,
                                       8,
                                       6,
                                       4,
                                       2,
                                       0,
                                       0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       0x80,
                                       14,
                                       12,
                                       10,
                                       8,
                                       6,
                                       4,
                                       2,
                                       0);
    __m256i complexVal, outputVal;
    __m128i outputVal0;

    unsigned int sixteenthPoints = num_points / 16;

    for (number = 0; number < sixteenthPoints; number++) {
        complexVal = _mm256_loadu_si256((__m256i*)complexVectorPtr);
        complexVectorPtr += 32;

        complexVal = _mm256_shuffle_epi8(complexVal, moveMask);
        complexVal = _mm256_permute4x64_epi64(complexVal, 0xd8);

        outputVal0 = _mm256_extractf128_si256(complexVal, 0);

        outputVal = _mm256_cvtepi8_epi16(outputVal0);
        outputVal = _mm256_slli_epi16(outputVal, 7);

        _mm256_storeu_si256((__m256i*)iBufferPtr, outputVal);

        iBufferPtr += 16;
    }

    number = sixteenthPoints * 16;
    for (; number < num_points; number++) {
        *iBufferPtr++ = ((int16_t)*complexVectorPtr++) * 128;
        complexVectorPtr++;
    }
}
#endif /* LV_HAVE_AVX2 */
#endif /* INCLUDED_volk_8ic_deinterleave_real_16i_u_H */
