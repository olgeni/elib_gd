-module (gd_font).

%% === Include files =================================================

-include ("gd_font.hrl").

%% === External exports ==============================================

-export ([new/0,
          factory/2,
          factory/3,
          set_font_path/2,
          get_font_path/1,
          set_point_size/2,
          get_point_size/1,
          set_line_spacing/2,
          get_line_spacing/1]).

%% === Internal exports ==============================================

%% === Macros ========================================================

-define (DEFAULT_LINE_SPACING, 1.05).

%% === Records =======================================================

%% === Type exports ==================================================

-export_type ([gd_font/0]).

%% === Types =========================================================

-type gd_font () :: #gd_font{ font_path :: string (),
			      point_size :: float (),
			      line_spacing :: float () }.

%% === Type exports ==================================================

%% === Types =========================================================

%% === External functions ============================================

%%
%% new
%%

new () ->
  #gd_font{}.

%%
%% factory
%%

factory (FontPath, PointSize) ->
  factory (FontPath, PointSize, ?DEFAULT_LINE_SPACING).

factory (FontPath, PointSize, LineSpacing) ->
  #gd_font{ font_path = FontPath,
            point_size = PointSize,
            line_spacing = LineSpacing }.

%%
%% set_font_path
%%

set_font_path (Font, FontPath) ->
  Font#gd_font{ font_path = FontPath }.

%%
%% get_font_path
%%

get_font_path (#gd_font{ font_path = FontPath }) ->
  FontPath.

%%
%% set_point_size
%%

set_point_size (Font, PointSize) ->
  Font#gd_font{ point_size = PointSize }.

%%
%% get_point_size
%%

get_point_size (#gd_font{ point_size = PointSize }) ->
  PointSize.

%%
%% set_line_spacing
%%

set_line_spacing (Font, LineSpacing) ->
  Font#gd_font{ line_spacing = LineSpacing }.

%%
%% get_line_spacing
%%

get_line_spacing (#gd_font{ line_spacing = LineSpacing }) ->
  LineSpacing.

%% === Internal functions ============================================
