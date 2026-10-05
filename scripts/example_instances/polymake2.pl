use application 'polytope';

sub upper_bound_5 {

   # we consider the example with one positive point, five possibilities for l, and gamma_il values 1, 2, 3, 4, 5
   # we order the variables as [z1, z2, z3, z4, z5, gamma]
   my $P = new Polytope(INEQUALITIES=>[
			   [6,-5,0,0,0,0,-1],
			   [6,0,-4,0,0,0,-1],
			   [6,0,0,-3,0,0,-1],
			   [6,0,0,0,-2,0,-1],
			   [6,0,0,0,0,-1,-1], # defining inequalities
#			   [-1,1,1,1,1,1,0], # at least one literal is selected
			   [0,1,0,0,0,0,0],[0,0,1,0,0,0,0],[0,0,0,1,0,0,0],[0,0,0,0,1,0,0],[0,0,0,0,0,1,0],[0,0,0,0,0,0,1], # all variables are nonnegative
			   [1,-1,0,0,0,0,0],[1,0,-1,0,0,0,0],[1,0,0,-1,0,0,0],[1,0,0,0,-1,0,0],[1,0,0,0,0,-1,0] # the z-variables are at most one
			]);

   # we compute the convex hull of all integer points in P
   my $Q = new Polytope(POINTS=>$P->LATTICE_POINTS);

   # the strongest inequalities
   print "The facets are:\n";
   print $Q->FACETS;

   # inequalities have the shape b + ax >= 0
   # interesting inequalities
   # 5 -4 0 0 0 0 -1
   # 4 -3 0 0 0 1 -1
   # 3 -2 0 0 1 2 -1
   # 2 -1 0 1 2 3 -1
   #
   # 3 -1 -1 0 1 2 -1
   # 4 -1 -1 -1 0 1 -1
   # 5 -1 -1 -1 -1 0 -1
   #
   # 5 -3 0 0 -1 0 -1
   # 5 -2 0 -2 0 0 -1
   # 5 -1 -3 0 0 0 -1
   #
   # 5 -1 -1 -2 0 0 -1
   # 5 -1 -2 0 -1 0 -1
   # 5 -2 0 -1 -1 0 -1
   #
   # 4 -2 0 -1 0 1 -1
   # 4 -1 -2 0 0 1 -1
   #
   # select at least one z
   # -1 1 1 1 1 1 0
   # upper bound constraints
   # 1 -1 0 0 0 0 0
   # 1 0 -1 0 0 0 0
   # 1 0 0 -1 0 0 0
   # 1 0 0 0 -1 0 0
   # 1 0 0 0 0 -1 0
   # lower bound constraints
   # 0 1 0 0 0 0 0
   # 0 0 1 0 0 0 0
   # 0 0 0 1 0 0 0
   # 0 0 0 0 1 0 0
   # 0 0 0 0 0 1 0
   # 0 0 0 0 0 0 1

}

