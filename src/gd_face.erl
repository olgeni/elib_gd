%%--------------------------------------------------------------------
%% $Id$
%%--------------------------------------------------------------------

-module (gd_face).

-vsn ("$Id$").

%% === Include files =================================================

-include ("gd_face.hrl").

%% === External exports ==============================================

-export ([new/0,
          factory/2,
          set_family_name/2,
          get_family_name/1,
          set_style_name/2,
          get_style_name/1]).

%% === Internal exports ==============================================

%% === Macros ========================================================

%% === Records =======================================================

%% === Type exports ==================================================

-export_type ([gd_face/0]).

%% === Types =========================================================

-type gd_face () :: #gd_face{ family_name :: string (),
			      style_name :: string () }.

%% === Type exports ==================================================

%% === Types =========================================================

%% === External functions ============================================

%%
%% new
%%

new () ->
  #gd_face{}.

%%
%% factory
%%

factory (FamilyName, StyleName) ->
  #gd_face{ family_name = FamilyName,
            style_name = StyleName }.

%%
%% set_family_name
%%

set_family_name (Face, FamilyName) ->
  Face#gd_face{ family_name = FamilyName }.

%%
%% get_family_name
%%

get_family_name (#gd_face{ family_name = FamilyName }) ->
  FamilyName.

%%
%% set_style_name
%%

set_style_name (Face, StyleName) ->
  Face#gd_face{ style_name = StyleName }.

%%
%% get_style_name
%%

get_style_name (#gd_face{ style_name = StyleName }) ->
  StyleName.

%% === Internal functions ============================================
