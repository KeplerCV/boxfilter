/******************************************************************************
* boxfilter Algorithm
* Author: Keplore
* Copyright 2018-2028 Keplore, Inc. All Rights Reserved.
* Date: 2018-01-14
******************************************************************************/
#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <minmax.h>
#include <assert.h>

/* the alignment of all the allocated buffers */
#define  KEPLORE_MALLOC_ALIGN    64

//http://blog.csdn.net/tcx19900712/article/details/18218225
//align memory block reference linking
void* aligned_malloc(size_t size, int alignment)
{
	const int pointerSize = sizeof(void*);
	const int requestedSize = size + alignment - 1 + pointerSize;
	void* raw = malloc(requestedSize);
	uintptr_t start = (uintptr_t)raw + pointerSize;
	void* aligned = (void*)((start + alignment - 1) & ~(alignment - 1));
	*(void**)((uintptr_t)aligned - pointerSize) = raw;
	return aligned;
}

void aligned_free(void* aligned)
{
	void* raw = *(void**)((uintptr_t)aligned - sizeof(void*));
	free(raw);
	raw = NULL;
}

int isAligned(void* data, int alignment)
{
	return ((uintptr_t)data & (alignment - 1)) == 0;
}

void boxfilter_x(float *id, float *od, int w, int h, int r)
{
	float x = 0.0f;
	float y = 0.0f;
	float t = 0.0f;
	float scale = 0.0f;
	scale = 1.0f / (2 * r + 1);

	for (int y = 0; y < h; y++)
	{

		float t;
		// do left edge
		t = id[y*w] * r;

		for (int x = 0; x < r + 1; x++)
		{
			t += id[y*w + x];
		}

		od[y*w] = t * scale;

		for (int x = 1; x < r + 1; x++)
		{
			int c = y*w + x;
			t += id[c + r];
			t -= id[y*w];
			od[c] = t * scale;
		}

		// main loop
		for (int x = r + 1; x < w - r; x++)
		{
			int c = y*w + x;
			t += id[c + r];
			t -= id[c - r - 1];
			od[c] = t * scale;
		}

		// do right edge
		for (int x = w - r; x < w; x++)
		{
			int c = y*w + x;
			t += id[(y*w) + w - 1];
			t -= id[c - r - 1];
			od[c] = t * scale;
		}

	}
}

void boxfilter_y(float *id, float *od, int w, int h, int r)
{
	float scale = 1.0f / (2 * r + 1);

	for (int x = 0; x < w; x++)
	{

		float t;
		// do left edge
		t = id[x] * r;

		for (int y = 0; y < r + 1; y++)
		{
			t += id[y*w + x];
		}

		od[x] = t * scale;

		for (int y = 1; y < r + 1; y++)
		{
			int c = y*w + x;
			t += id[c + r*w];
			t -= id[x];
			od[c] = t * scale;
		}

		// main loop
		for (int y = r + 1; y < h - r; y++)
		{
			int c = y*w + x;
			t += id[c + r*w];
			t -= id[c - (r*w) - w];
			od[c] = t * scale;
		}

		// do right edge
		for (int y = h - r; y < h; y++)
		{
			int c = y*w + x;
			t += id[(h - 1)*w + x];
			t -= id[c - (r*w) - w];
			od[c] = t * scale;
		}

	}
}

////////////////////////////////////////////////////////////////////////////////
//boxFilter
//param image      pointer to input data
//param temp       pointer to temporary store
//param w          width of image
//param h          height of image
//param r          radius of filter
////////////////////////////////////////////////////////////////////////////////
void boxfilter(float *image, float *temp, float *dst, int w, int h, int r)
{
	boxfilter_x(image, temp, w, h, r);
	boxfilter_y(temp, dst, w, h, r);
}

int main()
{
	int w = 5;
	int h = 5;
	int r = 1;
	int i = 0;
	int j = 0;
	float src_data[5][5] = { { 17, 24, 1, 8, 15 },
	                         { 23, 5, 7, 14, 16 },
	                         { 4, 6, 13, 20, 22 },
	                         { 10, 12, 19, 21, 3 },
	                         { 11, 18, 25, 2, 9 } };
	float temp[5][5] = { 0 };
	float *dst = (float *)aligned_malloc(w * h * sizeof(float), KEPLORE_MALLOC_ALIGN);
	boxfilter(src_data, temp, dst, w, h, r); //BORDER_REFLECT !< `fedcba|abcdefgh|hgfedcb`

	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			printf("%lf ", dst[i * w + j]);
		}
		printf("\n");
	}

	aligned_free(dst);
	return 0;
}
