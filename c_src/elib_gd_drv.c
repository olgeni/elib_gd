/* -*- c-file-style: "bsd"; indent-tabs-mode: t; -*- */

/* $Id: elib_gd_drv.c 30386 2012-09-02 12:00:27Z olgeni $ */

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#include "ei.h"
#include "ei_tools.h"
#include "erl_driver.h"

#include <gd.h>
#include <ft2build.h>

#define TERM_VECTOR_ALLOC(spec, count)                                  \
	ErlDrvTermData *spec;						\
	ErlDrvTermData *spec##_mark;					\
	ErlDrvTermData *spec##_current;					\
	spec = (ErlDrvTermData *) driver_alloc (sizeof (ErlDrvTermData) * (count)); \
	spec##_mark = spec + (count);					\
	spec##_current = spec;

#define TERM_VECTOR_FREE(spec)                  \
	driver_free (spec);

#define TERM_VECTOR_OUTPUT(spec, port)					\
	driver_output_term ((port), spec, (spec##_current - spec));

#define TERM_VECTOR_PUSH(spec, value)                                   \
	if (spec##_current >= spec##_mark)				\
	{								\
		int spec##_count = (spec##_mark - spec) * 2;		\
		int spec##_offset = (spec##_current - spec);	\
		spec = (ErlDrvTermData *) driver_realloc (spec, sizeof (ErlDrvTermData) * spec##_count); \
		spec##_mark = spec + spec##_count;			\
		spec##_current = spec + spec##_offset; 			\
	}								\
	*spec##_current++ = (value);

#define RLE_ENCODE_PROTOTYPE(function, type)				\
	void function (type *in_array, uint32_t in_length,		\
		       uint32_t **out_array, uint32_t *out_length);

#define RLE_ENCODE_GENERATE(function, type)				\
	void function (type *in_array, uint32_t in_length,		\
		       uint32_t **out_array, uint32_t *out_length)	\
	{								\
		*out_array = calloc (2 * in_length, sizeof (uint32_t));	\
									\
		type *inptr = in_array;					\
		uint32_t *outptr = *out_array;				\
		type *mark = in_array + in_length;			\
									\
		while (inptr < mark)					\
		{							\
			*outptr = *inptr++;				\
			*(outptr + 1) = 1;				\
									\
			while (inptr < mark && (*inptr == *outptr))	\
			{						\
				inptr++;				\
				(*(outptr + 1))++;			\
			}						\
									\
			outptr += 2;					\
		}							\
									\
		*out_length = (outptr - *out_array);			\
	}

RLE_ENCODE_PROTOTYPE (rle_encode_u8, uint8_t)
RLE_ENCODE_PROTOTYPE (rle_encode_u16, uint16_t)
RLE_ENCODE_PROTOTYPE (rle_encode_u32, uint32_t)

RLE_ENCODE_GENERATE (rle_encode_u8, uint8_t)
RLE_ENCODE_GENERATE (rle_encode_u16, uint16_t)
RLE_ENCODE_GENERATE (rle_encode_u32, uint32_t)

#include FT_FREETYPE_H

#define IMAGE_COUNT      16
#define IMAGE_NULL_INDEX -1

#define ATOM_GD_BOUNDS "gd_bounds"
#define ATOM_GD_CLIP   "gd_clip"
#define ATOM_GD_FACE   "gd_face"

#define GD_IMAGE_CREATE                         0x00
#define GD_IMAGE_CREATE_TRUE_COLOR              0x01
#define GD_IMAGE_DESTROY                        0x02
#define GD_IMAGE_SET_PIXEL                      0x03
#define GD_IMAGE_GET_PIXEL                      0x04
#define GD_IMAGE_COLOR_ALLOCATE                 0x05
#define GD_IMAGE_COLOR_ALLOCATE_ALPHA           0x06
#define GD_IMAGE_COLOR_CLOSEST                  0x07
#define GD_IMAGE_COLOR_CLOSEST_ALPHA            0x08
#define GD_IMAGE_COLOR_CLOSEST_HWB              0x09
#define GD_IMAGE_COLOR_EXACT                    0x0A
#define GD_IMAGE_COLOR_EXACT_ALPHA              0x0B
#define GD_IMAGE_COLOR_RESOLVE                  0x0C
#define GD_IMAGE_COLOR_RESOLVE_ALPHA            0x0D
#define GD_IMAGE_COLOR_DEALLOCATE               0x0E
#define GD_IMAGE_CREATE_FROM_PNG_PTR            0x0F
#define GD_IMAGE_CREATE_FROM_JPEG_PTR           0x10
#define GD_IMAGE_CREATE_FROM_GIF_PTR            0x11
#define GD_IMAGE_PNG_PTR                        0x12
#define GD_IMAGE_PNG_PTR_EX                     0x13
#define GD_IMAGE_JPEG_PTR                       0x14
#define GD_IMAGE_GIF_PTR                        0x15
#define GD_IMAGE_SET_CLIP                       0x16
#define GD_IMAGE_GET_CLIP                       0x17
#define GD_IMAGE_BOUNDS_SAFE                    0x18
#define GD_IMAGE_LINE                           0x19
#define GD_IMAGE_RECTANGLE                      0x1A
#define GD_IMAGE_FILLED_RECTANGLE               0x1B
#define GD_IMAGE_ELLIPSE                        0x1C
#define GD_IMAGE_FILLED_ELLIPSE                 0x1D
#define GD_IMAGE_ARC                            0x1E
#define GD_IMAGE_FILLED_ARC                     0x1F
#define GD_IMAGE_POLYGON                        0x20
#define GD_IMAGE_OPEN_POLYGON                   0x21
#define GD_IMAGE_FILLED_POLYGON                 0x22
#define GD_IMAGE_FILL                           0x23
#define GD_IMAGE_FILL_TO_BORDER                 0x24
#define GD_IMAGE_STRING_FT                      0x25
#define GD_IMAGE_COPY                           0x26
#define GD_IMAGE_COPY_MERGE                     0x27
#define GD_IMAGE_COPY_MERGE_GRAY                0x28
#define GD_IMAGE_COPY_RESIZED                   0x29
#define GD_IMAGE_COPY_RESAMPLED                 0x2A
#define GD_IMAGE_COPY_ROTATED                   0x2B
#define GD_IMAGE_SET_BRUSH                      0x2C
#define GD_IMAGE_SET_TILE                       0x2D
#define GD_IMAGE_SET_ANTI_ALIASED               0x2E
#define GD_IMAGE_SET_ANTI_ALIASED_DONT_BLEND    0x2F
#define GD_IMAGE_SET_STYLE                      0x30
#define GD_IMAGE_SET_THICKNESS                  0x31
#define GD_IMAGE_INTERLACE                      0x32
#define GD_IMAGE_ALPHA_BLENDING                 0x33
#define GD_IMAGE_SAVE_ALPHA                     0x34
#define GD_IMAGE_CREATE_PALETTE_FROM_TRUE_COLOR 0x35
#define GD_IMAGE_TRUE_COLOR_TO_PALETTE          0x36
#define GD_IMAGE_COLOR_TRANSPARENT              0x37
#define GD_IMAGE_PALETTE_COPY                   0x38
#define GD_IMAGE_TRUECOLOR                      0x39
#define GD_IMAGE_SX                             0x3A
#define GD_IMAGE_SY                             0x3B
#define GD_IMAGE_COLORS_TOTAL                   0x3C
#define GD_IMAGE_RED                            0x3D
#define GD_IMAGE_GREEN                          0x3E
#define GD_IMAGE_BLUE                           0x3F
#define GD_IMAGE_ALPHA                          0x40
#define GD_IMAGE_GET_TRANSPARENT                0x41
#define GD_IMAGE_GET_INTERLACED                 0x42
#define GD_IMAGE_GET_PALETTE                    0x43
#define GD_IMAGE_GET_ROW_INDEXED                0x44
#define GD_IMAGE_GET_ROW_INDEXED_RLE            0x45
#define GD_IMAGE_GET_ROW_TRUECOLOR              0x46
#define GD_IMAGE_GET_INDEXED_RLE                0x47
#define GD_GET_FACE                             0x48

#define EI_DECODE_IMAGE_INDEX(state, buffer, termIndex, imageIndex)	\
	long imageIndex;						\
	EI_DECODE_LONG (state->port, buffer, termIndex, imageIndex);	\
	if (!assert_index (state, imageIndex))				\
	{								\
		driver_failure_atom (state->port, "image_index_error");	\
		return;							\
	}

struct DriverState
{
	ErlDrvPort port;
	gdImagePtr *image;
};

static FT_Library freeTypeLibrary = NULL;

static int allocate_index (struct DriverState *state)
{
	for (int i = 0; i < IMAGE_COUNT; i++)
		if (state->image[i] == NULL)
			return i;

	return -1;
}

static int is_index (struct DriverState *state, int imageIndex)
{
	if (imageIndex >= IMAGE_COUNT)
		return 0;

	return state->image[imageIndex] != NULL;
}

static int assert_index (struct DriverState *state, int imageIndex)
{
	if (is_index (state, imageIndex))
		return 1;

	driver_failure_atom (state->port, "assert_index");

	return 0;
}

static int init (void)
{
	if (FT_Init_FreeType (&freeTypeLibrary))
		return -1;

	gdFontCacheSetup ();

	return 0;
}

static void finish (void)
{
	if (freeTypeLibrary != NULL)
		FT_Done_FreeType (freeTypeLibrary);

	gdFontCacheShutdown ();
}

static ErlDrvData start (ErlDrvPort port, char *command)
{
	struct DriverState *state = (struct DriverState *) driver_alloc (sizeof (struct DriverState));

	if (state == NULL)
		return ERL_DRV_ERROR_GENERAL;

	state->port = port;
	state->image = (gdImagePtr *) driver_alloc (IMAGE_COUNT * sizeof (gdImagePtr));

	memset (state->image, 0, IMAGE_COUNT * sizeof (gdImagePtr));

	return (ErlDrvData) state;
}

static void stop (ErlDrvData handle)
{
	struct DriverState *state = (struct DriverState *) handle;

	for (int i = 0; i < IMAGE_COUNT; i++)
		if (is_index (state, i))
			gdImageDestroy (state->image[i]);

	driver_free (state->image);
	driver_free (state);
}

static void output (ErlDrvData handle, char *buffer, ErlDrvSizeT bufferLength)
{
	struct DriverState *state = (struct DriverState *) handle;

	int termIndex = 0;

	int version;

	EI_DECODE_VERSION (state->port, buffer, termIndex, version);

	int tupleArity;

	EI_DECODE_TUPLE_HEADER (state->port, buffer, termIndex, tupleArity);

	long command;

	EI_DECODE_LONG (state->port, buffer, termIndex, command);

	switch (command)
	{
	case GD_IMAGE_CREATE:
	{
		long width, height;

		EI_DECODE_LONG (state->port, buffer, termIndex, width);
		EI_DECODE_LONG (state->port, buffer, termIndex, height);

		int imageIndex = allocate_index (state);

		if (imageIndex == -1)
		{
			driver_failure_atom (state->port, "too_many_images");
			return;
		}

		gdImagePtr image = gdImageCreate (width, height);

		if (image == NULL)
		{
			driver_failure_atom (state->port, ATOM_BADARG);
			return;
		}

		state->image[imageIndex] = image;

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, imageIndex,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_CREATE_TRUE_COLOR:
	{
		long width, height;

		EI_DECODE_LONG (state->port, buffer, termIndex, width);
		EI_DECODE_LONG (state->port, buffer, termIndex, height);

		int imageIndex = allocate_index (state);

		if (imageIndex == -1)
		{
			driver_failure_atom (state->port, "too_many_images");
			return;
		}

		gdImagePtr image = gdImageCreateTrueColor (width, height);

		if (image == NULL)
		{
			driver_failure_atom (state->port, ATOM_BADARG);
			return;
		}

		state->image[imageIndex] = image;

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, imageIndex,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_DESTROY:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		if (state->image[imageIndex] == NULL)
		{
			driver_failure_atom (state->port, ATOM_BADARG);

			return;
		}

		gdImageDestroy (state->image[imageIndex]);
		state->image[imageIndex] = NULL;

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_SET_PIXEL:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long x, y, color;

		EI_DECODE_LONG (state->port, buffer, termIndex, x);
		EI_DECODE_LONG (state->port, buffer, termIndex, y);
		EI_DECODE_LONG (state->port, buffer, termIndex, color);

		gdImageSetPixel (state->image[imageIndex], x, y, color);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_GET_PIXEL:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long x, y;

		EI_DECODE_LONG (state->port, buffer, termIndex, x);
		EI_DECODE_LONG (state->port, buffer, termIndex, y);

		int color = gdImageGetPixel (state->image[imageIndex], x, y);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, color,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_COLOR_ALLOCATE:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long red, green, blue;

		EI_DECODE_LONG (state->port, buffer, termIndex, red);
		EI_DECODE_LONG (state->port, buffer, termIndex, green);
		EI_DECODE_LONG (state->port, buffer, termIndex, blue);

		int color = gdImageColorAllocate (state->image[imageIndex], red, green, blue);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, color,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_COLOR_ALLOCATE_ALPHA:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long red, green, blue, alpha;

		EI_DECODE_LONG (state->port, buffer, termIndex, red);
		EI_DECODE_LONG (state->port, buffer, termIndex, green);
		EI_DECODE_LONG (state->port, buffer, termIndex, blue);
		EI_DECODE_LONG (state->port, buffer, termIndex, alpha);

		int color = gdImageColorAllocateAlpha (state->image[imageIndex], red, green, blue, alpha);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, color,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_COLOR_CLOSEST:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long red, green, blue;

		EI_DECODE_LONG (state->port, buffer, termIndex, red);
		EI_DECODE_LONG (state->port, buffer, termIndex, green);
		EI_DECODE_LONG (state->port, buffer, termIndex, blue);

		int color = gdImageColorClosest (state->image[imageIndex], red, green, blue);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, color,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_COLOR_CLOSEST_ALPHA:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long red, green, blue, alpha;

		EI_DECODE_LONG (state->port, buffer, termIndex, red);
		EI_DECODE_LONG (state->port, buffer, termIndex, green);
		EI_DECODE_LONG (state->port, buffer, termIndex, blue);
		EI_DECODE_LONG (state->port, buffer, termIndex, alpha);

		int color = gdImageColorClosestAlpha (state->image[imageIndex], red, green, blue, alpha);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, color,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_COLOR_CLOSEST_HWB:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long red, green, blue;

		EI_DECODE_LONG (state->port, buffer, termIndex, red);
		EI_DECODE_LONG (state->port, buffer, termIndex, green);
		EI_DECODE_LONG (state->port, buffer, termIndex, blue);

		int color = gdImageColorClosestHWB (state->image[imageIndex], red, green, blue);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, color,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_COLOR_EXACT:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long red, green, blue;

		EI_DECODE_LONG (state->port, buffer, termIndex, red);
		EI_DECODE_LONG (state->port, buffer, termIndex, green);
		EI_DECODE_LONG (state->port, buffer, termIndex, blue);

		int color = gdImageColorExact (state->image[imageIndex], red, green, blue);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, color,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_COLOR_EXACT_ALPHA:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long red, green, blue, alpha;

		EI_DECODE_LONG (state->port, buffer, termIndex, red);
		EI_DECODE_LONG (state->port, buffer, termIndex, green);
		EI_DECODE_LONG (state->port, buffer, termIndex, blue);
		EI_DECODE_LONG (state->port, buffer, termIndex, alpha);

		int color = gdImageColorExactAlpha (state->image[imageIndex], red, green, blue, alpha);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, color,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_COLOR_RESOLVE:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long red, green, blue;

		EI_DECODE_LONG (state->port, buffer, termIndex, red);
		EI_DECODE_LONG (state->port, buffer, termIndex, green);
		EI_DECODE_LONG (state->port, buffer, termIndex, blue);

		int color = gdImageColorResolve (state->image[imageIndex], red, green, blue);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, color,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_COLOR_RESOLVE_ALPHA:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long red, green, blue, alpha;

		EI_DECODE_LONG (state->port, buffer, termIndex, red);
		EI_DECODE_LONG (state->port, buffer, termIndex, green);
		EI_DECODE_LONG (state->port, buffer, termIndex, blue);
		EI_DECODE_LONG (state->port, buffer, termIndex, alpha);

		int color = gdImageColorResolveAlpha (state->image[imageIndex], red, green, blue, alpha);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, color,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_COLOR_DEALLOCATE:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long color;

		EI_DECODE_LONG (state->port, buffer, termIndex, color);

		gdImageColorDeallocate (state->image[imageIndex], color);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_CREATE_FROM_PNG_PTR:
	{
		EI_GET_TYPE (state->port, buffer, termIndex, valueType, valueSize);

		void *binary = (void *) driver_alloc (valueSize);

		ASSERT_DRIVER_ALLOC (state->port, binary, GD_IMAGE_CREATE_FROM_PNG_PTR_failure0);

		long binarySize;

		EI_DECODE_BINARY (state->port, buffer, termIndex, binary, binarySize);

		int imageIndex = allocate_index (state);

		if (imageIndex == -1)
		{
			driver_failure_atom (state->port, "too_many_images");
			goto GD_IMAGE_CREATE_FROM_PNG_PTR_failure1;
		}

		gdImagePtr image = gdImageCreateFromPngPtr (binarySize, binary);

		if (image == NULL)
		{
			driver_failure_atom (state->port, ATOM_BADARG);
			goto GD_IMAGE_CREATE_FROM_PNG_PTR_failure1;
		}

		state->image[imageIndex] = image;

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, imageIndex,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

	GD_IMAGE_CREATE_FROM_PNG_PTR_failure1:
		driver_free (binary);

	GD_IMAGE_CREATE_FROM_PNG_PTR_failure0:
		return;
	}

	case GD_IMAGE_CREATE_FROM_JPEG_PTR:
	{
		EI_GET_TYPE (state->port, buffer, termIndex, valueType, valueSize);

		void *binary = (void *) driver_alloc (valueSize);

		ASSERT_DRIVER_ALLOC (state->port, binary, GD_IMAGE_CREATE_FROM_JPEG_PTR_failure0);

		long binarySize;

		EI_DECODE_BINARY (state->port, buffer, termIndex, binary, binarySize);

		int imageIndex = allocate_index (state);

		if (imageIndex == -1)
		{
			driver_failure_atom (state->port, "too_many_images");
			goto GD_IMAGE_CREATE_FROM_JPEG_PTR_failure1;
		}

		gdImagePtr image = gdImageCreateFromJpegPtr (binarySize, binary);

		if (image == NULL)
		{
			driver_failure_atom (state->port, ATOM_BADARG);
			goto GD_IMAGE_CREATE_FROM_JPEG_PTR_failure1;
		}

		state->image[imageIndex] = image;

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, imageIndex,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

	GD_IMAGE_CREATE_FROM_JPEG_PTR_failure1:
		driver_free (binary);

	GD_IMAGE_CREATE_FROM_JPEG_PTR_failure0:
		return;
	}

	case GD_IMAGE_CREATE_FROM_GIF_PTR:
	{
		EI_GET_TYPE (state->port, buffer, termIndex, valueType, valueSize);

		void *binary = (void *) driver_alloc (valueSize);

		ASSERT_DRIVER_ALLOC (state->port, binary, GD_IMAGE_CREATE_FROM_GIF_PTR_failure0);

		long binarySize;

		EI_DECODE_BINARY (state->port, buffer, termIndex, binary, binarySize);

		int imageIndex = allocate_index (state);

		if (imageIndex == -1)
		{
			driver_failure_atom (state->port, "too_many_images");
			goto GD_IMAGE_CREATE_FROM_GIF_PTR_failure1;
		}

		gdImagePtr image = gdImageCreateFromGifPtr (binarySize, binary);

		if (image == NULL)
		{
			driver_failure_atom (state->port, ATOM_BADARG);
			goto GD_IMAGE_CREATE_FROM_GIF_PTR_failure1;
		}

		state->image[imageIndex] = image;

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, imageIndex,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

	GD_IMAGE_CREATE_FROM_GIF_PTR_failure1:
		driver_free (binary);

	GD_IMAGE_CREATE_FROM_GIF_PTR_failure0:
		return;
	}

	case GD_IMAGE_PNG_PTR:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		int size;

		void *image = gdImagePngPtr (state->image[imageIndex], &size);

		if (image == NULL)
		{
			driver_failure_atom (state->port, "gdImagePngPtr");
			goto GD_IMAGE_PNG_PTR_failure0;
		}

		ErlDrvBinary *result = driver_alloc_binary (size);

		ASSERT_DRIVER_ALLOC_BINARY (state->port, result, GD_IMAGE_PNG_PTR_failure1);

		memcpy (result->orig_bytes, image, size);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_BINARY, (ErlDrvTermData) result, size, 0,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2};

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		driver_free_binary (result);

	GD_IMAGE_PNG_PTR_failure1:
		gdFree (image);

	GD_IMAGE_PNG_PTR_failure0:
		return;
	}

	case GD_IMAGE_PNG_PTR_EX:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long level;

		EI_DECODE_LONG (state->port, buffer, termIndex, level);

		int size;

		void *image = gdImagePngPtrEx (state->image[imageIndex], &size, level);

		if (image == NULL)
		{
			driver_failure_atom (state->port, "gdImagePngPtrEx");
			goto GD_IMAGE_PNG_PTR_EX_failure0;
		}

		ErlDrvBinary *result = driver_alloc_binary (size);

		ASSERT_DRIVER_ALLOC_BINARY (state->port, result, GD_IMAGE_PNG_PTR_EX_failure1);

		memcpy (result->orig_bytes, image, size);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_BINARY, (ErlDrvTermData) result, size, 0,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2};

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		driver_free_binary (result);

	GD_IMAGE_PNG_PTR_EX_failure1:
		gdFree (image);

	GD_IMAGE_PNG_PTR_EX_failure0:
		return;
	}

	case GD_IMAGE_JPEG_PTR:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long quality;

		EI_DECODE_LONG (state->port, buffer, termIndex, quality);

		int size;

		void *image = gdImageJpegPtr (state->image[imageIndex], &size, quality);

		if (image == NULL)
		{
			driver_failure_atom (state->port, "gdImageJpegPtr");
			goto GD_IMAGE_JPEG_PTR_failure0;
		}

		ErlDrvBinary *result = driver_alloc_binary (size);

		ASSERT_DRIVER_ALLOC_BINARY (state->port, result, GD_IMAGE_JPEG_PTR_failure1);

		memcpy (result->orig_bytes, image, size);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_BINARY, (ErlDrvTermData) result, size, 0,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2};

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		driver_free_binary (result);

	GD_IMAGE_JPEG_PTR_failure1:
		gdFree (image);

	GD_IMAGE_JPEG_PTR_failure0:
		return;
	}

	case GD_IMAGE_GIF_PTR:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		int size;

		void *image = gdImageGifPtr (state->image[imageIndex], &size);

		if (image == NULL)
		{
			driver_failure_atom (state->port, "gdImageGifPtr");
			goto GD_IMAGE_GIF_PTR_failure0;
		}

		ErlDrvBinary *result = driver_alloc_binary (size);

		ASSERT_DRIVER_ALLOC_BINARY (state->port, result, GD_IMAGE_GIF_PTR_failure1);

		memcpy (result->orig_bytes, image, size);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_BINARY, (ErlDrvTermData) result, size, 0,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2};

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		driver_free_binary (result);

	GD_IMAGE_GIF_PTR_failure1:
		gdFree (image);

	GD_IMAGE_GIF_PTR_failure0:
		return;
	}

	case GD_IMAGE_SET_CLIP:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long x1, y1, x2, y2;

		EI_DECODE_LONG (state->port, buffer, termIndex, x1);
		EI_DECODE_LONG (state->port, buffer, termIndex, y1);
		EI_DECODE_LONG (state->port, buffer, termIndex, x2);
		EI_DECODE_LONG (state->port, buffer, termIndex, y2);

		gdImageSetClip (state->image[imageIndex], x1, y1, x2, y2);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_GET_CLIP:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		int x1, y1, x2, y2;

		gdImageGetClip (state->image[imageIndex], &x1, &y1, &x2, &y2);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_GD_CLIP),
			ERL_DRV_INT, x1, ERL_DRV_INT, y1,
			ERL_DRV_INT, x2, ERL_DRV_INT, y2,
			ERL_DRV_TUPLE, 5,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_BOUNDS_SAFE:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long x, y;

		EI_DECODE_LONG (state->port, buffer, termIndex, x);
		EI_DECODE_LONG (state->port, buffer, termIndex, y);

		int boundsSafe = gdImageBoundsSafe (state->image[imageIndex], x, y);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_ATOM, driver_mk_atom (boundsSafe ? ATOM_TRUE : ATOM_FALSE),
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_LINE:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long x1, y1, x2, y2, color;

		EI_DECODE_LONG (state->port, buffer, termIndex, x1);
		EI_DECODE_LONG (state->port, buffer, termIndex, y1);
		EI_DECODE_LONG (state->port, buffer, termIndex, x2);
		EI_DECODE_LONG (state->port, buffer, termIndex, y2);
		EI_DECODE_LONG (state->port, buffer, termIndex, color);

		gdImageLine (state->image[imageIndex], x1, y1, x2, y2, color);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_RECTANGLE:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long x1, y1, x2, y2, color;

		EI_DECODE_LONG (state->port, buffer, termIndex, x1);
		EI_DECODE_LONG (state->port, buffer, termIndex, y1);
		EI_DECODE_LONG (state->port, buffer, termIndex, x2);
		EI_DECODE_LONG (state->port, buffer, termIndex, y2);
		EI_DECODE_LONG (state->port, buffer, termIndex, color);

		gdImageRectangle (state->image[imageIndex], x1, y1, x2, y2, color);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_FILLED_RECTANGLE:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long x1, y1, x2, y2, color;

		EI_DECODE_LONG (state->port, buffer, termIndex, x1);
		EI_DECODE_LONG (state->port, buffer, termIndex, y1);
		EI_DECODE_LONG (state->port, buffer, termIndex, x2);
		EI_DECODE_LONG (state->port, buffer, termIndex, y2);
		EI_DECODE_LONG (state->port, buffer, termIndex, color);

		gdImageFilledRectangle (state->image[imageIndex], x1, y1, x2, y2, color);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_ELLIPSE:
	{
		// void gdImageEllipse (gdImagePtr im, int cx, int cy, int w, int h, int color);
		driver_failure_atom (state->port, "not_implemented");

		return;
	}

	case GD_IMAGE_FILLED_ELLIPSE:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long centerX, centerY, width, height, color;

		EI_DECODE_LONG (state->port, buffer, termIndex, centerX);
		EI_DECODE_LONG (state->port, buffer, termIndex, centerY);
		EI_DECODE_LONG (state->port, buffer, termIndex, width);
		EI_DECODE_LONG (state->port, buffer, termIndex, height);
		EI_DECODE_LONG (state->port, buffer, termIndex, color);

		gdImageFilledEllipse (state->image[imageIndex], centerX, centerY, width, height, color);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_ARC:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long centerX, centerY, width, height, initialDegree, finalDegree, color;

		EI_DECODE_LONG (state->port, buffer, termIndex, centerX);
		EI_DECODE_LONG (state->port, buffer, termIndex, centerY);
		EI_DECODE_LONG (state->port, buffer, termIndex, width);
		EI_DECODE_LONG (state->port, buffer, termIndex, height);
		EI_DECODE_LONG (state->port, buffer, termIndex, initialDegree);
		EI_DECODE_LONG (state->port, buffer, termIndex, finalDegree);
		EI_DECODE_LONG (state->port, buffer, termIndex, color);

		gdImageArc (state->image[imageIndex], centerX, centerY, width,
			    height, initialDegree, finalDegree, color);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_FILLED_ARC:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long centerX, centerY, width, height, initialDegree, finalDegree, style, color;

		EI_DECODE_LONG (state->port, buffer, termIndex, centerX);
		EI_DECODE_LONG (state->port, buffer, termIndex, centerY);
		EI_DECODE_LONG (state->port, buffer, termIndex, width);
		EI_DECODE_LONG (state->port, buffer, termIndex, height);
		EI_DECODE_LONG (state->port, buffer, termIndex, initialDegree);
		EI_DECODE_LONG (state->port, buffer, termIndex, finalDegree);
		EI_DECODE_LONG (state->port, buffer, termIndex, color);
		EI_DECODE_LONG (state->port, buffer, termIndex, style);

		gdImageFilledArc (state->image[imageIndex], centerX, centerY, width,
				  height, initialDegree, finalDegree, color, style);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_POLYGON:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		int count;

		EI_DECODE_LIST_HEADER (state->port, buffer, termIndex, count);

		gdPoint *points = (gdPoint *) driver_alloc (sizeof (gdPoint) * count);

		ASSERT_DRIVER_ALLOC (state->port, points, GD_IMAGE_POLYGON_failure0);

		for (int i = 0; i < count; i++)
		{
			int arity;

			EI_DECODE_TUPLE_HEADER (state->port, buffer, termIndex, arity);

			long x, y;

			EI_DECODE_LONG (state->port, buffer, termIndex, x);
			EI_DECODE_LONG (state->port, buffer, termIndex, y);

			points[i].x = x;
			points[i].y = y;
		}

        // get the empty list at the end
        EI_DECODE_LIST_HEADER (state->port, buffer, termIndex, count);

		long color;

		EI_DECODE_LONG (state->port, buffer, termIndex, color);

		gdImagePolygon (state->image[imageIndex], points, count, color);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		driver_free (points);

	GD_IMAGE_POLYGON_failure0:
		return;
	}

	case GD_IMAGE_OPEN_POLYGON:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		int count;

		EI_DECODE_LIST_HEADER (state->port, buffer, termIndex, count);

		gdPoint *points = (gdPoint *) driver_alloc (sizeof (gdPoint) * count);

		ASSERT_DRIVER_ALLOC (state->port, points, GD_IMAGE_OPEN_POLYGON_failure0);

		for (int i = 0; i < count; i++)
		{
			int arity;

			EI_DECODE_TUPLE_HEADER (state->port, buffer, termIndex, arity);

			long x, y;

			EI_DECODE_LONG (state->port, buffer, termIndex, x);
			EI_DECODE_LONG (state->port, buffer, termIndex, y);

			points[i].x = x;
			points[i].y = y;
		}

        // get the empty list at the end
        EI_DECODE_LIST_HEADER (state->port, buffer, termIndex, count);

		long color;

		EI_DECODE_LONG (state->port, buffer, termIndex, color);

		gdImageOpenPolygon (state->image[imageIndex], points, count, color);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		driver_free (points);

	GD_IMAGE_OPEN_POLYGON_failure0:
		return;
	}

	case GD_IMAGE_FILLED_POLYGON:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		int count;

		EI_DECODE_LIST_HEADER (state->port, buffer, termIndex, count);

		gdPoint *points = (gdPoint *) driver_alloc (sizeof (gdPoint) * count);

		ASSERT_DRIVER_ALLOC (state->port, points, GD_IMAGE_FILLED_POLYGON_failure0);

		for (int i = 0; i < count; i++)
		{
			int arity;

			EI_DECODE_TUPLE_HEADER (state->port, buffer, termIndex, arity);

			long x, y;

			EI_DECODE_LONG (state->port, buffer, termIndex, x);
			EI_DECODE_LONG (state->port, buffer, termIndex, y);

			points[i].x = x;
			points[i].y = y;
		}

        // get the empty list at the end
        EI_DECODE_LIST_HEADER (state->port, buffer, termIndex, count);

		long color;

		EI_DECODE_LONG (state->port, buffer, termIndex, color);

		gdImageFilledPolygon (state->image[imageIndex], points, count, color);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		driver_free (points);

	GD_IMAGE_FILLED_POLYGON_failure0:
		return;
	}

	case GD_IMAGE_FILL:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long x, y, color;

		EI_DECODE_LONG (state->port, buffer, termIndex, x);
		EI_DECODE_LONG (state->port, buffer, termIndex, y);
		EI_DECODE_LONG (state->port, buffer, termIndex, color);

		gdImageFill (state->image[imageIndex], x, y, color);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_FILL_TO_BORDER:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long x, y, borderColor, color;

		EI_DECODE_LONG (state->port, buffer, termIndex, x);
		EI_DECODE_LONG (state->port, buffer, termIndex, y);
		EI_DECODE_LONG (state->port, buffer, termIndex, borderColor);
		EI_DECODE_LONG (state->port, buffer, termIndex, color);

		gdImageFillToBorder (state->image[imageIndex], x, y, borderColor, color);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_STRING_FT:
	{
		long imageIndex, x, y, color;

		double pointSize, angle, lineSpacing;

		EI_DECODE_LONG (state->port, buffer, termIndex, imageIndex);
		EI_DECODE_LONG (state->port, buffer, termIndex, color);

		int type, size;

		if (ei_get_type (buffer, &termIndex, &type, &size) == -1)
		{
			driver_failure_atom (state->port, "ei_get_type");
			goto GD_IMAGE_STRING_FT_failure0;
		}

		char *fontPath = (char *) driver_alloc (size + 1);

		ASSERT_DRIVER_ALLOC (state->port, fontPath, GD_IMAGE_STRING_FT_failure0);

		EI_DECODE_STRING (state->port, buffer, termIndex, fontPath);
		EI_DECODE_DOUBLE (state->port, buffer, termIndex, pointSize);
		EI_DECODE_DOUBLE (state->port, buffer, termIndex, lineSpacing);
		EI_DECODE_DOUBLE (state->port, buffer, termIndex, angle);
		EI_DECODE_LONG (state->port, buffer, termIndex, x);
		EI_DECODE_LONG (state->port, buffer, termIndex, y);

		if (ei_get_type (buffer, &termIndex, &type, &size) == -1)
		{
			driver_failure_atom (state->port, "ei_get_type");
			goto GD_IMAGE_STRING_FT_failure1;
		}

		char *text = (char *) driver_alloc (size + 1);

		ASSERT_DRIVER_ALLOC (state->port, text, GD_IMAGE_STRING_FT_failure1);

		EI_DECODE_STRING (state->port, buffer, termIndex, text);

		int bounds[8];

		char *error;

		gdFTStringExtra extra;

		extra.flags = gdFTEX_LINESPACE;
		extra.linespacing = lineSpacing;

		if (imageIndex == IMAGE_NULL_INDEX)
			error = gdImageStringFTEx (NULL, bounds, color, fontPath, pointSize, angle, x, y, text, &extra);
		else
		{
			if (!assert_index (state, imageIndex))
				goto GD_IMAGE_STRING_FT_failure2;

			error = gdImageStringFTEx (state->image[imageIndex], bounds, color, fontPath,
						   pointSize, angle, x, y, text, &extra);
		}

		if (error != NULL)
		{
			ErlDrvTermData spec[] = {
				ERL_DRV_PORT, driver_mk_port (state->port),
				ERL_DRV_ATOM, driver_mk_atom (ATOM_ERROR),
				ERL_DRV_STRING, (ErlDrvTermData) error, strlen (error),
				ERL_DRV_TUPLE, 2,
				ERL_DRV_TUPLE, 2 };

			driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));
		}
		else
		{
			ErlDrvTermData spec[] = {
				ERL_DRV_PORT, driver_mk_port (state->port),
				ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
				ERL_DRV_ATOM, driver_mk_atom (ATOM_GD_BOUNDS),
				ERL_DRV_INT, bounds[0], ERL_DRV_INT, bounds[1],
				ERL_DRV_INT, bounds[2], ERL_DRV_INT, bounds[3],
				ERL_DRV_INT, bounds[4], ERL_DRV_INT, bounds[5],
				ERL_DRV_INT, bounds[6], ERL_DRV_INT, bounds[7],
				ERL_DRV_TUPLE, 9,
				ERL_DRV_TUPLE, 2,
				ERL_DRV_TUPLE, 2 };

			driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));
		}

	GD_IMAGE_STRING_FT_failure2:
		driver_free (text);

	GD_IMAGE_STRING_FT_failure1:
		driver_free (fontPath);

	GD_IMAGE_STRING_FT_failure0:
		return;
	}

	case GD_IMAGE_COPY:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndexDst);
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndexSrc);

		long dstX, dstY, srcX, srcY, width, height;

		EI_DECODE_LONG (state->port, buffer, termIndex, dstX);
		EI_DECODE_LONG (state->port, buffer, termIndex, dstY);
		EI_DECODE_LONG (state->port, buffer, termIndex, srcX);
		EI_DECODE_LONG (state->port, buffer, termIndex, srcY);
		EI_DECODE_LONG (state->port, buffer, termIndex, width);
		EI_DECODE_LONG (state->port, buffer, termIndex, height);

		gdImageCopy (state->image[imageIndexDst], state->image[imageIndexSrc],
			     dstX, dstY, srcX, srcY, width, height);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_COPY_MERGE:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndexDst);
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndexSrc);

		long dstX, dstY, srcX, srcY, width, height, percent;

		EI_DECODE_LONG (state->port, buffer, termIndex, dstX);
		EI_DECODE_LONG (state->port, buffer, termIndex, dstY);
		EI_DECODE_LONG (state->port, buffer, termIndex, srcX);
		EI_DECODE_LONG (state->port, buffer, termIndex, srcY);
		EI_DECODE_LONG (state->port, buffer, termIndex, width);
		EI_DECODE_LONG (state->port, buffer, termIndex, height);
		EI_DECODE_LONG (state->port, buffer, termIndex, percent);

		gdImageCopyMerge (state->image[imageIndexDst], state->image[imageIndexSrc],
				  dstX, dstY, srcX, srcY, width, height, percent);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_COPY_MERGE_GRAY:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndexDst);
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndexSrc);

		long dstX, dstY, srcX, srcY, width, height, percent;

		EI_DECODE_LONG (state->port, buffer, termIndex, dstX);
		EI_DECODE_LONG (state->port, buffer, termIndex, dstY);
		EI_DECODE_LONG (state->port, buffer, termIndex, srcX);
		EI_DECODE_LONG (state->port, buffer, termIndex, srcY);
		EI_DECODE_LONG (state->port, buffer, termIndex, width);
		EI_DECODE_LONG (state->port, buffer, termIndex, height);
		EI_DECODE_LONG (state->port, buffer, termIndex, percent);

		gdImageCopyMergeGray (state->image[imageIndexDst], state->image[imageIndexSrc],
				      dstX, dstY, srcX, srcY, width, height, percent);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_COPY_RESIZED:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndexDst);
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndexSrc);

		long dstX, dstY, srcX, srcY, dstWidth, dstHeight, srcWidth, srcHeight;

		EI_DECODE_LONG (state->port, buffer, termIndex, dstX);
		EI_DECODE_LONG (state->port, buffer, termIndex, dstY);
		EI_DECODE_LONG (state->port, buffer, termIndex, srcX);
		EI_DECODE_LONG (state->port, buffer, termIndex, srcY);
		EI_DECODE_LONG (state->port, buffer, termIndex, dstWidth);
		EI_DECODE_LONG (state->port, buffer, termIndex, dstHeight);
		EI_DECODE_LONG (state->port, buffer, termIndex, srcWidth);
		EI_DECODE_LONG (state->port, buffer, termIndex, srcHeight);

		gdImageCopyResized (state->image[imageIndexDst], state->image[imageIndexSrc],
				    dstX, dstY, srcX, srcY, dstWidth, dstHeight, srcWidth, srcHeight);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_COPY_RESAMPLED:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndexDst);
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndexSrc);

		long dstX, dstY, srcX, srcY, dstWidth, dstHeight, srcWidth, srcHeight;

		EI_DECODE_LONG (state->port, buffer, termIndex, dstX);
		EI_DECODE_LONG (state->port, buffer, termIndex, dstY);
		EI_DECODE_LONG (state->port, buffer, termIndex, srcX);
		EI_DECODE_LONG (state->port, buffer, termIndex, srcY);
		EI_DECODE_LONG (state->port, buffer, termIndex, dstWidth);
		EI_DECODE_LONG (state->port, buffer, termIndex, dstHeight);
		EI_DECODE_LONG (state->port, buffer, termIndex, srcWidth);
		EI_DECODE_LONG (state->port, buffer, termIndex, srcHeight);

		gdImageCopyResampled (state->image[imageIndexDst], state->image[imageIndexSrc],
				      dstX, dstY, srcX, srcY, dstWidth, dstHeight, srcWidth, srcHeight);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_COPY_ROTATED:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndexDst);
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndexSrc);

		double dstCenterX, dstCenterY;

		long srcX, srcY, srcWidth, srcHeight, angle;

		EI_DECODE_DOUBLE (state->port, buffer, termIndex, dstCenterX);
		EI_DECODE_DOUBLE (state->port, buffer, termIndex, dstCenterY);
		EI_DECODE_LONG (state->port, buffer, termIndex, srcX);
		EI_DECODE_LONG (state->port, buffer, termIndex, srcY);
		EI_DECODE_LONG (state->port, buffer, termIndex, srcWidth);
		EI_DECODE_LONG (state->port, buffer, termIndex, srcHeight);
		EI_DECODE_LONG (state->port, buffer, termIndex, angle);

		gdImageCopyRotated (state->image[imageIndexDst], state->image[imageIndexSrc], dstCenterX,
				    dstCenterY, srcX, srcY, srcWidth, srcHeight, angle);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_SET_BRUSH:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, brushIndex);

		gdImageSetBrush (state->image[imageIndex], state->image[brushIndex]);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_SET_TILE:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, tileIndex);

		gdImageSetTile (state->image[imageIndex], state->image[tileIndex]);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_SET_ANTI_ALIASED:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long color;

		EI_DECODE_LONG (state->port, buffer, termIndex, color);

		gdImageSetAntiAliased (state->image[imageIndex], color);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_SET_ANTI_ALIASED_DONT_BLEND:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long color, dontBlend;

		EI_DECODE_LONG (state->port, buffer, termIndex, color);
		EI_DECODE_LONG (state->port, buffer, termIndex, dontBlend);

		gdImageSetAntiAliasedDontBlend (state->image[imageIndex], color, dontBlend);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_SET_STYLE:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		int count;

		EI_DECODE_LIST_HEADER (state->port, buffer, termIndex, count);

		int *style = (int *) driver_alloc (sizeof (int) * count);

		ASSERT_DRIVER_ALLOC (state->port, style, GD_IMAGE_SET_STYLE_failure0);

		for (int i = 0; i < count; i++)
		{
			long color;
			EI_DECODE_LONG (state->port, buffer, termIndex, color);
			style[i] = color;
		}

		gdImageSetStyle (state->image[imageIndex], style, count);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		driver_free (style);

	GD_IMAGE_SET_STYLE_failure0:
		return;
	}

	case GD_IMAGE_SET_THICKNESS:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long thickness;

		EI_DECODE_LONG (state->port, buffer, termIndex, thickness);

		gdImageSetThickness (state->image[imageIndex], thickness);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_INTERLACE:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long interlace;

		EI_DECODE_LONG (state->port, buffer, termIndex, interlace);

		gdImageInterlace (state->image[imageIndex], interlace);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_ALPHA_BLENDING:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long alphaBlending;

		EI_DECODE_LONG (state->port, buffer, termIndex, alphaBlending);

		gdImageAlphaBlending (state->image[imageIndex], alphaBlending);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_SAVE_ALPHA:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long saveAlpha;

		EI_DECODE_LONG (state->port, buffer, termIndex, saveAlpha);

		gdImageSaveAlpha (state->image[imageIndex], saveAlpha);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_CREATE_PALETTE_FROM_TRUE_COLOR:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long ditherFlag, colorsWanted;

		EI_DECODE_LONG (state->port, buffer, termIndex, ditherFlag);
		EI_DECODE_LONG (state->port, buffer, termIndex, colorsWanted);

		int paletteImageIndex = allocate_index (state);

		if (paletteImageIndex == -1)
		{
			driver_failure_atom (state->port, "too_many_images");

			return;
		}

		state->image[paletteImageIndex] = gdImageCreatePaletteFromTrueColor (state->image[imageIndex], ditherFlag, colorsWanted);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, paletteImageIndex,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_TRUE_COLOR_TO_PALETTE:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long ditherFlag, colorsWanted;

		EI_DECODE_LONG (state->port, buffer, termIndex, ditherFlag);
		EI_DECODE_LONG (state->port, buffer, termIndex, colorsWanted);

		gdImageTrueColorToPalette (state->image[imageIndex], ditherFlag, colorsWanted);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_COLOR_TRANSPARENT:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long transparent;

		EI_DECODE_LONG (state->port, buffer, termIndex, transparent);

		gdImageColorTransparent (state->image[imageIndex], transparent);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_PALETTE_COPY:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, dstImageIndex);
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, srcImageIndex);

		gdImagePaletteCopy (state->image[dstImageIndex], state->image[srcImageIndex]);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_TRUECOLOR:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		int imageTrueColor = gdImageTrueColor (state->image[imageIndex]);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_ATOM, driver_mk_atom (imageTrueColor ? ATOM_TRUE : ATOM_FALSE),
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_SX:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		int imageSX = gdImageSX (state->image[imageIndex]);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, imageSX,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_SY:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		int imageSY = gdImageSY (state->image[imageIndex]);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, imageSY,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_COLORS_TOTAL:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		int colorsTotal = gdImageColorsTotal (state->image[imageIndex]);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, colorsTotal,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_RED:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long color;

		EI_DECODE_LONG (state->port, buffer, termIndex, color);

		int imageRed = gdImageRed (state->image[imageIndex], color);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, imageRed,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_GREEN:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long color;

		EI_DECODE_LONG (state->port, buffer, termIndex, color);

		int imageGreen = gdImageGreen (state->image[imageIndex], color);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, imageGreen,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_BLUE:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long color;

		EI_DECODE_LONG (state->port, buffer, termIndex, color);

		int imageBlue = gdImageBlue (state->image[imageIndex], color);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, imageBlue,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_ALPHA:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long color;

		EI_DECODE_LONG (state->port, buffer, termIndex, color);

		int imageAlpha = gdImageAlpha (state->image[imageIndex], color);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, imageAlpha,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_GET_TRANSPARENT:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		int transparent = gdImageGetTransparent (state->image[imageIndex]);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_INT, transparent,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_GET_INTERLACED:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		int interlaced = gdImageGetInterlaced (state->image[imageIndex]);

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_ATOM, driver_mk_atom (interlaced ? ATOM_TRUE : ATOM_FALSE),
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		return;
	}

	case GD_IMAGE_GET_PALETTE:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		if (gdImageTrueColor (state->image[imageIndex]))
			driver_failure_atom (state->port, ATOM_BADARG);

		int colorsTotal = gdImageColorsTotal (state->image[imageIndex]);

		ErlDrvTermData *spec = (ErlDrvTermData *) driver_alloc (sizeof (ErlDrvTermData) * (11 + 10 * colorsTotal));

		ASSERT_DRIVER_ALLOC (state->port, spec, GD_IMAGE_GET_PALETTE_failure0);

		ErlDrvTermData *current = spec;

		*current++ = ERL_DRV_PORT;
		*current++ = driver_mk_port (state->port);

		*current++ = ERL_DRV_ATOM;
		*current++ = driver_mk_atom (ATOM_OK);

		for (int color = 0; color < colorsTotal; color++)
		{
			*current++ = ERL_DRV_INT;
			*current++ = gdImageRed (state->image[imageIndex], color);

			*current++ = ERL_DRV_INT;
			*current++ = gdImageGreen (state->image[imageIndex], color);

			*current++ = ERL_DRV_INT;
			*current++ = gdImageBlue (state->image[imageIndex], color);

			*current++ = ERL_DRV_INT;
			*current++ = gdImageAlpha (state->image[imageIndex], color);

			*current++ = ERL_DRV_TUPLE;
			*current++ = 4;
		}

		*current++ = ERL_DRV_NIL;
		*current++ = ERL_DRV_LIST;
		*current++ = colorsTotal + 1;

		*current++ = ERL_DRV_TUPLE;
		*current++ = 2;

		*current++ = ERL_DRV_TUPLE;
		*current++ = 2;

		driver_output_term (state->port, spec, current - spec);

		driver_free (spec);

	GD_IMAGE_GET_PALETTE_failure0:
		return;
	}

	case GD_IMAGE_GET_ROW_INDEXED:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long row;

		EI_DECODE_LONG (state->port, buffer, termIndex, row);

		if (gdImageTrueColor (state->image[imageIndex]))
			driver_failure_atom (state->port, ATOM_BADARG);

		int sizeX = gdImageSX (state->image[imageIndex]);
		int sizeY = gdImageSY (state->image[imageIndex]);

		if (row >= sizeY)
		{
			driver_failure_atom (state->port, ATOM_BADARG);

			return;
		}

		ErlDrvTermData *spec = (ErlDrvTermData *) driver_alloc (sizeof (ErlDrvTermData) * (11 + 2 * sizeX));

		ASSERT_DRIVER_ALLOC (state->port, spec, GD_IMAGE_GET_ROW_INDEXED_failure0);

		ErlDrvTermData *current = spec;

		*current++ = ERL_DRV_PORT;
		*current++ = driver_mk_port (state->port);

		*current++ = ERL_DRV_ATOM;
		*current++ = driver_mk_atom (ATOM_OK);

		for (int x = 0; x < sizeX; x++)
		{
			*current++ = ERL_DRV_INT;
			*current++ = gdImagePalettePixel (state->image[imageIndex], x, row);
		}

		*current++ = ERL_DRV_NIL;
		*current++ = ERL_DRV_LIST;
		*current++ = sizeX + 1;

		*current++ = ERL_DRV_TUPLE;
		*current++ = 2;

		*current++ = ERL_DRV_TUPLE;
		*current++ = 2;

		driver_output_term (state->port, spec, current - spec);

		driver_free (spec);

	GD_IMAGE_GET_ROW_INDEXED_failure0:
		return;
	}

	case GD_IMAGE_GET_ROW_INDEXED_RLE:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long row;

		EI_DECODE_LONG (state->port, buffer, termIndex, row);

		if (gdImageTrueColor (state->image[imageIndex]))
			driver_failure_atom (state->port, ATOM_BADARG);

		int sizeX = gdImageSX (state->image[imageIndex]);
		int sizeY = gdImageSY (state->image[imageIndex]);

		if (row >= sizeY)
		{
			driver_failure_atom (state->port, ATOM_BADARG);

			return;
		}

		uint32_t *rle_array = NULL;
		uint32_t rle_length = 0;

		rle_encode_u8 ((uint8_t *) state->image[imageIndex]->pixels[row], sizeX, &rle_array, &rle_length);

		TERM_VECTOR_ALLOC (spec, 11 + 6 * (rle_length >> 1));

		ASSERT_DRIVER_ALLOC (state->port, spec, GD_IMAGE_GET_ROW_INDEXED_RLE_failure0);

		TERM_VECTOR_PUSH (spec, ERL_DRV_PORT);
		TERM_VECTOR_PUSH (spec, driver_mk_port (state->port));

		TERM_VECTOR_PUSH (spec, ERL_DRV_ATOM);
		TERM_VECTOR_PUSH (spec, driver_mk_atom (ATOM_OK));

		for (int x = 0; x < rle_length >> 1; x++)
		{
			TERM_VECTOR_PUSH (spec, ERL_DRV_INT);
			TERM_VECTOR_PUSH (spec, rle_array [(x << 1) + 1]);

			TERM_VECTOR_PUSH (spec, ERL_DRV_INT);
			TERM_VECTOR_PUSH (spec, rle_array [x << 1]);

			TERM_VECTOR_PUSH (spec, ERL_DRV_TUPLE);
			TERM_VECTOR_PUSH (spec, 2);
		}

		TERM_VECTOR_PUSH (spec, ERL_DRV_NIL);
		TERM_VECTOR_PUSH (spec, ERL_DRV_LIST);
		TERM_VECTOR_PUSH (spec, (rle_length >> 1) + 1);

		TERM_VECTOR_PUSH (spec, ERL_DRV_TUPLE);
		TERM_VECTOR_PUSH (spec, 2);

		TERM_VECTOR_PUSH (spec, ERL_DRV_TUPLE);
		TERM_VECTOR_PUSH (spec, 2);

		TERM_VECTOR_OUTPUT (spec, state->port);

		TERM_VECTOR_FREE (spec);

		free (rle_array);

	GD_IMAGE_GET_ROW_INDEXED_RLE_failure0:
		return;
	}

	case GD_IMAGE_GET_ROW_TRUECOLOR:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		long row;

		EI_DECODE_LONG (state->port, buffer, termIndex, row);

		int sizeX = gdImageSX (state->image[imageIndex]);
		int sizeY = gdImageSY (state->image[imageIndex]);

		if (row >= sizeY)
		{
			driver_failure_atom (state->port, ATOM_BADARG);

			return;
		}

		ErlDrvTermData *spec = (ErlDrvTermData *) driver_alloc (sizeof (ErlDrvTermData) * (11 + 10 * sizeX));

		ASSERT_DRIVER_ALLOC (state->port, spec, GD_IMAGE_GET_ROW_TRUECOLOR_failure0);

		ErlDrvTermData *current = spec;

		*current++ = ERL_DRV_PORT;
		*current++ = driver_mk_port (state->port);

		*current++ = ERL_DRV_ATOM;
		*current++ = driver_mk_atom (ATOM_OK);

		for (int x = 0; x < sizeX; x++)
		{
			int color;

			if (gdImageTrueColor (state->image[imageIndex]))
				color = gdImageTrueColorPixel (state->image[imageIndex], x, row);
			else
				color = gdImagePalettePixel (state->image[imageIndex], x, row);

			*current++ = ERL_DRV_INT;
			*current++ = gdImageRed (state->image[imageIndex], color);

			*current++ = ERL_DRV_INT;
			*current++ = gdImageGreen (state->image[imageIndex], color);

			*current++ = ERL_DRV_INT;
			*current++ = gdImageBlue (state->image[imageIndex], color);

			*current++ = ERL_DRV_INT;
			*current++ = gdImageAlpha (state->image[imageIndex], color);

			*current++ = ERL_DRV_TUPLE;
			*current++ = 4;
		}

		*current++ = ERL_DRV_NIL;
		*current++ = ERL_DRV_LIST;
		*current++ = sizeX + 1;

		*current++ = ERL_DRV_TUPLE;
		*current++ = 2;

		*current++ = ERL_DRV_TUPLE;
		*current++ = 2;

		driver_output_term (state->port, spec, current - spec);

		driver_free (spec);

	GD_IMAGE_GET_ROW_TRUECOLOR_failure0:
		return;
	}

	case GD_IMAGE_GET_INDEXED_RLE:
	{
		EI_DECODE_IMAGE_INDEX (state, buffer, termIndex, imageIndex);

		if (gdImageTrueColor (state->image[imageIndex]))
			driver_failure_atom (state->port, ATOM_BADARG);

		int sizeX = gdImageSX (state->image[imageIndex]);
		int sizeY = gdImageSY (state->image[imageIndex]);

		TERM_VECTOR_ALLOC (spec, 256);

		ASSERT_DRIVER_ALLOC (state->port, spec, GD_IMAGE_GET_INDEXED_RLE_failure0);

		TERM_VECTOR_PUSH (spec, ERL_DRV_PORT);
		TERM_VECTOR_PUSH (spec, driver_mk_port (state->port));

		TERM_VECTOR_PUSH (spec, ERL_DRV_ATOM);
		TERM_VECTOR_PUSH (spec, driver_mk_atom (ATOM_OK));

		for (int y = 0; y < sizeY; y++)
		{
			uint32_t *rle_array = NULL;
			uint32_t rle_length = 0;

			rle_encode_u8 ((uint8_t *) state->image[imageIndex]->pixels[y], sizeX, &rle_array, &rle_length);

			for (int x = 0; x < rle_length >> 1; x++)
			{
				TERM_VECTOR_PUSH (spec, ERL_DRV_INT);
				TERM_VECTOR_PUSH (spec, rle_array [(x << 1) + 1]);

				TERM_VECTOR_PUSH (spec, ERL_DRV_INT);
				TERM_VECTOR_PUSH (spec, rle_array [x << 1]);

				TERM_VECTOR_PUSH (spec, ERL_DRV_TUPLE);
				TERM_VECTOR_PUSH (spec, 2);
			}

			TERM_VECTOR_PUSH (spec, ERL_DRV_NIL);
			TERM_VECTOR_PUSH (spec, ERL_DRV_LIST);
			TERM_VECTOR_PUSH (spec, (rle_length >> 1) + 1);

			free (rle_array);
		}

		TERM_VECTOR_PUSH (spec, ERL_DRV_NIL);
		TERM_VECTOR_PUSH (spec, ERL_DRV_LIST);
		TERM_VECTOR_PUSH (spec, sizeY + 1);

		TERM_VECTOR_PUSH (spec, ERL_DRV_TUPLE);
		TERM_VECTOR_PUSH (spec, 2);

		TERM_VECTOR_PUSH (spec, ERL_DRV_TUPLE);
		TERM_VECTOR_PUSH (spec, 2);

		TERM_VECTOR_OUTPUT (spec, state->port);

		TERM_VECTOR_FREE (spec);

	GD_IMAGE_GET_INDEXED_RLE_failure0:
		return;
	}

	case GD_GET_FACE:
	{
		int type, size;

		if (ei_get_type (buffer, &termIndex, &type, &size) == -1)
		{
			driver_failure_atom (state->port, "ei_get_type");
			goto GD_GET_FACE_failure0;
		}

		char *fontPath = (char *) driver_alloc (size + 1);

		ASSERT_DRIVER_ALLOC (state->port, fontPath, GD_GET_FACE_failure0);

		EI_DECODE_STRING (state->port, buffer, termIndex, fontPath);

		FT_Face face;

		if (FT_New_Face (freeTypeLibrary, fontPath, 0, &face))
		{
			driver_failure_atom (state->port, "FT_New_Face");
			goto GD_GET_FACE_failure1;
		}

		ErlDrvTermData spec[] = {
			ERL_DRV_PORT, driver_mk_port (state->port),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_OK),
			ERL_DRV_ATOM, driver_mk_atom (ATOM_GD_FACE),
			ERL_DRV_STRING, (ErlDrvTermData) face->family_name, strlen (face->family_name),
			ERL_DRV_STRING, (ErlDrvTermData) face->style_name, strlen (face->style_name),
			ERL_DRV_TUPLE, 3,
			ERL_DRV_TUPLE, 2,
			ERL_DRV_TUPLE, 2 };

		driver_output_term (state->port, spec, sizeof (spec) / sizeof (spec[0]));

		FT_Done_Face (face);

	GD_GET_FACE_failure1:
		driver_free (fontPath);

	GD_GET_FACE_failure0:
		return;
	}

	default:
		driver_failure_atom (state->port, "switch_failed");
	}
}

ErlDrvEntry driver_entry = {
	.init            = init,
	.finish          = finish,
	.start           = start,
	.stop            = stop,
	.output          = output,
	.ready_input     = NULL,
	.ready_output    = NULL,
	.driver_name     = "elib_gd_drv",
	.control         = NULL,
	.timeout         = NULL,
	.outputv         = NULL,
	.ready_async     = NULL,
	.flush           = NULL,
	.call            = NULL,
	.event           = NULL,
	.extended_marker = ERL_DRV_EXTENDED_MARKER,
	.major_version   = ERL_DRV_EXTENDED_MAJOR_VERSION,
	.minor_version   = ERL_DRV_EXTENDED_MINOR_VERSION,
	.driver_flags    = 0,
	.process_exit    = NULL,
	.stop_select     = NULL
};

DRIVER_INIT (elib_gd_drv)	/* must match name in driver_entry */
{
	return &driver_entry;
}
