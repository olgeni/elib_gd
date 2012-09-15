%%--------------------------------------------------------------------
%% $Id: elib_gd_drv.erl 30386 2012-09-02 12:00:27Z olgeni $
%%--------------------------------------------------------------------

-module (elib_gd_drv).

-vsn ("$Id: elib_gd_drv.erl 30386 2012-09-02 12:00:27Z olgeni $").

-behaviour (gen_server).

%%====================================================================
%% Include files
%%====================================================================

%%====================================================================
%% External exports
%%====================================================================

-export ([start_link/0,
          stop/0]).

%% gen_server callbacks

-export ([init/1,
          handle_call/3,
          handle_cast/2,
          handle_info/2,
          terminate/2,
          code_change/3]).

%%====================================================================
%% Internal exports
%%====================================================================

%%====================================================================
%% Macros
%%====================================================================

-define (DRIVER, elib_gd_drv).

%%====================================================================
%% Records
%%====================================================================

-record (state, {}).

%%====================================================================
%% External functions
%%====================================================================

%%
%% start_link
%%

start_link () ->
  gen_server:start_link ({ local, ?MODULE }, ?MODULE, [], []).

%%
%% stop
%%

stop () ->
  gen_server:cast (?MODULE, stop).

%%====================================================================
%% Server functions
%%====================================================================

%%====================================================================
%% Function: init/1
%% Description: Initiates the server
%% Returns: {ok, State}          |
%%          {ok, State, Timeout} |
%%          ignore               |
%%          {stop, Reason}
%%====================================================================

init ([]) ->
  process_flag (trap_exit, true),

  case os:type () of
    { win32, _Flavor } ->
      ignore;

    { unix, freebsd } ->
      ok = erl_ddll:load_driver (code:priv_dir (elib_gd), atom_to_list (?DRIVER)),

      { ok, #state{} }
  end.

%%====================================================================
%% Function: handle_call/3
%% Description: Handling call messages
%% Returns: {reply, Reply, State}          |
%%          {reply, Reply, State, Timeout} |
%%          {noreply, State}               |
%%          {noreply, State, Timeout}      |
%%          {stop, Reason, Reply, State}   | (terminate/2 is called)
%%          {stop, Reason, State}            (terminate/2 is called)
%%====================================================================

handle_call (_Request, _From, State) ->
  Reply = ok,
  { reply, Reply, State }.

%%====================================================================
%% Function: handle_cast/2
%% Description: Handling cast messages
%% Returns: {noreply, State}          |
%%          {noreply, State, Timeout} |
%%          {stop, Reason, State}            (terminate/2 is called)
%%====================================================================

handle_cast (stop, State) ->
  { stop, normal, State };

handle_cast (_Msg, State) ->
  { noreply, State }.

%%====================================================================
%% Function: handle_info/2
%% Description: Handling all non call/cast messages
%% Returns: {noreply, State}          |
%%          {noreply, State, Timeout} |
%%          {stop, Reason, State}            (terminate/2 is called)
%%====================================================================

handle_info (_Info, State) ->
  { noreply, State }.

%%====================================================================
%% Function: terminate/2
%% Description: Shutdown the server
%% Returns: any (ignored by gen_server)
%%====================================================================

terminate (_Reason, _State) ->
  erl_ddll:unload_driver (?DRIVER).

%%====================================================================
%% Function: code_change/3
%% Purpose: Convert process state when code is changed
%% Returns: {ok, NextState}
%%====================================================================

code_change (_OldVsn, State, _Extra) ->
  { ok, State }.

%%====================================================================
%% Internal functions
%%====================================================================
