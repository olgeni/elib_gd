%% === Macros ========================================================

%% === Records =======================================================

-record (gd_bounds, { ll_x = 0 :: non_neg_integer (),
		      ll_y = 0 :: non_neg_integer (),
		      lr_x = 0 :: non_neg_integer (),
		      lr_y = 0 :: non_neg_integer (),
		      ur_x = 0 :: non_neg_integer (),
		      ur_y = 0 :: non_neg_integer (),
		      ul_x = 0 :: non_neg_integer (),
		      ul_y = 0 :: non_neg_integer () }).

%% === Types =========================================================