sub upper_bound_7 {

   # we consider the example with one positive point, five possibilities for l, and gamma_il values 1, 2, 4, 7, 9, 13, 14
   # we order the variables as [z1, z2, z3, z4, z5, z6, z7, gamma]
   my $P = new Polytope(INEQUALITIES=>[
			   [14,-13,0,0,0,0,0,0,-1],[14,0,-12,0,0,0,0,0,-1],[14,0,0,-10,0,0,0,0,-1],[14,0,0,0,-7,0,0,0,-1],[14,0,0,0,0,-5,0,0,-1],[14,0,0,0,0,0,-1,0,-1],[14,0,0,0,0,0,0,0,-1], # defining inequalities
			   [-1,1,1,1,1,1,1,1,0], # at least one literal is selected
			   [0,1,0,0,0,0,0,0,0],[0,0,1,0,0,0,0,0,0],[0,0,0,1,0,0,0,0,0],[0,0,0,0,1,0,0,0,0],[0,0,0,0,0,1,0,0,0],[0,0,0,0,0,0,1,0,0],[0,0,0,0,0,0,0,1,0],[0,0,0,0,0,0,0,0,1], # all variables are nonnegative
			   [1,-1,0,0,0,0,0,0,0],[1,0,-1,0,0,0,0,0,0],[1,0,0,-1,0,0,0,0,0],[1,0,0,0,-1,0,0,0,0],[1,0,0,0,0,-1,0,0,0],[1,0,0,0,0,0,-1,0,0],[1,0,0,0,0,0,0,-1,0] # the z-variables are at most one
			]);

   # we compute the convex hull of all integer points in P
   my $Q = new Polytope(POINTS=>$P->LATTICE_POINTS);

   # the strongest inequalities
   print "The facets are:\n";
   print $Q->FACETS;

   # inequalities have the shape b + ax >= 0
   # interesting inequalities with some pattern
   #
   # one 0 block length 1
   # 2 -1 0 2 5 7 11 12 -1
   # 4 -1 -2 0 3 5 9 10 -1
   # 7 -1 -2 -3 0 2 6 7 -1
   # 9 -1 -2 -3 -2 0 4 5 -1
   # 13 -1 -2 -3 -2 -4 0 1 -1
   # 14 -1 -2 -3 -2 -4 -1 0 -1
   #
   # one 0 block length 2
   # 4 -3 0 0 3 5 9 10 -1
   # 7 -1 -5 0 0 2 6 7 -1
   # 9 -1 -2 -5 0 0 4 5 -1
   # 13 -1 -2 -3 -6 0 0 1 -1
   # 14 -1 -2 -3 -2 -5 0 0 -1
   #
   # one 0 block length 3
   # 7 -6 0 0 0 2 6 7 -1
   # 9 -1 -7 0 0 0 4 5 -1
   # 13 -1 -2 -9 0 0 0 1 -1
   # 14 -1 -2 -3 -7 0 0 0 -1
   #
   # one 0 block length 4
   # 9 -8 0 0 0 0 4 5 -1
   # 13 -1 -11 0 0 0 0 1 -1
   # 14 -1 -2 -10 0 0 0 0 -1
   #
   # one 0 block length 5
   # 13 -12 0 0 0 0 0 1 -1
   # 14 -1 -12 0 0 0 0 0 -1
   #
   # one 0 block length 6
   # 14 -13 0 0 0 0 0 0 -1
   #
   # some other interesting inequalities
   # 14 -1 -5 0 -7 0 0 0 -1
   # 14 -3 0 -5 0 -4 -1 0 -1
   # 14 -6 0 0 -7 0 0 0 -1
   # 14 -3 0 -3 -7 0 0 0 -1
   # 14 -1 -2 -5 0 -5 0 0 -1
   # 14 -1 -5 0 -2 -5 0 0 -1
   # 14 -8 0 0 0 -5 0 0 -1
   # 14 -3 0 -5 0 -5 0 0 -1
   # 14 -6 0 0 -2 -5 0 0 -1
   # 14 -3 0 -3 -2 -5 0 0 -1
   # 14 -3 0 -10 0 0 0 0 -1
   # 14 -1 -7 0 0 -5 0 0 -1
   # 14 -1 -2 -3 -6 0 -1 0 -1
   # 14 -8 0 0 0 -4 -1 0 -1
   # 14 -1 -5 0 -2 -4 -1 0 -1
   # 14 -6 0 0 -2 -4 -1 0 -1
   # 14 -1 -2 -9 0 0 -1 0 -1
   # 14 -1 -11 0 0 0 -1 0 -1
   # 14 -1 -2 -5 0 -4 -1 0 -1
   # 14 -1 -5 0 -6 0 -1 0 -1
   # 14 -12 0 0 0 0 -1 0 -1
   # 14 -6 0 0 -6 0 -1 0 -1
   # 14 -3 0 -9 0 0 -1 0 -1
   # 14 -1 -7 0 0 -4 -1 0 -1
   # 14 -3 0 -3 -6 0 -1 0 -1
   # 14 -3 0 -3 -2 -4 -1 0 -1
   # 13 -3 0 -9 0 0 0 1 -1
   # 13 -1 -2 -5 0 -4 0 1 -1
   # 13 -1 -7 0 0 -4 0 1 -1
   # 13 -3 0 -5 0 -4 0 1 -1
   # 13 -1 -5 0 -2 -4 0 1 -1
   # 13 -1 -5 0 -6 0 0 1 -1
   # 13 -6 0 0 -2 -4 0 1 -1
   # 13 -3 0 -3 -2 -4 0 1 -1
   # 13 -6 0 0 -6 0 0 1 -1
   # 9 -1 -5 0 -2 0 4 5 -1
   # 9 -3 0 -5 0 0 4 5 -1
   # 7 -3 0 -3 0 2 6 7 -1
   # 9 -6 0 0 -2 0 4 5 -1
   # 9 -3 0 -3 -2 0 4 5 -1
   # 13 -8 0 0 0 -4 0 1 -1
   # 13 -3 0 -3 -6 0 0 1 -1
   #
   # at least one z is selected
   # -1 1 1 1 1 1 1 1 0
   # lower bounds on all variables
   # 0 1 0 0 0 0 0 0 0
   # 0 0 1 0 0 0 0 0 0
   # 0 0 0 1 0 0 0 0 0
   # 0 0 0 0 1 0 0 0 0
   # 0 0 0 0 0 1 0 0 0
   # 0 0 0 0 0 0 1 0 0
   # 0 0 0 0 0 0 0 1 0
   # 0 0 0 0 0 0 0 0 1
   # upper bounds on z
   # 1 -1 0 0 0 0 0 0 0
   # 1 0 -1 0 0 0 0 0 0
   # 1 0 0 -1 0 0 0 0 0
   # 1 0 0 0 -1 0 0 0 0
   # 1 0 0 0 0 -1 0 0 0
   # 1 0 0 0 0 0 -1 0 0
   # 1 0 0 0 0 0 0 -1 0

}
#
#sub lower_bound_5 {
#
#   # we consider the example with one positive point, five possibilities for l, and gamma_il values 1, 2, 3, 4, 5
#   # we order the variables as [z1, z2, z3, z4, z5, gamma]
#   my $P = new Polytope(INEQUALITIES=>[
#			   [-1,0,0,0,0,0,1],[-2,1,0,0,0,0,1],[-3,2,1,0,0,0,1],[-4,3,2,1,0,0,1],[-5,4,3,2,1,0,1],  # defining inequalities
#			   [-1,1,1,1,1,1,0], # at least one literal is selected
#			   [0,1,0,0,0,0,0],[0,0,1,0,0,0,0],[0,0,0,1,0,0,0],[0,0,0,0,1,0,0],[0,0,0,0,0,1,0],[0,0,0,0,0,0,1], # all variables are nonnegative
#			   [1,-1,0,0,0,0,0],[1,0,-1,0,0,0,0],[1,0,0,-1,0,0,0],[1,0,0,0,-1,0,0],[1,0,0,0,0,-1,0], # the z-variables are at most one
#			   [5,0,0,0,0,0,-1] # gamma is at most 5
#			]);
#
#   # we compute the convex hull of all integer points in P
#   my $Q = new Polytope(POINTS=>$P->LATTICE_POINTS);
#
#   # the strongest inequalities
#   print "The facets are:\n";
#   print $Q->FACETS;
#
#   # inequalities have the shape b + ax >= 0
#   # the inequalities are exactly the defining inequalities
#   # 1 0 -1 0 0 0 0
#   # 1 -1 0 0 0 0 0
#   # 1 0 0 -1 0 0 0
#   # 1 0 0 0 -1 0 0
#   # 1 0 0 0 0 -1 0
#   # 5 0 0 0 0 0 -1
#   # -2 1 0 0 0 0 1
#   # 0 0 0 0 1 0 0
#   # 0 0 0 1 0 0 0
#   # -1 1 1 1 1 1 0
#   # -5 4 3 2 1 0 1
#   # 0 0 0 0 0 1 0
#   # 0 1 0 0 0 0 0
#   # -4 3 2 1 0 0 1
#   # -3 2 1 0 0 0 1
#   # 0 0 1 0 0 0 0
#
#}
#
#sub lower_bound_7 {
#
#   # we consider the example with one positive point, five possibilities for l, and gamma_il values 1, 2, 4, 7, 9, 13, 14
#   # we order the variables as [z1, z2, z3, z4, z5, z6, z7, gamma]
#   my $P = new Polytope(INEQUALITIES=>[
#			   [-1,0,0,0,0,0,0,0,1],[-2,1,0,0,0,0,0,0,1],[-4,3,2,0,0,0,0,0,1],[-7,6,5,3,0,0,0,0,1],[-9,8,6,5,2,0,0,0,1],[-13,12,11,9,6,4,0,0,1],[-14,13,12,10,7,5,1,0,1],  # defining inequalities
#			   [-1,1,1,1,1,1,1,1,0], # at least one literal is selected
#			   [0,1,0,0,0,0,0,0,0],[0,0,1,0,0,0,0,0,0],[0,0,0,1,0,0,0,0,0],[0,0,0,0,1,0,0,0,0],[0,0,0,0,0,1,0,0,0],[0,0,0,0,0,0,1,0,0],[0,0,0,0,0,0,0,1,0],[0,0,0,0,0,0,0,0,1], # all variables are nonnegative
#			   [1,-1,0,0,0,0,0,0,0],[1,0,-1,0,0,0,0,0,0],[1,0,0,-1,0,0,0,0,0],[1,0,0,0,-1,0,0,0,0],[1,0,0,0,0,-1,0,0,0],[1,0,0,0,0,0,-1,0,0],[1,0,0,0,0,0,0,-1,0], # the z-variables are at most one
#			   [14,0,0,0,0,0,0,0,-1] # gamma is at most 14
#			]);
#
#   # we compute the convex hull of all integer points in P
#   my $Q = new Polytope(POINTS=>$P->LATTICE_POINTS);
#
#   # the strongest inequalities
#   print "The facets are:\n";
#   print $Q->FACETS;
#
#   # inequalities have the shape b + ax >= 0
#   # here some new inequalities arise
#   # -2 1 0 0 0 0 0 0 1
#   # -4 3 2 0 0 0 0 0 1
#   # -7 6 5 3 0 0 0 0 1
#   # -8 7 5 4 1 0 0 0 1
#   # -9 8 6 5 2 0 0 0 1
#   # -5 4 2 1 1 0 0 0 1
#   # -13 12 10 9 6 4 0 0 1
#   # -14 13 11 10 7 5 1 0 1
#   #
#   # -3 2 0 1 1 0 0 0 1
#   #
#   # select one z
#   # -1 1 1 1 1 1 1 1 0
#   #
#   # upper bound
#   # 1 -1 0 0 0 0 0 0 0
#   # 1 0 -1 0 0 0 0 0 0
#   # 1 0 0 -1 0 0 0 0 0
#   # 1 0 0 0 -1 0 0 0 0
#   # 1 0 0 0 0 -1 0 0 0
#   # 1 0 0 0 0 0 -1 0 0
#   # 1 0 0 0 0 0 0 -1 0
#   # 14 0 0 0 0 0 0 0 -1
#   #
#   # lower bounds
#   # 0 1 0 0 0 0 0 0 0
#   # 0 0 1 0 0 0 0 0 0
#   # 0 0 0 1 0 0 0 0 0
#   # 0 0 0 0 1 0 0 0 0
#   # 0 0 0 0 0 1 0 0 0
#   # 0 0 0 0 0 0 1 0 0
#   # 0 0 0 0 0 0 0 1 0
#
#}
#
#sub upper_bound_5_alternative {
#
#   # we consider the example with one positive point, five possibilities for l, and gamma_il values 1, 2, 3, 4, 5
#   # we order the variables as [z1, z2, z3, z4, z5, gamma]
#   my $P = new Polytope(INEQUALITIES=>[
#                [1, 4, 0, 0, 0, 0, -1],
#                [2, 0, 3, 0, 0, 0, -1],
#                [3, 0, 0, 2, 0, 0, -1],
#                [4, 0, 0, 0, 1, 0, -1],
#                [1, 1, 3, 0, 0, 0, -1],
#                [1, 2, 0, 2, 0, 0, -1],
#                [1, 3, 0, 0, 1, 0, -1],
#                [2, 0, 1, 2, 0, 0, -1],
#                [2, 0, 2, 0, 1, 0, -1],
#                [3, 0, 0, 1, 1, 0, -1],
#                [1, 1, 1, 2, 0, 0, -1],
#                [1, 1, 2, 0, 1, 0, -1],
#                [1, 2, 0, 1, 1, 0, -1],
#                [2, 0, 1, 1, 1, 0, -1],
#                [1, 1, 1, 1, 1, 0, -1],
#			   [-1,1,1,1,1,1,0], # at least one literal is selected
#			   [0,1,0,0,0,0,0],
#			   [0,0,1,0,0,0,0],
#			   [0,0,0,1,0,0,0],
#			   [0,0,0,0,1,0,0],
#			   [0,0,0,0,0,1,0],
#			   [0,0,0,0,0,0,1], # all variables are nonnegative
#			   [1,-1,0,0,0,0,0],
#			   [1,0,-1,0,0,0,0],
#			   [1,0,0,-1,0,0,0],
#			   [1,0,0,0,-1,0,0],
#			   [1,0,0,0,0,-1,0] # the z-variables are at most one
#			]);
#
#    print "The integer points are are:\n";
#    print $P->VERTICES;
#
##   # we compute the convex hull of all integer points in P
#   my $Q = new Polytope(POINTS=>$P->LATTICE_POINTS);
##
##   # the strongest inequalities
#   print "The facets are:\n";
#   print $Q->FACETS;
#
#   # inequalities have the shape b + ax >= 0
#   # interesting inequalities
#   # 5 -4 0 0 0 0 -1
#   # 4 -3 0 0 0 1 -1
#   # 3 -2 0 0 1 2 -1
#   # 2 -1 0 1 2 3 -1
#   #
#   # 3 -1 -1 0 1 2 -1
#   # 4 -1 -1 -1 0 1 -1
#   # 5 -1 -1 -1 -1 0 -1
#   #
#   # 5 -3 0 0 -1 0 -1
#   # 5 -2 0 -2 0 0 -1
#   # 5 -1 -3 0 0 0 -1
#   #
#   # 5 -1 -1 -2 0 0 -1
#   # 5 -1 -2 0 -1 0 -1
#   # 5 -2 0 -1 -1 0 -1
#   #
#   # 4 -2 0 -1 0 1 -1
#   # 4 -1 -2 0 0 1 -1
#   #
#   # select at least one z
#   # -1 1 1 1 1 1 0
#   # upper bound constraints
#   # 1 -1 0 0 0 0 0
#   # 1 0 -1 0 0 0 0
#   # 1 0 0 -1 0 0 0
#   # 1 0 0 0 -1 0 0
#   # 1 0 0 0 0 -1 0
#   # lower bound constraints
#   # 0 1 0 0 0 0 0
#   # 0 0 1 0 0 0 0
#   # 0 0 0 1 0 0 0
#   # 0 0 0 0 1 0 0
#   # 0 0 0 0 0 1 0
#   # 0 0 0 0 0 0 1
#
#}
#
#
#upper_bound_5_alternative();

