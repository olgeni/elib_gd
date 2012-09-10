%%--------------------------------------------------------------------
%% $Id: gd.erl 30386 2012-09-02 12:00:27Z olgeni $
%%--------------------------------------------------------------------

-module (gd).

-vsn ("$Id: gd.erl 30386 2012-09-02 12:00:27Z olgeni $").

%%====================================================================
%% Include files
%%====================================================================

-include ("gd.hrl").
-include ("gd_bounds.hrl").
-include ("gd_font.hrl").

%%====================================================================
%% External exports
%%====================================================================

-export ([new/0,
          stop/1,
          image_create/3,
          image_create_true_color/3,
          image_destroy/2,
          image_set_pixel/5,
          image_get_pixel/4,
          image_color_allocate/5,
          image_color_allocate_alpha/6,
          image_color_closest/5,
          image_color_closest_alpha/6,
          image_color_closest_hwb/5,
          image_color_exact/5,
          image_color_exact_alpha/6,
          image_color_resolve/5,
          image_color_resolve_alpha/6,
          image_color_deallocate/3,
          image_create_from_png_ptr/2,
          image_create_from_jpeg_ptr/2,
          image_create_from_gif_ptr/2,
          image_png_ptr/2,
          image_png_ptr_ex/3,
          image_jpeg_ptr/3,
          image_gif_ptr/2,
          image_set_clip/6,
          image_get_clip/2,
          image_bounds_safe/4,
          image_line/7,
          image_rectangle/7,
          image_filled_rectangle/7,
          image_ellipse/7,
          image_filled_ellipse/7,
          image_arc/9,
          image_filled_arc/10,
          image_polygon/4,
          image_open_polygon/4,
          image_filled_polygon/4,
          image_fill/5,
          image_fill_to_border/6,
          image_string_ft/8,
          image_string_ft_border/10,
          image_copy/9,
          image_copy_merge/10,
          image_copy_merge_gray/10,
          image_copy_resized/11,
          image_copy_resampled/11,
          image_copy_rotated/10,
          image_set_brush/3,
          image_set_tile/3,
          image_set_anti_aliased/3,
          image_set_anti_aliased_dont_blend/4,
          image_set_style/3,
          image_set_thickness/3,
          image_interlace/3,
          image_alpha_blending/3,
          image_save_alpha/3,
          image_create_palette_from_true_color/4,
          image_true_color_to_palette/4,
          image_color_transparent/3,
          image_palette_copy/3,
          image_true_color/2,
          image_sx/2,
          image_sy/2,
          image_colors_total/2,
          image_red/3,
          image_green/3,
          image_blue/3,
          image_alpha/3,
          image_get_transparent/2,
          image_get_interlaced/2,
          image_get_palette/2,
          image_get_row_indexed/3,
          image_get_row_indexed_rle/3,
          image_get_row_truecolor/3,
          image_get_indexed_rle/2,
          text_size/3,
          text_width/3,
          text_height/3,
          font_metrics/2,
          get_face/2,
          get_font_root/0]).

%%====================================================================
%% Internal exports
%%====================================================================

%%====================================================================
%% Macros
%%====================================================================

