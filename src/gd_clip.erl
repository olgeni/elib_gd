-module (gd_clip).

%% === Include files =================================================

-include ("gd_clip.hrl").

%% === External exports ==============================================

-export ([new/0,
	  factory/4,
	  set_x1/2,
	  get_x1/1,
	  set_y1/2,
	  get_y1/1,
	  set_x2/2,
	  get_x2/1,
	  set_y2/2,
	  get_y2/1]).

%% === Internal exports ==============================================

%% === Macros ========================================================

%% === Records =======================================================

%% === Type exports ==================================================

-export_type ([gd_clip/0]).

%% === Types =========================================================

-type gd_clip () :: #gd_clip{ x1 :: non_neg_integer (),
			      y1 :: non_neg_integer (),
			      x2 :: non_neg_integer (),
			      y2 :: non_neg_integer () }.

%% === Type exports ==================================================

%% === Types =========================================================

%% === External functions ============================================

%%
%% new
%%

new () ->
  #gd_clip{}.

%%
%% factory
%%

factory (X1, Y1, X2, Y2) ->
  #gd_clip{ x1 = X1,
	    y1 = Y1,
	    x2 = X2,
	    y2 = Y2 }.

%%
%% set_x1
%%

set_x1 (Clip, X1) ->
  Clip#gd_clip{ x1 = X1 }.

%%
%% get_x1
%%

get_x1 (#gd_clip{ x1 = X1 }) ->
  X1.

%%
%% set_y1
%%

set_y1 (Clip, Y1) ->
  Clip#gd_clip{ y1 = Y1 }.

%%
%% get_y1
%%

get_y1 (#gd_clip{ y1 = Y1 }) ->
  Y1.

%%
%% set_x2
%%

set_x2 (Clip, X2) ->
  Clip#gd_clip{ x2 = X2 }.

%%
%% get_x2
%%

get_x2 (#gd_clip{ x2 = X2 }) ->
  X2.

%%
%% set_y2
%%

set_y2 (Clip, Y2) ->
  Clip#gd_clip{ y2 = Y2 }.

%%
%% get_y2
%%

get_y2 (#gd_clip{ y2 = Y2 }) ->
  Y2.

%% === Internal functions ============================================
