%%--------------------------------------------------------------------
%% $Id: gd_bounds.erl 30386 2012-09-02 12:00:27Z olgeni $
%%--------------------------------------------------------------------

-module (gd_bounds).

-vsn ("$Id: gd_bounds.erl 30386 2012-09-02 12:00:27Z olgeni $").

%%====================================================================
%% Include files
%%====================================================================

-include ("gd_bounds.hrl").

%%====================================================================
%% External exports
%%====================================================================

-export ([new/0,
          factory/8,
          set_ll_x/2,
          get_ll_x/1,
          set_ll_y/2,
          get_ll_y/1,
          set_lr_x/2,
          get_lr_x/1,
          set_lr_y/2,
          get_lr_y/1,
          set_ur_x/2,
          get_ur_x/1,
          set_ur_y/2,
          get_ur_y/1,
          set_ul_x/2,
          get_ul_x/1,
          set_ul_y/2,
          get_ul_y/1,
          min_x/1,
          min_y/1,
          max_x/1,
          max_y/1,
          normalize/1]).

%%====================================================================
%% Internal exports
%%====================================================================

%%====================================================================
%% Macros
%%====================================================================

%%====================================================================
%% Records
%%====================================================================

%%====================================================================
%% Type exports
%%====================================================================

-export_type ([gd_bounds/0]).

%%====================================================================
%% Types
%%====================================================================

-type gd_bounds () :: #gd_bounds{ ll_x :: non_neg_integer (),
				  ll_y :: non_neg_integer (),
				  lr_x :: non_neg_integer (),
				  lr_y :: non_neg_integer (),
				  ur_x :: non_neg_integer (),
				  ur_y :: non_neg_integer (),
				  ul_x :: non_neg_integer (),
				  ul_y :: non_neg_integer () }.

%%====================================================================
%% External functions
%%====================================================================

%%
%% new
%%

new () ->
  #gd_bounds{}.

%%
%% factory
%%

factory (LL_X, LL_Y, LR_X, LR_Y, UR_X, UR_Y, UL_X, UL_Y) ->
  #gd_bounds{ ll_x = LL_X,
              ll_y = LL_Y,
              lr_x = LR_X,
              lr_y = LR_Y,
              ur_x = UR_X,
              ur_y = UR_Y,
              ul_x = UL_X,
              ul_y = UL_Y }.

%%
%% set_ll_x
%%

set_ll_x (Bounds, LL_X) ->
  Bounds#gd_bounds{ ll_x = LL_X }.

%%
%% get_ll_x
%%

get_ll_x (#gd_bounds{ ll_x = LL_X }) ->
  LL_X.

%%
%% set_ll_y
%%

set_ll_y (Bounds, LL_Y) ->
  Bounds#gd_bounds{ ll_y = LL_Y }.

%%
%% get_ll_y
%%

get_ll_y (#gd_bounds{ ll_y = LL_Y }) ->
  LL_Y.

%%
%% set_lr_x
%%

set_lr_x (Bounds, LR_X) ->
  Bounds#gd_bounds{ lr_x = LR_X }.

%%
%% get_lr_x
%%

get_lr_x (#gd_bounds{ lr_x = LR_X }) ->
  LR_X.

%%
%% set_lr_y
%%

set_lr_y (Bounds, LR_Y) ->
  Bounds#gd_bounds{ lr_y = LR_Y }.

%%
%% get_lr_y
%%

get_lr_y (#gd_bounds{ lr_y = LR_Y }) ->
  LR_Y.

%%
%% set_ur_x
%%

set_ur_x (Bounds, UR_X) ->
  Bounds#gd_bounds{ ur_x = UR_X }.

%%
%% get_ur_x
%%

get_ur_x (#gd_bounds{ ur_x = UR_X }) ->
  UR_X.

%%
%% set_ur_y
%%

set_ur_y (Bounds, UR_Y) ->
  Bounds#gd_bounds{ ur_y = UR_Y }.

%%
%% get_ur_y
%%

get_ur_y (#gd_bounds{ ur_y = UR_Y }) ->
  UR_Y.

%%
%% set_ul_x
%%

set_ul_x (Bounds, UL_X) ->
  Bounds#gd_bounds{ ul_x = UL_X }.

%%
%% get_ul_x
%%

get_ul_x (#gd_bounds{ ul_x = UL_X }) ->
  UL_X.

%%
%% set_ul_y
%%

set_ul_y (Bounds, UL_Y) ->
  Bounds#gd_bounds{ ul_y = UL_Y }.

%%
%% get_ul_y
%%

get_ul_y (#gd_bounds{ ul_y = UL_Y }) ->
  UL_Y.

%%
%% min_x
%%

min_x (#gd_bounds{ ll_x = LL_X, lr_x = LR_X, ur_x = UR_X, ul_x = UL_X }) ->
  lists:min ([LL_X, LR_X, UR_X, UL_X]).

%%
%% min_y
%%

min_y (#gd_bounds{ ll_y = LL_Y, lr_y = LR_Y, ur_y = UR_Y, ul_y = UL_Y }) ->
  lists:min ([LL_Y, LR_Y, UR_Y, UL_Y]).

%%
%% max_x
%%

max_x (#gd_bounds{ ll_x = LL_X, lr_x = LR_X, ur_x = UR_X, ul_x = UL_X }) ->
  lists:max ([LL_X, LR_X, UR_X, UL_X]).

%%
%% max_y
%%

max_y (#gd_bounds{ ll_y = LL_Y, lr_y = LR_Y, ur_y = UR_Y, ul_y = UL_Y }) ->
  lists:max ([LL_Y, LR_Y, UR_Y, UL_Y]).

%%
%% normalize
%%

normalize (Bounds = #gd_bounds{ ll_x = LL_X, ll_y = LL_Y, lr_x = LR_X, lr_y = LR_Y,
                                ur_x = UR_X, ur_y = UR_Y, ul_x = UL_X, ul_y = UL_Y }) ->

  DeltaX = min_x (Bounds),
  DeltaY = min_y (Bounds),

  factory (LL_X - DeltaX, LL_Y - DeltaY, LR_X - DeltaX, LR_Y - DeltaY,
           UR_X - DeltaX, UR_Y - DeltaY, UL_X - DeltaX, UL_Y - DeltaY).

%%====================================================================
%% Internal functions
%%====================================================================
