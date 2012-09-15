%%--------------------------------------------------------------------
%% $Id$
%%--------------------------------------------------------------------

-module (gd_font_metrics).

-vsn ("$Id$").

%% === Include files =================================================

-include ("gd_font_metrics.hrl").

%% === External exports ==============================================

-export ([new/0,
          factory/5,
          set_x_height/2,
          get_x_height/1,
          set_ascent/2,
          get_ascent/1,
          set_descent/2,
          get_descent/1,
          set_cap_height/2,
          get_cap_height/1,
          set_line_height/2,
          get_line_height/1]).

%% === Internal exports ==============================================

%% === Macros ========================================================

%% === Records =======================================================

%% === Type exports ==================================================

-export_type ([gd_font_metrics/0]).

%% === Types =========================================================

-type gd_font_metrics () :: #gd_font_metrics{ x_height :: non_neg_integer (),
					      ascent :: non_neg_integer (),
					      descent :: non_neg_integer (),
					      cap_height :: non_neg_integer (),
					      line_height :: non_neg_integer () }.

%% === External functions ============================================

%%
%% new
%%

new () ->
  #gd_font_metrics{}.

%%
%% factory
%%

factory (XHeight, Ascent, Descent, CapHeight, LineHeight) ->
  #gd_font_metrics{ x_height = XHeight,
                    ascent = Ascent,
                    descent = Descent,
                    cap_height = CapHeight,
                    line_height = LineHeight }.

%%
%% set_x_height
%%

set_x_height (FontMetrics, XHeight) ->
  FontMetrics#gd_font_metrics{ x_height = XHeight }.

%%
%% get_x_height
%%

get_x_height (#gd_font_metrics{ x_height = XHeight }) ->
  XHeight.

%%
%% set_ascent
%%

set_ascent (FontMetrics, Ascent) ->
  FontMetrics#gd_font_metrics{ ascent = Ascent }.

%%
%% get_ascent
%%

get_ascent (#gd_font_metrics{ ascent = Ascent }) ->
  Ascent.

%%
%% set_descent
%%

set_descent (FontMetrics, Descent) ->
  FontMetrics#gd_font_metrics{ descent = Descent }.

%%
%% get_descent
%%

get_descent (#gd_font_metrics{ descent = Descent }) ->
  Descent.

%%
%% set_cap_height
%%

set_cap_height (FontMetrics, CapHeight) ->
  FontMetrics#gd_font_metrics{ cap_height = CapHeight }.

%%
%% get_cap_height
%%

get_cap_height (#gd_font_metrics{ cap_height = CapHeight }) ->
  CapHeight.

%%
%% set_line_height
%%

set_line_height (FontMetrics, LineHeight) ->
  FontMetrics#gd_font_metrics{ line_height = LineHeight }.

%%
%% get_line_height
%%

get_line_height (#gd_font_metrics{ line_height = LineHeight }) ->
  LineHeight.

%% === Internal functions ============================================
