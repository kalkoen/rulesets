# 1. Clean start
rm -rf build/
mkdir build
cd build
# 2. Load environment
source ../../ondemand/data/sys/dashboard/projects/boolean-rule-sets/hpc/load_modules.sh 

module load CMake/3.29.3-GCCcore-13.3.0



# Clean the previous failed build state to ensure flags take effect
rm -rf CMakeCache.txt CMakeFiles/
# The "HPC-Safe" CMake call with the linker fix
cmake .. \
  -DTHREADS_PREFER_PTHREAD_FLAG=ON \
  -DCMAKE_EXE_LINKER_FLAGS="-lpthread -lgfortran" \
  -DCMAKE_INSTALL_PREFIX=$HOME/scip_install \
  -DSHARED=OFF

# Build and Install
make -j8
make install