#
#sub upper_bound_5_less {
#
#   # we consider the example with one positive point, five possibilities for l, and gamma_il values 1, 2, 3, 4, 5
#   # we order the variables as [z1, z2, z3, z4, z5, gamma]
#   my $P = new Polytope(INEQUALITIES=>[
#			   [5,-4,0,0,0,0,-1],[5,0,-3,0,0,0,-1],[5,0,0,-2,0,0,-1],[5,0,0,0,-1,0,-1],[5,0,0,0,0,0,-1], # defining inequalities
#			   [-1,1,1,1,1,1,0], # at least one literal is selected
##			   [5,1,1,1,1,1,0], # at most five literal is selected
#			   [0,1,0,0,0,0,0],[0,0,1,0,0,0,0],[0,0,0,1,0,0,0],[0,0,0,0,1,0,0],[0,0,0,0,0,1,0],[0,0,0,0,0,0,1], # all variables are nonnegative
#			   [1,-1,0,0,0,0,0],[1,0,-1,0,0,0,0],[1,0,0,-1,0,0,0],[1,0,0,0,-1,0,0],[1,0,0,0,0,-1,0] # the z-variables are at most one
#			]);
#
#   # we compute the convex hull of all integer points in P
#   my $Q = new Polytope(POINTS=>$P->LATTICE_POINTS);
#
#   # the integer points
#   print "The integer points are:\n";
#   print $Q->LATTICE_POINTS;
#
#   # the strongest inequalities
#   print "The facets are:\n";
#   print $Q->FACETS;
#
#
#}
#
#upper_bound_5_less();


