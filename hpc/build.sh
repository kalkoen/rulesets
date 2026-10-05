module purge
source hpc/load_modules.sh
module load CMake/3.31.3-GCCcore-14.2.0
#module load CMake/3.29.3-GCCcore-13.3.0
rm -r build-backup/
cp -r build build-backup/
rm -r build
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=TRUE
make -j
cd ..
