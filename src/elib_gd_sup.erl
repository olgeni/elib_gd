-module (elib_gd_sup).

-behaviour (supervisor).

%% === Include files =================================================

%% === External exports ==============================================

-export ([start_link/0]).

%% === Internal exports ==============================================

-export ([init/1]).

%% === Macros ========================================================

%% === Records =======================================================

%% === Type exports ==================================================

%% === Types =========================================================

%% === External functions ============================================

%%--------------------------------------------------------------------
%% Function: start_link/0
%% Description: Starts the supervisor
%%--------------------------------------------------------------------

%%
%% start_link
%%

start_link () ->
  supervisor:start_link ({ local, ?MODULE }, ?MODULE, []).

%% === Server functions ==============================================

%%--------------------------------------------------------------------
%% Function: init/1
%% Returns: {ok,  {SupFlags,  [ChildSpec]}} |
%%          ignore                          |
%%          {error, Reason}
%%--------------------------------------------------------------------

init ([]) ->
  Spec = [{ elib_gd_drv, { elib_gd_drv, start_link, [] },
            permanent, 2000, worker, [elib_gd_drv] }],

  { ok, { { one_for_all, 5, 20 }, Spec } }.

%% === Internal functions ============================================