sub upper_bound_5_buildup {

   my $P = new Polytope(INEQUALITIES=>[
#            [5, 0, 0, 0, -1, 0, -1],
#            [5, 0, 0, -1, -1, 0, -1],
#            [5, 0, 0, -2, 0, 0, -1],
#            [5, 0, -1, -1, -1, 0, -1],
#            [5, 0, -1, -2, 0, 0, -1],
#            [5, 0, -2, 0, -1, 0, -1],
#            [5, 0, -3, 0, 0, 0, -1],
#            [5, -1, -1, -1, -1, 0, -1],
#            [5, -1, -1, -2, 0, 0, -1],
#            [5, -1, -2, 0, -1, 0, -1],
#            [5, -1, -3, 0, 0, 0, -1],
#            [5, -2, 0, -1, -1, 0, -1],
#            [5, -2, 0, -2, 0, 0, -1],
#            [5, -3, 0, 0, -1, 0, -1],
#            [5, -4, 0, 0, 0, 0, -1],
#            [4, 0, 0, -1, 0, 1, -1],
#            [4, 0, -1, -1, 0, 1, -1],
#            [4, 0, -2, 0, 0, 1, -1],
#            [4, -1, -1, -1, 0, 1, -1],
#            [4, -1, -2, 0, 0, 1, -1],
#            [4, -2, 0, -1, 0, 1, -1],
#            [4, -3, 0, 0, 0, 1, -1],
#            [3, 0, -1, 0, 1, 2, -1],
#            [3, -1, -1, 0, 1, 2, -1],
#            [3, -2, 0, 0, 1, 2, -1],
#            [2, -1, 0, 1, 2, 3, -1],
			   [-1,1,1,1,1,1,0], # at least one literal is selected
			   [0,1,0,0,0,0,0],[0,0,1,0,0,0,0],[0,0,0,1,0,0,0],[0,0,0,0,1,0,0],[0,0,0,0,0,1,0],[0,0,0,0,0,0,1], # all variables are nonnegative
			   [1,-1,0,0,0,0,0],[1,0,-1,0,0,0,0],[1,0,0,-1,0,0,0],[1,0,0,0,-1,0,0],[1,0,0,0,0,-1,0] # the z-variables are at most one
			]);

   # the strongest inequalities
   print "\n The vertices are:\n";
   print $P->VERTICES;

   my $Q = new Polytope(POINTS=>$P->LATTICE_POINTS);
   print "\n The integer points are:\n";
   print $P->LATTICE_POINTS;

   print "\n The facets are:\n";
   print $Q->FACETS;

}

