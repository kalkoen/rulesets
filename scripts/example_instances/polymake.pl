# hull.pl
use strict;
use application 'polytope';

# 1. Load the Polytope
my $lp_file = "../../data/test_grid/model_fixed.lp";
my $p = lp2poly($lp_file);

# 2. Get variable names in polymake's own column order
my $labels = $p->COORDINATE_LABELS;
# labels->[0] is "inhomog_var" (homogenizing coord), skip it
# labels->[i] corresponds to polymake coordinate index i (1-based in homogeneous coords)
my @z_list;
print "--- VARIABLE MAPPING ---\n";
for (my $i = 1; $i < scalar(@$labels); $i++) {
    my $var_name = $labels->[$i];
    print "Column ", $i, " : ", $var_name, "\n";
    if ($var_name =~ /^z/) { push @z_list, $i; }
}
my $z_indices = new Set<Int>(@z_list);

print "\nz indices: ", join(", ", @z_list), "\n";

# 3. Find feasible integer z-vectors
my $z_proj = projection($p, $z_indices);
my $z_lattice = $z_proj->LATTICE_POINTS;

print "\n--- FEASIBLE INTEGER Z-COMBINATIONS ---\n";
for (my $i = 0; $i < $z_lattice->rows; ++$i) {
    print $z_lattice->row($i), "\n";
}

# 4. Compute Hull
my @all_points;
for (my $i = 0; $i < $z_lattice->rows; ++$i) {
    my $z_vec = $z_lattice->row($i);
    # z_vec is [1, z_val_0, z_val_1, ...] in homogeneous coords of the projected space
    my $eq_matrix = new Matrix<Rational>(scalar(@z_list), $p->AMBIENT_DIM + 1);
    for (my $j = 0; $j < scalar(@z_list); ++$j) {
        my $col = $z_list[$j];          # coordinate index in original polytope
        $eq_matrix->elem($j, $col) = 1;
        $eq_matrix->elem($j, 0)    = -$z_vec->[$j + 1];
    }
    my $trivial_ineq = new Matrix<Rational>(1, $p->AMBIENT_DIM + 1);
    $trivial_ineq->elem(0, 0) = 1;
    my $fixing_poly = new Polytope<Rational>(EQUATIONS=>$eq_matrix, INEQUALITIES=>$trivial_ineq);
    my $slice = intersection($p, $fixing_poly);
    my $v = $slice->VERTICES;
    for (my $k = 0; $k < $v->rows; ++$k) { push @all_points, $v->row($k); }
}

my $mi_hull = new Polytope<Rational>(POINTS=>\@all_points);
print "\n--- FACETS OF THE MIXED-INTEGER HULL ---\n";
print $mi_hull->FACETS;