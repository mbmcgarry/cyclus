#!/bin/bash

# export LD_LIBRARY_PATH=$PREFIX/lib
# export DYLD_LIBRARY_PATH=$PREFIX/lib
# export CMAKE_LIBRARY_PATH=$PREFIX/lib
# export PATH=$PREFIX/bin:$PATH

export MACOSX_DEPLOYMENT_TARGET=

# # 
# # Previous iterations have ahd trouble with hdf build vs. link
# # versions. Something like the following has helped in the past.
# #
# #### hack fix for hdf5 issues
#  if [[  `uname` == 'Linux' ]]; then
#    ln -s $PREFIX/lib/libhdf5.so.9 $PREFIX/lib/libhdf5.so.8
#    ln -s $PREFIX/lib/libhdf5_hl.so.9 $PREFIX/lib/libhdf5_hl.so.8
#  else
#ln -s $PREFIX/lib/libhdf5.10.dylib $PREFIX/lib/libhdf5.9.dylib
#ln -s $PREFIX/lib/libhdf5_hl.10.dylib $PREFIX/lib/libhdf5_hl.9.dylib
#  fi

echo `pwd`
mkdir build
cd build
export PREFIX=/Users/mbmcgarry/miniconda/envs/_build
cmake .. -DCMAKE_INSTALL_PREFIX=$PREFIX -DHDF5_ROOT=/opt/local/lib -DCOIN_ROOT_DIR=/opt/local/lib -DBOOST_ROOT=$PREFIX/lib -DCMAKE_BUILD_TYPE=Release -DLAPACK_LIBRARIES=$PREFIX/lib/liblapack.dylib -DBLAS_LIBRARIES=$PREFIX/lib/libblas.dylib -DCMAKE_PREFIX_PATH=$PREFIX -DHDF5_ROOT=$PREFIX
# if [[  `uname` == 'Linux' ]]; then
#     cmake .. \
# 	-DCMAKE_INSTALL_PREFIX=$PREFIX \
# 	-DHDF5_ROOT=$PREFIX \
# 	-DBOOST_ROOT=$PREFIX \
# 	-DBOOST_LIBRARYDIR=$PREFIX/lib \
# 	-DBoost_NO_SYSTEM_PATHS=ON \
# 	-DCMAKE_BUILD_TYPE=Release \
# 	-DLAPACK_LIBRARIES=$PREFIX/lib/liblapack.so \
# 	-DBLAS_LIBRARIES=$PREFIX/lib/libblas.so
# else
#     echo $CFLAGS
#     echo $LDFLAGS
#     export MACOSX_DEPLOYMENT_TARGET=
#     export DYLD_LIBRARY_PATH=$PREFIX/lib
#  #   export LDFLAGS="-headerpad_max_install_names -headerpad"
#  #   export CFLAGS="-headerpad_max_install_names -headerpad"
#  #   export CXXFLAGS=
#     cmake .. \
# 	-DCMAKE_INSTALL_PREFIX=$PREFIX \
# 	-DHDF5_ROOT=/opt/local/lib \
# 	-DCOIN_ROOT_DIR=/opt/local/lib \
# 	-DBOOST_ROOT=$PREFIX/lib \
# 	-DCMAKE_BUILD_TYPE=Release \
# 	-DLAPACK_LIBRARIES=$PREFIX/lib/liblapack.dylib \
# 	-DBLAS_LIBRARIES=$PREFIX/lib/libblas.dylib \
# 	-DCMAKE_PREFIX_PATH=$PREFIX
# fi

make VERBOSE=1 -j 4
make install

 
cd ..