sub upper_bound_5_buildup_2 {

   my $P = new Polytope(INEQUALITIES=>[
#[12, 0, 0, 0, -3, 0, -1],
#[12, 0, 0, -4, -3, 0, -1],
#[12, 0, 0, -7, 0, 0, -1],
#[12, 0, -2, -4, -3, 0, -1],
#[12, 0, -2, -7, 0, 0, -1],
#[12, 0, -6, 0, -3, 0, -1],
#[12, 0, -9, 0, 0, 0, -1],
#[12, -2, -2, -4, -3, 0, -1],
#[12, -2, -2, -7, 0, 0, -1],
#[12, -2, -6, 0, -3, 0, -1],
#[12, -2, -9, 0, 0, 0, -1],
#[12, -4, 0, -4, -3, 0, -1],
#[12, -4, 0, -7, 0, 0, -1],
#[12, -8, 0, 0, -3, 0, -1],
#[12, -11, 0, 0, 0, 0, -1],
[9, 0, 0, -4, 0, 3, -1],
[9, 0, -2, -4, 0, 3, -1],
[9, 0, -6, 0, 0, 3, -1],
[9, -2, -2, -4, 0, 3, -1],
[9, -2, -6, 0, 0, 3, -1],
[9, -4, 0, -4, 0, 3, -1],
[9, -8, 0, 0, 0, 3, -1],
#[5, 0, -2, 0, 4, 7, -1],
#[5, -2, -2, 0, 4, 7, -1],
#[5, -4, 0, 0, 4, 7, -1],
#[3, -2, 0, 2, 6, 9, -1],
			   [-1,1,1,1,1,1,0], # at least one literal is selected
			   [0,1,0,0,0,0,0],[0,0,1,0,0,0,0],[0,0,0,1,0,0,0],[0,0,0,0,1,0,0],[0,0,0,0,0,1,0],[0,0,0,0,0,0,1], # all variables are nonnegative
			   [1,-1,0,0,0,0,0],[1,0,-1,0,0,0,0],[1,0,0,-1,0,0,0],[1,0,0,0,-1,0,0],[1,0,0,0,0,-1,0] # the z-variables are at most one
			]);

   # the strongest inequalities
   print "\n The vertices are:\n";
   print $P->VERTICES;

   my $Q = new Polytope(POINTS=>$P->LATTICE_POINTS);
   print "\n The integer points are:\n";
   print $P->LATTICE_POINTS;

   print "\n The facets are:\n";
   print $Q->FACETS;

   # inequalities have the shape b + ax >= 0
   # interesting inequalities
   # 5 -4 0 0 0 0 -1
   # 4 -3 0 0 0 1 -1
   # 3 -2 0 0 1 2 -1
   # 2 -1 0 1 2 3 -1
   #
   # 3 -1 -1 0 1 2 -1
   # 4 -1 -1 -1 0 1 -1
   # 5 -1 -1 -1 -1 0 -1
   #
   # 5 -3 0 0 -1 0 -1
   # 5 -2 0 -2 0 0 -1
   # 5 -1 -3 0 0 0 -1
   #
   # 5 -1 -1 -2 0 0 -1
   # 5 -1 -2 0 -1 0 -1
   # 5 -2 0 -1 -1 0 -1
   #
   # 4 -2 0 -1 0 1 -1
   # 4 -1 -2 0 0 1 -1
   #
   # select at least one z
   # -1 1 1 1 1 1 0
   # upper bound constraints
   # 1 -1 0 0 0 0 0
   # 1 0 -1 0 0 0 0
   # 1 0 0 -1 0 0 0
   # 1 0 0 0 -1 0 0
   # 1 0 0 0 0 -1 0
   # lower bound constraints
   # 0 1 0 0 0 0 0
   # 0 0 1 0 0 0 0
   # 0 0 0 1 0 0 0
   # 0 0 0 0 1 0 0
   # 0 0 0 0 0 1 0
   # 0 0 0 0 0 0 1

}

upper_bound_5()