-define (GD_IMAGE_CREATE,                         16#00).
-define (GD_IMAGE_CREATE_TRUE_COLOR,              16#01).
-define (GD_IMAGE_DESTROY,                        16#02).
-define (GD_IMAGE_SET_PIXEL,                      16#03).
-define (GD_IMAGE_GET_PIXEL,                      16#04).
-define (GD_IMAGE_COLOR_ALLOCATE,                 16#05).
-define (GD_IMAGE_COLOR_ALLOCATE_ALPHA,           16#06).
-define (GD_IMAGE_COLOR_CLOSEST,                  16#07).
-define (GD_IMAGE_COLOR_CLOSEST_ALPHA,            16#08).
-define (GD_IMAGE_COLOR_CLOSEST_HWB,              16#09).
-define (GD_IMAGE_COLOR_EXACT,                    16#0A).
-define (GD_IMAGE_COLOR_EXACT_ALPHA,              16#0B).
-define (GD_IMAGE_COLOR_RESOLVE,                  16#0C).
-define (GD_IMAGE_COLOR_RESOLVE_ALPHA,            16#0D).
-define (GD_IMAGE_COLOR_DEALLOCATE,               16#0E).
-define (GD_IMAGE_CREATE_FROM_PNG_PTR,            16#0F).
-define (GD_IMAGE_CREATE_FROM_JPEG_PTR,           16#10).
-define (GD_IMAGE_CREATE_FROM_GIF_PTR,            16#11).
-define (GD_IMAGE_PNG_PTR,                        16#12).
-define (GD_IMAGE_PNG_PTR_EX,                     16#13).
-define (GD_IMAGE_JPEG_PTR,                       16#14).
-define (GD_IMAGE_GIF_PTR,                        16#15).
-define (GD_IMAGE_SET_CLIP,                       16#16).
-define (GD_IMAGE_GET_CLIP,                       16#17).
-define (GD_IMAGE_BOUNDS_SAFE,                    16#18).
-define (GD_IMAGE_LINE,                           16#19).
-define (GD_IMAGE_RECTANGLE,                      16#1A).
-define (GD_IMAGE_FILLED_RECTANGLE,               16#1B).
-define (GD_IMAGE_ELLIPSE,                        16#1C).
-define (GD_IMAGE_FILLED_ELLIPSE,                 16#1D).
-define (GD_IMAGE_ARC,                            16#1E).
-define (GD_IMAGE_FILLED_ARC,                     16#1F).
-define (GD_IMAGE_POLYGON,                        16#20).
-define (GD_IMAGE_OPEN_POLYGON,                   16#21).
-define (GD_IMAGE_FILLED_POLYGON,                 16#22).
-define (GD_IMAGE_FILL,                           16#23).
-define (GD_IMAGE_FILL_TO_BORDER,                 16#24).
-define (GD_IMAGE_STRING_FT,                      16#25).
-define (GD_IMAGE_COPY,                           16#26).
-define (GD_IMAGE_COPY_MERGE,                     16#27).
-define (GD_IMAGE_COPY_MERGE_GRAY,                16#28).
-define (GD_IMAGE_COPY_RESIZED,                   16#29).
-define (GD_IMAGE_COPY_RESAMPLED,                 16#2A).
-define (GD_IMAGE_COPY_ROTATED,                   16#2B).
-define (GD_IMAGE_SET_BRUSH,                      16#2C).
-define (GD_IMAGE_SET_TILE,                       16#2D).
-define (GD_IMAGE_SET_ANTI_ALIASED,               16#2E).
-define (GD_IMAGE_SET_ANTI_ALIASED_DONT_BLEND,    16#2F).
-define (GD_IMAGE_SET_STYLE,                      16#30).
-define (GD_IMAGE_SET_THICKNESS,                  16#31).
-define (GD_IMAGE_INTERLACE,                      16#32).
-define (GD_IMAGE_ALPHA_BLENDING,                 16#33).
-define (GD_IMAGE_SAVE_ALPHA,                     16#34).
-define (GD_IMAGE_CREATE_PALETTE_FROM_TRUE_COLOR, 16#35).
-define (GD_IMAGE_TRUE_COLOR_TO_PALETTE,          16#36).
-define (GD_IMAGE_COLOR_TRANSPARENT,              16#37).
-define (GD_IMAGE_PALETTE_COPY,                   16#38).
-define (GD_IMAGE_TRUECOLOR,                      16#39).
-define (GD_IMAGE_SX,                             16#3A).
-define (GD_IMAGE_SY,                             16#3B).
-define (GD_IMAGE_COLORS_TOTAL,                   16#3C).
-define (GD_IMAGE_RED,                            16#3D).
-define (GD_IMAGE_GREEN,                          16#3E).
-define (GD_IMAGE_BLUE,                           16#3F).
-define (GD_IMAGE_ALPHA,                          16#40).
-define (GD_IMAGE_GET_TRANSPARENT,                16#41).
-define (GD_IMAGE_GET_INTERLACED,                 16#42).
-define (GD_IMAGE_GET_PALETTE,                    16#43).
-define (GD_IMAGE_GET_ROW_INDEXED,                16#44).
-define (GD_IMAGE_GET_ROW_INDEXED_RLE,            16#45).
-define (GD_IMAGE_GET_ROW_TRUECOLOR,              16#46).
-define (GD_IMAGE_GET_INDEXED_RLE,                16#47).
-define (GD_GET_FACE,                             16#48).

-define (GD_ARC_CONST,    0).
-define (GD_CHORD_CONST,  1).
-define (GD_NOFILL_CONST, 2).
-define (GD_EDGED_CONST,  4).

%%====================================================================
%% Records
%%====================================================================

-record (gd, { port }).

%%====================================================================
%% External functions
%%====================================================================

%%
%% new
%%

new () ->
  GD = #gd{ port = open_port ({ spawn, "gd" }, [binary]) },
  { ok, GD }.

%%
%% stop
%%

stop (#gd{ port = Port }) ->
  unlink (Port),
  port_close (Port).

%%
%% image_create
%%

image_create (GD, Width, Height) ->
  transaction (GD, ?GD_IMAGE_CREATE, [Width, Height]).

%%
%% image_create_true_color
%%

image_create_true_color (GD, Width, Height) ->
  transaction (GD, ?GD_IMAGE_CREATE_TRUE_COLOR, [Width, Height]).

%%
%% image_destroy
%%

image_destroy (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_DESTROY, [ImageIndex]).

%%
%% image_set_pixel
%%

image_set_pixel (GD, ImageIndex, X, Y, Color) ->
  transaction (GD, ?GD_IMAGE_SET_PIXEL, [ImageIndex, X, Y, Color]).

%%
%% image_get_pixel
%%

image_get_pixel (GD, ImageIndex, X, Y) ->
  transaction (GD, ?GD_IMAGE_GET_PIXEL, [ImageIndex, X, Y]).

%%
%% image_color_allocate
%%

image_color_allocate (GD, ImageIndex, Red, Green, Blue) ->
  transaction (GD, ?GD_IMAGE_COLOR_ALLOCATE, [ImageIndex, Red, Green, Blue]).

%%
%% image_color_allocate_alpha
%%

image_color_allocate_alpha (GD, ImageIndex, Red, Green, Blue, Alpha) ->
  transaction (GD, ?GD_IMAGE_COLOR_ALLOCATE_ALPHA, [ImageIndex, Red, Green, Blue, Alpha]).

%%
%% image_color_closest
%%

image_color_closest (GD, ImageIndex, Red, Green, Blue) ->
  transaction (GD, ?GD_IMAGE_COLOR_CLOSEST, [ImageIndex, Red, Green, Blue]).

%%
%% image_color_closest_alpha
%%

image_color_closest_alpha (GD, ImageIndex, Red, Green, Blue, Alpha) ->
  transaction (GD, ?GD_IMAGE_COLOR_CLOSEST_ALPHA, [ImageIndex, Red, Green, Blue, Alpha]).

%%
%% image_color_closest_hwb
%%

image_color_closest_hwb (GD, ImageIndex, Red, Green, Blue) ->
  transaction (GD, ?GD_IMAGE_COLOR_CLOSEST_HWB, [ImageIndex, Red, Green, Blue]).

%%
%% image_color_exact
%%

image_color_exact (GD, ImageIndex, Red, Green, Blue) ->
  transaction (GD, ?GD_IMAGE_COLOR_EXACT, [ImageIndex, Red, Green, Blue]).

%%
%% image_color_exact_alpha
%%

image_color_exact_alpha (GD, ImageIndex, Red, Green, Blue, Alpha) ->
  transaction (GD, ?GD_IMAGE_COLOR_EXACT_ALPHA, [ImageIndex, Red, Green, Blue, Alpha]).

%%
%% image_color_resolve
%%

image_color_resolve (GD, ImageIndex, Red, Green, Blue) ->
  transaction (GD, ?GD_IMAGE_COLOR_RESOLVE, [ImageIndex, Red, Green, Blue]).

%%
%% image_color_resolve_alpha
%%

image_color_resolve_alpha (GD, ImageIndex, Red, Green, Blue, Alpha) ->
  transaction (GD, ?GD_IMAGE_COLOR_RESOLVE_ALPHA, [ImageIndex, Red, Green, Blue, Alpha]).

%%
%% image_color_deallocate
%%

image_color_deallocate (GD, ImageIndex, Color) ->
  transaction (GD, ?GD_IMAGE_COLOR_DEALLOCATE, [ImageIndex, Color]).

%%
%% image_create_from_png_ptr
%%

image_create_from_png_ptr (GD, Binary) ->
  transaction (GD, ?GD_IMAGE_CREATE_FROM_PNG_PTR, [Binary]).

%%
%% image_create_from_jpeg_ptr
%%

image_create_from_jpeg_ptr (GD, Binary) ->
  transaction (GD, ?GD_IMAGE_CREATE_FROM_JPEG_PTR, [Binary]).

%%
%% image_create_from_gif_ptr
%%

image_create_from_gif_ptr (GD, Binary) ->
  transaction (GD, ?GD_IMAGE_CREATE_FROM_GIF_PTR, [Binary]).

%%
%% image_png_ptr
%%

image_png_ptr (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_PNG_PTR, [ImageIndex]).

%%
%% image_png_ptr
%%

image_png_ptr_ex (GD, ImageIndex, Level) ->
  transaction (GD, ?GD_IMAGE_PNG_PTR, [ImageIndex, Level]).

%%
%% image_jpeg_ptr
%%

image_jpeg_ptr (GD, ImageIndex, Quality) ->
  transaction (GD, ?GD_IMAGE_JPEG_PTR, [ImageIndex, Quality]).

%%
%% image_gif_ptr
%%

image_gif_ptr (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_GIF_PTR, [ImageIndex]).

%%
%% image_set_clip
%%

image_set_clip (GD, ImageIndex, X1, Y1, X2, Y2) ->
  transaction (GD, ?GD_IMAGE_SET_CLIP, [ImageIndex, X1, Y1, X2, Y2]).

%%
%% image_get_clip
%%

image_get_clip (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_GET_CLIP, [ImageIndex]).

%%
%% image_bounds_safe
%%

image_bounds_safe (GD, ImageIndex, X, Y) ->
  transaction (GD, ?GD_IMAGE_BOUNDS_SAFE, [ImageIndex, X, Y]).

%%
%% image_line
%%

image_line (GD, ImageIndex, X1, Y1, X2, Y2, Color) ->
  transaction (GD, ?GD_IMAGE_LINE, [ImageIndex, X1, Y1, X2, Y2, Color]).

%%
%% image_rectangle
%%

image_rectangle (GD, ImageIndex, X1, Y1, X2, Y2, Color) ->
  transaction (GD, ?GD_IMAGE_RECTANGLE, [ImageIndex, X1, Y1, X2, Y2, Color]).

%%
%% image_filled_rectangle
%%

image_filled_rectangle (GD, ImageIndex, X1, Y1, X2, Y2, Color) ->
  transaction (GD, ?GD_IMAGE_FILLED_RECTANGLE, [ImageIndex, X1, Y1, X2, Y2, Color]).

%%
%% image_ellipse
%%

image_ellipse (GD, ImageIndex, CenterX, CenterY, Width, Height, Color) ->
  transaction (GD, ?GD_IMAGE_ELLIPSE, [ImageIndex, CenterX, CenterY, Width, Height, Color]).

%%
%% image_filled_ellipse
%%

image_filled_ellipse (GD, ImageIndex, CenterX, CenterY, Width, Height, Color) ->
  transaction (GD, ?GD_IMAGE_FILLED_ELLIPSE, [ImageIndex, CenterX, CenterY, Width, Height, Color]).

%%
%% image_arc
%%

image_arc (GD, ImageIndex, CenterX, CenterY, Width, Height, InitialDegree, FinalDegree, Color) ->
  transaction (GD, ?GD_IMAGE_ARC, [ImageIndex, CenterX, CenterY, Width, Height, InitialDegree, FinalDegree, Color]).

%%
%% image_filled_arc
%%

image_filled_arc (GD, ImageIndex, CenterX, CenterY, Width, Height, InitialDegree, FinalDegree, Color, Style) ->
  StyleValue =
    lists:foldl (                               %%  _____
      fun (X, A) ->                             %% /     \
          fun (?GD_ARC)    -> ?GD_ARC_CONST;    %% vvvvvvv  /|__/|
              (?GD_CHORD)  -> ?GD_CHORD_CONST;  %%    I   /O,O   |
              (?GD_NOFILL) -> ?GD_NOFILL_CONST; %%    I /_____   |      /|/|
              (?GD_EDGED)  -> ?GD_EDGED_CONST   %%   J|/^ ^ ^ \  |    /00  |    _//|
          end (X) bor A                         %%    |^ ^ ^ ^ |W|   |/^^\ |   /oo |
      end, 0, Style),                           %%     \m___m__|_|    \m_m_|   \mm_|

  transaction (GD, ?GD_IMAGE_FILLED_ARC, [ImageIndex, CenterX, CenterY, Width, Height, InitialDegree, FinalDegree, Color, StyleValue]).

%%
%% image_polygon
%%

image_polygon (GD, ImageIndex, Points, Color) when is_list (Points) ->
  transaction (GD, ?GD_IMAGE_POLYGON, [ImageIndex, Points, Color]).

%%
%% image_open_polygon
%%

image_open_polygon (GD, ImageIndex, Points, Color) when is_list (Points) ->
  transaction (GD, ?GD_IMAGE_OPEN_POLYGON, [ImageIndex, Points, Color]).

%%
%% image_filled_polygon
%%

image_filled_polygon (GD, ImageIndex, Points, Color) when is_list (Points) ->
  transaction (GD, ?GD_IMAGE_FILLED_POLYGON, [ImageIndex, Points, Color]).

%%
%% image_fill
%%

image_fill (GD, ImageIndex, X, Y, Color) ->
  transaction (GD, ?GD_IMAGE_FILL, [ImageIndex, X, Y, Color]).

%%
%% image_fill_to_border
%%

image_fill_to_border (GD, ImageIndex, X, Y, BorderColor, Color) ->
  transaction (GD, ?GD_IMAGE_FILL_TO_BORDER, [ImageIndex, X, Y, BorderColor, Color]).

%%
%% image_string_ft
%%

image_string_ft (GD, ImageIndex, Color, Font, Angle, X, Y, Text) ->
  transaction (
    GD, ?GD_IMAGE_STRING_FT,
    [ImageIndex, Color,
     gd_font:get_font_path (Font),
     float (gd_font:get_point_size (Font)),
     float (gd_font:get_line_spacing (Font)),
     float (Angle), X, Y, Text]).

%%
%% image_string_ft_border
%%

image_string_ft_border (GD, ImageIndex, Color, BorderColor, BorderSize, Font, Angle, X, Y, Text) ->
  [image_string_ft (GD, ImageIndex, BorderColor, Font, Angle, X + DX, Y + DY, Text)
   || { DX, DY } <- [{ BorderSize, 0 }, { -BorderSize, 0 }, { 0, BorderSize }, { 0, -BorderSize }]],

  image_string_ft (GD, ImageIndex, Color, Font, Angle, X, Y, Text).

%%
%% image_copy
%%

image_copy (GD, ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, Width, Height) ->
  transaction (GD, ?GD_IMAGE_COPY, [ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, Width, Height]).

%%
%% image_copy_merge
%%

image_copy_merge (GD, ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, Width, Height, Percent) ->
  transaction (GD, ?GD_IMAGE_COPY_MERGE, [ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, Width, Height, Percent]).

%%
%% image_copy_merge_gray
%%

image_copy_merge_gray (GD, ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, Width, Height, Percent) ->
  transaction (GD, ?GD_IMAGE_COPY_MERGE_GRAY, [ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, Width, Height, Percent]).

%%
%% image_copy_resized
%%

image_copy_resized (GD, ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, DstWidth, DstHeight, SrcWidth, SrcHeight) ->
  transaction (GD, ?GD_IMAGE_COPY_RESIZED, [ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, DstWidth, DstHeight, SrcWidth, SrcHeight]).

%%
%% image_copy_resampled
%%

image_copy_resampled (GD, ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, DstWidth, DstHeight, SrcWidth, SrcHeight) ->
  transaction (GD, ?GD_IMAGE_COPY_RESAMPLED, [ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, DstWidth, DstHeight, SrcWidth, SrcHeight]).

%%
%% image_copy_rotated
%%

image_copy_rotated (GD, ImageIndexDst, ImageIndexSrc, DstCenterX, DstCenterY, SrcX, SrcY, SrcWidth, SrcHeight, Angle) ->
  transaction (GD, ?GD_IMAGE_COPY_ROTATED, [ImageIndexDst, ImageIndexSrc, float (DstCenterX), float (DstCenterY), SrcX, SrcY, SrcWidth, SrcHeight, Angle]).

%%
%% image_set_brush
%%

image_set_brush (GD, ImageIndex, BrushIndex) ->
  transaction (GD, ?GD_IMAGE_SET_BRUSH, [ImageIndex, BrushIndex]).

%%
%% image_set_tile
%%

image_set_tile (GD, ImageIndex, TileIndex) ->
  transaction (GD, ?GD_IMAGE_SET_TILE, [ImageIndex, TileIndex]).

%%
%% image_set_anti_aliased
%%

image_set_anti_aliased (GD, ImageIndex, Color) ->
  transaction (GD, ?GD_IMAGE_SET_ANTI_ALIASED, [ImageIndex, Color]).

%%
%% image_set_anti_aliased_dont_blend
%%

image_set_anti_aliased_dont_blend (GD, ImageIndex, Color, DontBlend) ->
  transaction (GD, ?GD_IMAGE_SET_ANTI_ALIASED_DONT_BLEND, [ImageIndex, Color, DontBlend]).

%%
%% image_set_style
%%

image_set_style (GD, ImageIndex, Style) ->
  transaction (GD, ?GD_IMAGE_SET_STYLE, [ImageIndex, Style]).

%%
%% image_set_thickness
%%

image_set_thickness (GD, ImageIndex, Thickness) ->
  transaction (GD, ?GD_IMAGE_SET_THICKNESS, [ImageIndex, Thickness]).

%%
%% image_interlace
%%

image_interlace (GD, ImageIndex, Interlace) ->
  transaction (GD, ?GD_IMAGE_INTERLACE, [ImageIndex, Interlace]).

%%
%% image_alpha_blending
%%

image_alpha_blending (GD, ImageIndex, AlphaBlending) ->
  transaction (GD, ?GD_IMAGE_ALPHA_BLENDING, [ImageIndex, AlphaBlending]).

%%
%% image_save_alpha
%%

image_save_alpha (GD, ImageIndex, SaveAlpha) ->
  transaction (GD, ?GD_IMAGE_SAVE_ALPHA, [ImageIndex, SaveAlpha]).

%%
%% image_create_palette_from_true_color
%%

image_create_palette_from_true_color (GD, ImageIndex, DitherFlag, ColorsWanted) ->
  transaction (GD, ?GD_IMAGE_CREATE_PALETTE_FROM_TRUE_COLOR, [ImageIndex, DitherFlag, ColorsWanted]).

%%
%% image_true_color_to_palette
%%

image_true_color_to_palette (GD, ImageIndex, DitherFlag, ColorsWanted) ->
  transaction (GD, ?GD_IMAGE_TRUE_COLOR_TO_PALETTE, [ImageIndex, DitherFlag, ColorsWanted]).

%%
%% image_color_transparent
%%

image_color_transparent (GD, ImageIndex, Transparent) ->
  transaction (GD, ?GD_IMAGE_COLOR_TRANSPARENT, [ImageIndex, Transparent]).

%%
%% image_palette_copy
%%

image_palette_copy (GD, DstImageIndex, SrcImageIndex) ->
  transaction (GD, ?GD_IMAGE_PALETTE_COPY, [DstImageIndex, SrcImageIndex]).

%%
%% image_true_color
%%

image_true_color (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_TRUECOLOR, [ImageIndex]).

%%
%% image_sx
%%

image_sx (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_SX, [ImageIndex]).

%%
%% image_sy
%%

image_sy (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_SY, [ImageIndex]).

%%
%% image_colors_total
%%

image_colors_total (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_COLORS_TOTAL, [ImageIndex]).

%%
%% image_red
%%

image_red (GD, ImageIndex, Color) ->
  transaction (GD, ?GD_IMAGE_RED, [ImageIndex, Color]).

%%
%% image_green
%%

image_green (GD, ImageIndex, Color) ->
  transaction (GD, ?GD_IMAGE_GREEN, [ImageIndex, Color]).

%%
%% image_blue
%%

image_blue (GD, ImageIndex, Color) ->
  transaction (GD, ?GD_IMAGE_BLUE, [ImageIndex, Color]).

%%
%% image_alpha
%%

image_alpha (GD, ImageIndex, Color) ->
  transaction (GD, ?GD_IMAGE_ALPHA, [ImageIndex, Color]).

%%
%% image_get_transparent
%%

image_get_transparent (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_GET_TRANSPARENT, [ImageIndex]).

%%
%% image_get_interlaced
%%

image_get_interlaced (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_GET_INTERLACED, [ImageIndex]).

%%
%% image_get_palette
%%

image_get_palette (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_GET_PALETTE, [ImageIndex]).

%%
%% image_get_row_indexed
%%

image_get_row_indexed (GD, ImageIndex, Row) ->
  transaction (GD, ?GD_IMAGE_GET_ROW_INDEXED, [ImageIndex, Row]).

%%
%% image_get_row_indexed_rle
%%

image_get_row_indexed_rle (GD, ImageIndex, Row) ->
  transaction (GD, ?GD_IMAGE_GET_ROW_INDEXED_RLE, [ImageIndex, Row]).

%%
%% image_get_row_truecolor
%%

image_get_row_truecolor (GD, ImageIndex, Row) ->
  transaction (GD, ?GD_IMAGE_GET_ROW_TRUECOLOR, [ImageIndex, Row]).

%%
%% image_get_indexed_rle
%%

image_get_indexed_rle (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_GET_INDEXED_RLE, [ImageIndex]).

%%
%% text_size
%%

text_size (GD, Font, String) ->
  { ok, Bounds } = image_string_ft (GD, -1, 0, Font, 0.0, 0, 0, String),
  NBounds = gd_bounds:normalize (Bounds),
  Width = gd_bounds:get_lr_x (NBounds),
  Height = gd_bounds:get_lr_y (NBounds),
  { ok, Width, Height }.

%%
%% text_width
%%

text_width (GD, Font, String) ->
  { ok, Width, _Height } = text_size (GD, Font, String),
  { ok, Width }.

%%
%% text_height
%%

text_height (GD, Font, String) ->
  { ok, _Width, Height } = text_size (GD, Font, String),
  { ok, Height }.

%%
%% font_metrics
%%

font_metrics (GD, Font) ->
  { ok, _, XHeight } = text_size (GD, Font, "x"),
  { ok, _, Ascent } = text_size (GD, Font, "bdfhiklt"),
  { ok, _, Descent0 } = text_size (GD, Font, "gpqy"),
  Descent = Descent0 - XHeight,
  { ok, _, CapHeight } = text_size (GD, Font, "E"),
  { ok, _, L1 } = text_size (GD, Font, "Ebdfhikltgpqy"),
  { ok, _, L2 } = text_size (GD, Font, "Ebdfhikltgpqy\nEbdfhikltgpqy"),
  LineHeight = L2 - L1,
  gd_font_metrics:factory (XHeight, Ascent, Descent, CapHeight, LineHeight).

%%
%% get_face
%%

get_face (GD, Font) when is_record (Font, gd_font) ->
  get_face (GD, gd_font:get_font_path (Font));

get_face (GD, FontPath) ->
  { ok, Face } = transaction (GD, ?GD_GET_FACE, [FontPath]),
  Face.

%%
%% get_font_root
%%

get_font_root () ->
  { ok, FontRoot } = application:get_env (elib_gd, font_root),
  FontRoot.

%%====================================================================
%% Internal functions
%%====================================================================

%%
%% transaction
%%

transaction (#gd{ port = Port }, Command, ParameterList) ->
  port_command (Port, term_to_binary (list_to_tuple ([Command | ParameterList]))),
  receive
    { Port, Reply } ->
      Reply
  end.
