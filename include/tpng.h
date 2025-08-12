/**************************************************************************
 *
 * tPNG:
 * tPNG is part of the topaz project. (https://github.com/jcorks/topaz)
 * 2021, Johnathan Corkery
 *
 *
 * TINFL:
 * Copyright 2013-2014 RAD Game Tools and Valve Software
 * Copyright 2010-2014 Rich Geldreich and Tenacious Software LLC
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


#ifndef TPNG_H_INCLUDED
#define TPNG_H_INCLUDED

#include <stdint.h>

// Returns a raw data buffer containing 
// 32-bit RGBA data buffer. Must be freed.
uint8_t * tpng_get_rgba(

    // The raw data to interpret.
    // This should be the entire data buffer of 
    // a valid PNG file.
    const uint8_t * rawData,

    // The number of bytes of the rawData.
    uint32_t        rawSize,


    // On success, outputs the width of the 
    // image.
    uint32_t * w, 

    // On success, outputs the height of the 
    // image.
    uint32_t * h 
);


#endif


#ifdef __cplusplus
}
#endif
