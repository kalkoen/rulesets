\ SCIP STATISTICS
\   Problem name     : PP
\   Variables        : 8 (2 binary, 0 integer, 0 implicit integer, 6 continuous)
\   Constraints      : 9
Minimize
 Obj: -1 g_2 -1 g_3 -1 g_4 +1 g_5 +1 g_0 +1 g_1
Subject to
 con_z_total_lhs: +1 z_0_1.000000 +1 z_1_1.000000 >= +1
 con_z_total_rhs: +1 z_0_1.000000 +1 z_1_1.000000 <= +3
 con_atmostone_0_lhs: +1 z_0_1.000000 >= +0
 con_atmostone_0_rhs: +1 z_0_1.000000 <= +1
 con_atmostone_1_lhs: +1 z_1_1.000000 >= +0
 con_atmostone_1_rhs: +1 z_1_1.000000 <= +1
\   [MinConstrhdlr] <mincons_0>: idx=0
;
\   [MinConstrhdlr] <mincons_1>: idx=1
;
\   [MinConstrhdlr] <mincons_2>: idx=2
;
\   [MinConstrhdlr] <mincons_3>: idx=3
;
\   [MinConstrhdlr] <mincons_4>: idx=4
;
\   [MinConstrhdlr] <mincons_5>: idx=5
;
Bounds
 0 <= z_0_1.000000 <= 1
 0 <= z_1_1.000000 <= 1
 0 <= g_2 <= 1
 0 <= g_3 <= 1
 0 <= g_4 <= 1
 0 <= g_5 <= 1
 0 <= g_0 <= 1
 0 <= g_1 <= 1
Binaries
 z_0_1.000000 z_1_1.000000
End
