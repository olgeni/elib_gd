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
-include ("gd_face.hrl").
-include ("gd_font.hrl").
-include ("gd_font_metrics.hrl").

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
%% Type exports
%%====================================================================

%%====================================================================
%% Types
%%====================================================================

-type gd () :: #gd{ port :: port () }.

-type style () :: [?GD_ARC | ?GD_CHORD | ?GD_NOFILL | ?GD_EDGED].

-type point () :: { non_neg_integer (), non_neg_integer () }.

%%====================================================================
%% External functions
%%====================================================================

%%
%% new
%%

-spec new () ->
  { 'ok', gd () }.

new () ->
  GD = #gd{ port = open_port ({ spawn, "gd" }, [binary]) },
  { ok, GD }.

%%
%% stop
%%

-spec stop (GD :: gd ()) ->
  'ok'.

stop (#gd{ port = Port }) when is_port (Port) ->
  unlink (Port),
  port_close (Port),
  ok.

%%
%% image_create
%%

-spec image_create (GD :: gd (), Width :: integer (), Height :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_create (GD, Width, Height) ->
  transaction (GD, ?GD_IMAGE_CREATE, [Width, Height]).

%%
%% image_create_true_color
%%

-spec image_create_true_color (GD :: gd (), Width :: integer (), Height :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_create_true_color (GD, Width, Height) ->
  transaction (GD, ?GD_IMAGE_CREATE_TRUE_COLOR, [Width, Height]).

%%
%% image_destroy
%%

-spec image_destroy (GD :: gd (), ImageIndex :: integer ()) ->
  'ok'.

image_destroy (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_DESTROY, [ImageIndex]).

%%
%% image_set_pixel
%%

-spec image_set_pixel (GD :: gd (), ImageIndex :: integer (), X :: integer (), Y :: integer (), Color :: integer ()) ->
  'ok'.

image_set_pixel (GD, ImageIndex, X, Y, Color) ->
  transaction (GD, ?GD_IMAGE_SET_PIXEL, [ImageIndex, X, Y, Color]).

%%
%% image_get_pixel
%%

-spec image_get_pixel (GD :: gd (), ImageIndex :: integer (), X :: integer (), Y :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_get_pixel (GD, ImageIndex, X, Y) ->
  transaction (GD, ?GD_IMAGE_GET_PIXEL, [ImageIndex, X, Y]).

%%
%% image_color_allocate
%%

-spec image_color_allocate (GD :: gd (), ImageIndex :: integer (), Red :: integer (), Green :: integer (), Blue :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_color_allocate (GD, ImageIndex, Red, Green, Blue) ->
  transaction (GD, ?GD_IMAGE_COLOR_ALLOCATE, [ImageIndex, Red, Green, Blue]).

%%
%% image_color_allocate_alpha
%%

-spec image_color_allocate_alpha (GD :: gd (), ImageIndex :: integer (), Red :: integer (), Green :: integer (), Blue :: integer (), Alpha :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_color_allocate_alpha (GD, ImageIndex, Red, Green, Blue, Alpha) ->
  transaction (GD, ?GD_IMAGE_COLOR_ALLOCATE_ALPHA, [ImageIndex, Red, Green, Blue, Alpha]).

%%
%% image_color_closest
%%

-spec image_color_closest (GD :: gd (), ImageIndex :: integer (), Red :: integer (), Green :: integer (), Blue :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_color_closest (GD, ImageIndex, Red, Green, Blue) ->
  transaction (GD, ?GD_IMAGE_COLOR_CLOSEST, [ImageIndex, Red, Green, Blue]).

%%
%% image_color_closest_alpha
%%

-spec image_color_closest_alpha (GD :: gd (), ImageIndex :: integer (), Red :: integer (), Green :: integer (), Blue :: integer (), Alpha :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_color_closest_alpha (GD, ImageIndex, Red, Green, Blue, Alpha) ->
  transaction (GD, ?GD_IMAGE_COLOR_CLOSEST_ALPHA, [ImageIndex, Red, Green, Blue, Alpha]).

%%
%% image_color_closest_hwb
%%

-spec image_color_closest_hwb (GD :: gd (), ImageIndex :: integer (), Red :: integer (), Green :: integer (), Blue :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_color_closest_hwb (GD, ImageIndex, Red, Green, Blue) ->
  transaction (GD, ?GD_IMAGE_COLOR_CLOSEST_HWB, [ImageIndex, Red, Green, Blue]).

%%
%% image_color_exact
%%

-spec image_color_exact (GD :: gd (), ImageIndex :: integer (), Red :: integer (), Green :: integer (), Blue :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_color_exact (GD, ImageIndex, Red, Green, Blue) ->
  transaction (GD, ?GD_IMAGE_COLOR_EXACT, [ImageIndex, Red, Green, Blue]).

%%
%% image_color_exact_alpha
%%

-spec image_color_exact_alpha (GD :: gd (), ImageIndex :: integer (), Red :: integer (), Green :: integer (), Blue :: integer (), Alpha :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_color_exact_alpha (GD, ImageIndex, Red, Green, Blue, Alpha) ->
  transaction (GD, ?GD_IMAGE_COLOR_EXACT_ALPHA, [ImageIndex, Red, Green, Blue, Alpha]).

%%
%% image_color_resolve
%%

-spec image_color_resolve (GD :: gd (), ImageIndex :: integer (), Red :: integer (), Green :: integer (), Blue :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_color_resolve (GD, ImageIndex, Red, Green, Blue) ->
  transaction (GD, ?GD_IMAGE_COLOR_RESOLVE, [ImageIndex, Red, Green, Blue]).

%%
%% image_color_resolve_alpha
%%

-spec image_color_resolve_alpha (GD :: gd (), ImageIndex :: integer (), Red :: integer (), Green :: integer (), Blue :: integer (), Alpha :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_color_resolve_alpha (GD, ImageIndex, Red, Green, Blue, Alpha) ->
  transaction (GD, ?GD_IMAGE_COLOR_RESOLVE_ALPHA, [ImageIndex, Red, Green, Blue, Alpha]).

%%
%% image_color_deallocate
%%

-spec image_color_deallocate (GD :: gd (), ImageIndex :: integer (), Color :: integer ()) ->
  'ok'.

image_color_deallocate (GD, ImageIndex, Color) ->
  transaction (GD, ?GD_IMAGE_COLOR_DEALLOCATE, [ImageIndex, Color]).

%%
%% image_create_from_png_ptr
%%

-spec image_create_from_png_ptr (GD :: gd (), Binary :: binary ()) ->
  { 'ok', non_neg_integer () }.

image_create_from_png_ptr (GD, Binary) ->
  transaction (GD, ?GD_IMAGE_CREATE_FROM_PNG_PTR, [Binary]).

%%
%% image_create_from_jpeg_ptr
%%

-spec image_create_from_jpeg_ptr (GD :: gd (), Binary :: binary ()) ->
  { 'ok', non_neg_integer () }.

image_create_from_jpeg_ptr (GD, Binary) ->
  transaction (GD, ?GD_IMAGE_CREATE_FROM_JPEG_PTR, [Binary]).

%%
%% image_create_from_gif_ptr
%%

-spec image_create_from_gif_ptr (GD :: gd (), Binary :: binary ()) ->
  { 'ok', non_neg_integer () }.

image_create_from_gif_ptr (GD, Binary) ->
  transaction (GD, ?GD_IMAGE_CREATE_FROM_GIF_PTR, [Binary]).

%%
%% image_png_ptr
%%

-spec image_png_ptr (GD :: gd (), ImageIndex :: integer ()) ->
  { 'ok', binary () }.

image_png_ptr (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_PNG_PTR, [ImageIndex]).

%%
%% image_png_ptr
%%

-spec image_png_ptr_ex (GD :: gd (), ImageIndex :: integer (), Level :: integer ()) ->
  { 'ok', binary () }.

image_png_ptr_ex (GD, ImageIndex, Level) ->
  transaction (GD, ?GD_IMAGE_PNG_PTR, [ImageIndex, Level]).

%%
%% image_jpeg_ptr
%%

-spec image_jpeg_ptr (GD :: gd (), ImageIndex :: integer (), Quality :: integer ()) ->
  { 'ok', binary () }.

image_jpeg_ptr (GD, ImageIndex, Quality) ->
  transaction (GD, ?GD_IMAGE_JPEG_PTR, [ImageIndex, Quality]).

%%
%% image_gif_ptr
%%

-spec image_gif_ptr (GD :: gd (), ImageIndex :: integer ()) ->
  { 'ok', binary () }.

image_gif_ptr (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_GIF_PTR, [ImageIndex]).

%%
%% image_set_clip
%%

-spec image_set_clip (GD :: gd (), ImageIndex :: integer (), X1 :: integer (), Y1 :: integer (), X2 :: integer (), Y2 :: integer ()) ->
  'ok'.

image_set_clip (GD, ImageIndex, X1, Y1, X2, Y2) ->
  transaction (GD, ?GD_IMAGE_SET_CLIP, [ImageIndex, X1, Y1, X2, Y2]).

%%
%% image_get_clip
%%

-spec image_get_clip (GD :: gd (), ImageIndex :: integer ()) ->
  { 'ok', { 'gd_clip', non_neg_integer (), non_neg_integer (), non_neg_integer (), non_neg_integer () } }.

image_get_clip (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_GET_CLIP, [ImageIndex]).

%%
%% image_bounds_safe
%%

-spec image_bounds_safe (GD :: gd (), ImageIndex :: integer (), X :: integer (), Y :: integer ()) ->
  { 'ok', boolean () }.

image_bounds_safe (GD, ImageIndex, X, Y) ->
  transaction (GD, ?GD_IMAGE_BOUNDS_SAFE, [ImageIndex, X, Y]).

%%
%% image_line
%%

-spec image_line (GD :: gd (), ImageIndex :: integer (), X1 :: integer (), Y1 :: integer (), X2 :: integer (), Y2 :: integer (), Color :: integer ()) ->
  'ok'.

image_line (GD, ImageIndex, X1, Y1, X2, Y2, Color) ->
  transaction (GD, ?GD_IMAGE_LINE, [ImageIndex, X1, Y1, X2, Y2, Color]).

%%
%% image_rectangle
%%

-spec image_rectangle (GD :: gd (), ImageIndex :: integer (), X1 :: integer (), Y1 :: integer (), X2 :: integer (), Y2 :: integer (), Color :: integer ()) ->
  'ok'.

image_rectangle (GD, ImageIndex, X1, Y1, X2, Y2, Color) ->
  transaction (GD, ?GD_IMAGE_RECTANGLE, [ImageIndex, X1, Y1, X2, Y2, Color]).

%%
%% image_filled_rectangle
%%

-spec image_filled_rectangle (GD :: gd (), ImageIndex :: integer (), X1 :: integer (), Y1 :: integer (), X2 :: integer (), Y2 :: integer (), Color :: integer ()) ->
  'ok'.

image_filled_rectangle (GD, ImageIndex, X1, Y1, X2, Y2, Color) ->
  transaction (GD, ?GD_IMAGE_FILLED_RECTANGLE, [ImageIndex, X1, Y1, X2, Y2, Color]).

%%
%% image_ellipse
%%

-spec image_ellipse (GD :: gd (), ImageIndex :: integer (), CenterX :: integer (), CenterY :: integer (), Width :: integer (), Height :: integer (), Color :: integer ()) ->
  'ok'.

image_ellipse (GD, ImageIndex, CenterX, CenterY, Width, Height, Color) ->
  transaction (GD, ?GD_IMAGE_ELLIPSE, [ImageIndex, CenterX, CenterY, Width, Height, Color]).

%%
%% image_filled_ellipse
%%

-spec image_filled_ellipse (GD :: gd (), ImageIndex :: integer (), CenterX :: integer (), CenterY :: integer (), Width :: integer (), Height :: integer (), Color :: integer ()) ->
  'ok'.

image_filled_ellipse (GD, ImageIndex, CenterX, CenterY, Width, Height, Color) ->
  transaction (GD, ?GD_IMAGE_FILLED_ELLIPSE, [ImageIndex, CenterX, CenterY, Width, Height, Color]).

%%
%% image_arc
%%

-spec image_arc (GD :: gd (), ImageIndex :: integer (), CenterX :: integer (), CenterY :: integer (), Width :: integer (), Height :: integer (), InitialDegree :: integer (), FinalDegree :: integer (), Color :: integer ()) ->
  'ok'.

image_arc (GD, ImageIndex, CenterX, CenterY, Width, Height, InitialDegree, FinalDegree, Color) ->
  transaction (GD, ?GD_IMAGE_ARC, [ImageIndex, CenterX, CenterY, Width, Height, InitialDegree, FinalDegree, Color]).

%%
%% image_filled_arc
%%

-spec image_filled_arc (GD :: gd (), ImageIndex :: integer (), CenterX :: integer (), CenterY :: integer (), Width :: integer (), Height :: integer (), InitialDegree :: integer (), FinalDegree :: integer (), Color :: integer (), Style :: style ()) ->
  'ok'.

image_filled_arc (GD, ImageIndex, CenterX, CenterY, Width, Height, InitialDegree, FinalDegree, Color, Style) ->
  StyleValue =
    lists:foldl (
      fun (X, A) ->
          fun (?GD_ARC)    -> ?GD_ARC_CONST;
              (?GD_CHORD)  -> ?GD_CHORD_CONST;
              (?GD_NOFILL) -> ?GD_NOFILL_CONST;
              (?GD_EDGED)  -> ?GD_EDGED_CONST
          end (X) bor A
      end, 0, Style),

  transaction (GD, ?GD_IMAGE_FILLED_ARC, [ImageIndex, CenterX, CenterY, Width, Height, InitialDegree, FinalDegree, Color, StyleValue]).

%%
%% image_polygon
%%

-spec image_polygon (GD :: gd (), ImageIndex :: integer (), Points :: [point (), ...], Color :: integer ()) ->
  'ok'.

image_polygon (GD, ImageIndex, Points, Color) when is_list (Points) ->
  transaction (GD, ?GD_IMAGE_POLYGON, [ImageIndex, Points, Color]).

%%
%% image_open_polygon
%%

-spec image_open_polygon (GD :: gd (), ImageIndex :: integer (), Points :: [point (), ...], Color :: integer ()) ->
  'ok'.

image_open_polygon (GD, ImageIndex, Points, Color) when is_list (Points) ->
  transaction (GD, ?GD_IMAGE_OPEN_POLYGON, [ImageIndex, Points, Color]).

%%
%% image_filled_polygon
%%

-spec image_filled_polygon (GD :: gd (), ImageIndex :: integer (), Points :: [point (), ...], Color :: integer ()) ->
  'ok'.

image_filled_polygon (GD, ImageIndex, Points, Color) when is_list (Points) ->
  transaction (GD, ?GD_IMAGE_FILLED_POLYGON, [ImageIndex, Points, Color]).

%%
%% image_fill
%%

-spec image_fill (GD :: gd (), ImageIndex :: integer (), X :: integer (), Y :: integer (), Color :: integer ()) ->
  'ok'.

image_fill (GD, ImageIndex, X, Y, Color) ->
  transaction (GD, ?GD_IMAGE_FILL, [ImageIndex, X, Y, Color]).

%%
%% image_fill_to_border
%%

-spec image_fill_to_border (GD :: gd (), ImageIndex :: integer (), X :: integer (), Y :: integer (), BorderColor :: integer (), Color :: integer ()) ->
  'ok'.

image_fill_to_border (GD, ImageIndex, X, Y, BorderColor, Color) ->
  transaction (GD, ?GD_IMAGE_FILL_TO_BORDER, [ImageIndex, X, Y, BorderColor, Color]).

%%
%% image_string_ft
%%

-spec image_string_ft (GD :: gd (), ImageIndex :: integer (), Color :: integer (), Font :: gd_font:gd_font (), Angle :: number (), X :: integer (), Y :: integer (), String :: string ()) ->
  { 'ok', gd_bounds:gd_bounds () } | { 'error', string () }.

image_string_ft (GD, ImageIndex, Color, Font, Angle, X, Y, String) ->
  transaction (
    GD, ?GD_IMAGE_STRING_FT,
    [ImageIndex, Color,
     gd_font:get_font_path (Font),
     float (gd_font:get_point_size (Font)),
     float (gd_font:get_line_spacing (Font)),
     float (Angle), X, Y, String]).

%%
%% image_string_ft_border
%%

-spec image_string_ft_border (GD :: gd (), ImageIndex :: integer (), Color :: integer (), BorderColor :: integer (), BorderSize :: non_neg_integer (), Font :: gd_font:gd_font (), Angle :: number (), X :: integer (), Y :: integer (), String :: string ()) ->
  { 'ok', gd_bounds:gd_bounds () } | { 'error', string () }.

image_string_ft_border (GD, ImageIndex, Color, BorderColor, BorderSize, Font, Angle, X, Y, String) ->
  [image_string_ft (GD, ImageIndex, BorderColor, Font, Angle, X + DX, Y + DY, String)
   || { DX, DY } <- [{ BorderSize, 0 }, { -BorderSize, 0 }, { 0, BorderSize }, { 0, -BorderSize }]],

  image_string_ft (GD, ImageIndex, Color, Font, Angle, X, Y, String).

%%
%% image_copy
%%

-spec image_copy (GD :: gd (), ImageIndexDst :: integer (), ImageIndexSrc :: integer (), DstX :: integer (), DstY :: integer (), SrcX :: integer (), SrcY :: integer (), Width :: integer (), Height :: integer ()) ->
  'ok'.

image_copy (GD, ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, Width, Height) ->
  transaction (GD, ?GD_IMAGE_COPY, [ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, Width, Height]).

%%
%% image_copy_merge
%%

-spec image_copy_merge (GD :: gd (), ImageIndexDst :: integer (), ImageIndexSrc :: integer (), DstX :: integer (), DstY :: integer (), SrcX :: integer (), SrcY :: integer (), Width :: integer (), Height :: integer (), Percent :: integer ()) ->
  'ok'.

image_copy_merge (GD, ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, Width, Height, Percent) ->
  transaction (GD, ?GD_IMAGE_COPY_MERGE, [ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, Width, Height, Percent]).

%%
%% image_copy_merge_gray
%%

-spec image_copy_merge_gray (GD :: gd (), ImageIndexDst :: integer (), ImageIndexSrc :: integer (), DstX :: integer (), DstY :: integer (), SrcX :: integer (), SrcY :: integer (), Width :: integer (), Height :: integer (), Percent :: integer ()) ->
  'ok'.

image_copy_merge_gray (GD, ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, Width, Height, Percent) ->
  transaction (GD, ?GD_IMAGE_COPY_MERGE_GRAY, [ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, Width, Height, Percent]).

%%
%% image_copy_resized
%%

-spec image_copy_resized (GD :: gd (), ImageIndexDst :: integer (), ImageIndexSrc :: integer (), DstX :: integer (), DstY :: integer (), SrcX :: integer (), SrcY :: integer (), DstWidth :: integer (), DstHeight :: integer (), SrcWidth :: integer (), SrcHeight :: integer ()) ->
  'ok'.

image_copy_resized (GD, ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, DstWidth, DstHeight, SrcWidth, SrcHeight) ->
  transaction (GD, ?GD_IMAGE_COPY_RESIZED, [ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, DstWidth, DstHeight, SrcWidth, SrcHeight]).

%%
%% image_copy_resampled
%%

-spec image_copy_resampled (GD :: gd (), ImageIndexDst :: integer (), ImageIndexSrc :: integer (), DstX :: integer (), DstY :: integer (), SrcX :: integer (), SrcY :: integer (), DstWidth :: integer (), DstHeight :: integer (), SrcWidth :: integer (), SrcHeight :: integer ()) ->
  'ok'.

image_copy_resampled (GD, ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, DstWidth, DstHeight, SrcWidth, SrcHeight) ->
  transaction (GD, ?GD_IMAGE_COPY_RESAMPLED, [ImageIndexDst, ImageIndexSrc, DstX, DstY, SrcX, SrcY, DstWidth, DstHeight, SrcWidth, SrcHeight]).

%%
%% image_copy_rotated
%%

-spec image_copy_rotated (GD :: gd (), ImageIndexDst :: integer (), ImageIndexSrc :: integer (), DstCenterX :: number (), DstCenterY :: number (), SrcX :: integer (), SrcY :: integer (), SrcWidth :: integer (), SrcHeight :: integer (), Angle :: integer ()) ->
  'ok'.

image_copy_rotated (GD, ImageIndexDst, ImageIndexSrc, DstCenterX, DstCenterY, SrcX, SrcY, SrcWidth, SrcHeight, Angle) ->
  transaction (GD, ?GD_IMAGE_COPY_ROTATED, [ImageIndexDst, ImageIndexSrc, float (DstCenterX), float (DstCenterY), SrcX, SrcY, SrcWidth, SrcHeight, Angle]).

%%
%% image_set_brush
%%

-spec image_set_brush (GD :: gd (), ImageIndex :: integer (), BrushIndex :: integer ()) ->
  'ok'.

image_set_brush (GD, ImageIndex, BrushIndex) ->
  transaction (GD, ?GD_IMAGE_SET_BRUSH, [ImageIndex, BrushIndex]).

%%
%% image_set_tile
%%

-spec image_set_tile (GD :: gd (), ImageIndex :: integer (), TileIndex :: integer ()) ->
  'ok'.

image_set_tile (GD, ImageIndex, TileIndex) ->
  transaction (GD, ?GD_IMAGE_SET_TILE, [ImageIndex, TileIndex]).

%%
%% image_set_anti_aliased
%%

-spec image_set_anti_aliased (GD :: gd (), ImageIndex :: integer (), Color :: integer ()) ->
  'ok'.

image_set_anti_aliased (GD, ImageIndex, Color) ->
  transaction (GD, ?GD_IMAGE_SET_ANTI_ALIASED, [ImageIndex, Color]).

%%
%% image_set_anti_aliased_dont_blend
%%

-spec image_set_anti_aliased_dont_blend (GD :: gd (), ImageIndex :: integer (), Color :: integer (), DontBlend :: integer ()) ->
  'ok'.

image_set_anti_aliased_dont_blend (GD, ImageIndex, Color, DontBlend) ->
  transaction (GD, ?GD_IMAGE_SET_ANTI_ALIASED_DONT_BLEND, [ImageIndex, Color, DontBlend]).

%%
%% image_set_style
%%

-spec image_set_style (GD :: gd (), ImageIndex :: integer (), Style :: integer ()) ->
  'ok'.

image_set_style (GD, ImageIndex, Style) ->
  transaction (GD, ?GD_IMAGE_SET_STYLE, [ImageIndex, Style]).

%%
%% image_set_thickness
%%

-spec image_set_thickness (GD :: gd (), ImageIndex :: integer (), Thickness :: integer ()) ->
  'ok'.

image_set_thickness (GD, ImageIndex, Thickness) ->
  transaction (GD, ?GD_IMAGE_SET_THICKNESS, [ImageIndex, Thickness]).

%%
%% image_interlace
%%

-spec image_interlace (GD :: gd (), ImageIndex :: integer (), Interlace :: integer ()) ->
  'ok'.

image_interlace (GD, ImageIndex, Interlace) ->
  transaction (GD, ?GD_IMAGE_INTERLACE, [ImageIndex, Interlace]).

%%
%% image_alpha_blending
%%

-spec image_alpha_blending (GD :: gd (), ImageIndex :: integer (), AlphaBlending :: integer ()) ->
  'ok'.

image_alpha_blending (GD, ImageIndex, AlphaBlending) ->
  transaction (GD, ?GD_IMAGE_ALPHA_BLENDING, [ImageIndex, AlphaBlending]).

%%
%% image_save_alpha
%%

-spec image_save_alpha (GD :: gd (), ImageIndex :: integer (), SaveAlpha :: integer ()) ->
  'ok'.

image_save_alpha (GD, ImageIndex, SaveAlpha) ->
  transaction (GD, ?GD_IMAGE_SAVE_ALPHA, [ImageIndex, SaveAlpha]).

%%
%% image_create_palette_from_true_color
%%

-spec image_create_palette_from_true_color (GD :: gd (), ImageIndex :: integer (), DitherFlag :: integer (), ColorsWanted :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_create_palette_from_true_color (GD, ImageIndex, DitherFlag, ColorsWanted) ->
  transaction (GD, ?GD_IMAGE_CREATE_PALETTE_FROM_TRUE_COLOR, [ImageIndex, DitherFlag, ColorsWanted]).

%%
%% image_true_color_to_palette
%%

-spec image_true_color_to_palette (GD :: gd (), ImageIndex :: integer (), DitherFlag :: integer (), ColorsWanted :: integer ()) ->
  'ok'.

image_true_color_to_palette (GD, ImageIndex, DitherFlag, ColorsWanted) ->
  transaction (GD, ?GD_IMAGE_TRUE_COLOR_TO_PALETTE, [ImageIndex, DitherFlag, ColorsWanted]).

%%
%% image_color_transparent
%%

-spec image_color_transparent (GD :: gd (), ImageIndex :: integer (), Transparent :: integer ()) ->
  'ok'.

image_color_transparent (GD, ImageIndex, Transparent) ->
  transaction (GD, ?GD_IMAGE_COLOR_TRANSPARENT, [ImageIndex, Transparent]).

%%
%% image_palette_copy
%%

-spec image_palette_copy (GD :: gd (), DstImageIndex :: integer (), SrcImageIndex :: integer ()) ->
  'ok'.

image_palette_copy (GD, DstImageIndex, SrcImageIndex) ->
  transaction (GD, ?GD_IMAGE_PALETTE_COPY, [DstImageIndex, SrcImageIndex]).

%%
%% image_true_color
%%

-spec image_true_color (GD :: gd (), ImageIndex :: integer ()) ->
  { 'ok', boolean () }.

image_true_color (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_TRUECOLOR, [ImageIndex]).

%%
%% image_sx
%%

-spec image_sx (GD :: gd (), ImageIndex :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_sx (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_SX, [ImageIndex]).

%%
%% image_sy
%%

-spec image_sy (GD :: gd (), ImageIndex :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_sy (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_SY, [ImageIndex]).

%%
%% image_colors_total
%%

-spec image_colors_total (GD :: gd (), ImageIndex :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_colors_total (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_COLORS_TOTAL, [ImageIndex]).

%%
%% image_red
%%

-spec image_red (GD :: gd (), ImageIndex :: integer (), Color :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_red (GD, ImageIndex, Color) ->
  transaction (GD, ?GD_IMAGE_RED, [ImageIndex, Color]).

%%
%% image_green
%%

-spec image_green (GD :: gd (), ImageIndex :: integer (), Color :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_green (GD, ImageIndex, Color) ->
  transaction (GD, ?GD_IMAGE_GREEN, [ImageIndex, Color]).

%%
%% image_blue
%%

-spec image_blue (GD :: gd (), ImageIndex :: integer (), Color :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_blue (GD, ImageIndex, Color) ->
  transaction (GD, ?GD_IMAGE_BLUE, [ImageIndex, Color]).

%%
%% image_alpha
%%

-spec image_alpha (GD :: gd (), ImageIndex :: integer (), Color :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_alpha (GD, ImageIndex, Color) ->
  transaction (GD, ?GD_IMAGE_ALPHA, [ImageIndex, Color]).

%%
%% image_get_transparent
%%

-spec image_get_transparent (GD :: gd (), ImageIndex :: integer ()) ->
  { 'ok', non_neg_integer () }.

image_get_transparent (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_GET_TRANSPARENT, [ImageIndex]).

%%
%% image_get_interlaced
%%

-spec image_get_interlaced (GD :: gd (), ImageIndex :: integer ()) ->
  { 'ok', boolean () }.

image_get_interlaced (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_GET_INTERLACED, [ImageIndex]).

%%
%% image_get_palette
%%

-spec image_get_palette (GD :: gd (), ImageIndex :: integer ()) ->
  any ().

image_get_palette (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_GET_PALETTE, [ImageIndex]).

%%
%% image_get_row_indexed
%%

-spec image_get_row_indexed (GD :: gd (), ImageIndex :: integer (), Row :: integer ()) ->
  any ().

image_get_row_indexed (GD, ImageIndex, Row) ->
  transaction (GD, ?GD_IMAGE_GET_ROW_INDEXED, [ImageIndex, Row]).

%%
%% image_get_row_indexed_rle
%%

-spec image_get_row_indexed_rle (GD :: gd (), ImageIndex :: integer (), Row :: integer ()) ->
  any ().

image_get_row_indexed_rle (GD, ImageIndex, Row) ->
  transaction (GD, ?GD_IMAGE_GET_ROW_INDEXED_RLE, [ImageIndex, Row]).

%%
%% image_get_row_truecolor
%%

-spec image_get_row_truecolor (GD :: gd (), ImageIndex :: integer (), Row :: integer ()) ->
  any ().

image_get_row_truecolor (GD, ImageIndex, Row) ->
  transaction (GD, ?GD_IMAGE_GET_ROW_TRUECOLOR, [ImageIndex, Row]).

%%
%% image_get_indexed_rle
%%

-spec image_get_indexed_rle (GD :: gd (), ImageIndex :: integer ()) ->
  any ().

image_get_indexed_rle (GD, ImageIndex) ->
  transaction (GD, ?GD_IMAGE_GET_INDEXED_RLE, [ImageIndex]).

%%
%% text_size
%%

-spec text_size (GD :: gd (), Font :: gd_font:gd_font (), String :: string ()) ->
  { 'ok', non_neg_integer (), non_neg_integer () }.

text_size (GD, Font, String) ->
  { ok, Bounds } = image_string_ft (GD, -1, 0, Font, 0.0, 0, 0, String),
  NBounds = gd_bounds:normalize (Bounds),
  Width = gd_bounds:get_lr_x (NBounds),
  Height = gd_bounds:get_lr_y (NBounds),
  { ok, Width, Height }.

%%
%% text_width
%%

-spec text_width (GD :: gd (), Font :: gd_font:gd_font (), String :: string ()) ->
  { 'ok', non_neg_integer () }.

text_width (GD, Font, String) ->
  { ok, Width, _Height } = text_size (GD, Font, String),
  { ok, Width }.

%%
%% text_height
%%

-spec text_height (GD :: gd (), Font :: gd_font:gd_font (), String :: string ()) ->
  { 'ok', non_neg_integer () }.

text_height (GD, Font, String) ->
  { ok, _Width, Height } = text_size (GD, Font, String),
  { ok, Height }.

%%
%% font_metrics
%%

-spec font_metrics (GD :: gd (), Font :: gd_font:gd_font ()) ->
  gd_font_metrics:gd_font_metrics ().

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

-spec get_face (GD :: gd (), Font :: string () | gd_font:gd_font ()) ->
  { 'ok', gd_face:gd_face () }.

get_face (GD, Font) when is_record (Font, gd_font) ->
  get_face (GD, gd_font:get_font_path (Font));

get_face (GD, Font) when is_list (Font) ->
  { ok, Face } = transaction (GD, ?GD_GET_FACE, [Font]),
  Face.

%%
%% get_font_root
%%

-spec get_font_root () ->
  string ().

get_font_root () ->
  { ok, FontRoot } = application:get_env (elib_gd, font_root),
  FontRoot.

%%====================================================================
%% Internal functions
%%====================================================================

%%
%% transaction
%%

-spec transaction (GD :: gd (), Command :: non_neg_integer (), Parameters :: [any ()]) ->
  any ().

transaction (#gd{ port = Port }, Command, Parameters) when is_port (Port) ->
  port_command (Port, term_to_binary (list_to_tuple ([Command | Parameters]))),
  receive
    { Port, Reply } ->
      Reply
  end.
