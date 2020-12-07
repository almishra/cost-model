###############################################################################
#                                                                             #
#                                                                             #
###############################################################################

usage()
{
  echo -ne "usage: ./install.sh"
  echo " [[--prefix <PREFIX> ] [-gcc | --with-gcc] [ -j | --jobs] [-h]]"
}

function MKDIR() {
  if [ ! -d $1 ]
  then
    mkdir -p $1;
  fi
}

PRINT=false

MAKE_JOBS=4
while [ "$1" != "" ]; do
  case $1 in
      --prefix )           shift
                           PREFIX="$1"
                           ;;
      -gcc | --with-gcc )  INSTALL_GCC=true
                           ;;
      -p | --print )       PRINT=true
                           ;;
      -j | --jobs )        shift
                           MAKE_JOBS="$1"
                           ;;
      * )                  usage
                           exit 1
  esac
  shift
done

if [ ! -z "$PREFIX" ]
then
  FRAMEWORK_PATH=$PREFIX
else
  FRAMEWORK_PATH=$PWD
fi
LLVM_SRC=$FRAMEWORK_PATH/src/llvm
GCC_SRC=$FRAMEWORK_PATH/src/gcc
CLANG_BUILD=$FRAMEWORK_PATH/build/clang
OPENMP_BUILD=$FRAMEWORK_PATH/build/openmp
GCC_BUILD=$FRAMEWORK_PATH/build/gcc
LLVM_BIN=$FRAMEWORK_PATH/opt/llvm
GCC_BIN=$FRAMEWORK_PATH/opt/gcc

echo "LLVM_SRC = $LLVM_SRC"
echo "GCC_SRC  = $GCC_SRC"
echo "LLVM_BIN = $LLVM_BIN"
echo "GCC_BIN  = $GCC_BIN"

MKDIR $FRAMEWORK_PATH/src
MKDIR $FRAMEWORK_PATH/opt
MKDIR $FRAMEWORK_PATH/build
MKDIR $FRAMEWORK_PATH/device

if [ ! -f $FRAMEWORK_PATH/device/makeDeviceCapability.mk ]
then
  cp makeDeviceCapability.mk $FRAMEWORK_PATH/device/
  cp deviceArch.cu $FRAMEWORK_PATH/device/
  cp deviceCapability.cu $FRAMEWORK_PATH/device/
fi
cd $FRAMEWORK_PATH

# Check make
CUR_CMAKE=`cmake --version | head -n1 | awk '{print $3}'`
REQ_CMAKE=3.13.4
if [ "$(printf '%s\n' "$CUR_CMAKE" "$REQ_CMAKE" | \
     sort -V | head -n1)" = "$REQ_CMAKE" ];
then
  echo "cmake version $CUR_CMAKE found"
else
  echo "Minimum cmake version $REQ_CMAKE is required"
  exit
fi

# Check CUDA
if ! type nvcc > /dev/null 2>&1
then
  echo "CUDA not installed"
  exit
fi
CUR_CUDA=`nvcc --version | grep release | \
         awk -F', ' '{print $3}' | sed 's/^.//'`
echo "cuda version $CUR_CUDA found"

# Build DeviceCapability
cd device
make -f makeDeviceCapability.mk
ARCH=`./deviceArch`
CAPABILITY=`./deviceCapability`
echo "ARCH=$ARCH"
echo "CAPABILITY=$CAPABILITY"
cd $FRAMEWORK_PATH

if [ "$INSTALL_GCC" = true ]
then
  echo
  echo "Installing GCC 7.3.0 ..."
  echo "cd $GCC_SRC"
  echo "curl -JLO https://ftp.gnu.org/gnu/gcc/gcc-7.3.0/gcc-7.3.0.tar.xz"
  echo "tar xf gcc-7.3.0.tar.xz"
  echo "cd gcc-7.3.0"
  echo "./contrib/download_prerequisites"
  echo "MKDIR $GCC_BUILD && cd $GCC_BUILD"
  echo "$GCC_SRC/gcc-7.3.0/configure \\"
  echo "       --prefix=$GCC_BIN \\"
  echo "       --enable-shared  \\"
  echo "       --enable-languages=c,c++,lto \\"
  echo "       --enable-__cxa_atexit \\"
  echo "       --enable-threads=posix \\"
  echo "       --enable-checking=release \\"
  echo "       --disable-nls \\"
  echo "       --disable-multilib \\"
  echo "       --disable-bootstrap \\"
  echo "       --disable-libssp \\"
  echo "       --disable-libgomp \\"
  echo "       --disable-libsanitizer \\"
  echo "       --disable-libstdcxx-pch \\"
  echo "       --with-system-zlib"
  echo "make -j $MAKE_JOBS && make install && \\"
  echo -ne "    sed -i \"s/^set root .*/set root ${GCC_BIN//\//\\/}/\""
  echo " $GCC_SRC/module/gcc-7.3.0 && \\"
  echo "    export MODULEPATH=$GCC_SRC/module:$MODULEPATH && \\"
  echo "    module unload gcc && module load gcc-7.3.0"
  if [ "$PRINT" = false ]
  then
    cd $GCC_SRC
    if [ ! -f gcc-7.3.0.tar.xz ]
    then
      curl -JLO https://ftp.gnu.org/gnu/gcc/gcc-7.3.0/gcc-7.3.0.tar.xz
      tar xf gcc-7.3.0.tar.xz
      cd gcc-7.3.0
      ./contrib/download_prerequisites
    fi
    MKDIR $GCC_BUILD && cd $GCC_BUILD
    $GCC_SRC/gcc-7.3.0/configure        \
           --prefix=$GCC_BIN            \
           --enable-shared              \
           --enable-languages=c,c++,lto \
           --enable-__cxa_atexit        \
           --enable-threads=posix       \
           --enable-checking=release    \
           --disable-nls                \
           --disable-multilib           \
           --disable-bootstrap          \
           --disable-libssp             \
           --disable-libgomp            \
           --disable-libsanitizer       \
           --disable-libstdcxx-pch      \
           --with-system-zlib
    make -j $MAKE_JOBS && make install && \
      sed -i "s/^set root .*/set root ${GCC_BIN//\//\\/}/" $GCC_SRC/module/gcc-7.3.0 && \
      export MODULEPATH=$GCC_SRC/module:$MODULEPATH && \
      module unload gcc && module load gcc-7.3.0
    cd $FRAMEWORK_PATH
  fi
fi

# GCC version
CUR_GCC=`gcc -dumpversion`
REQ_GCC="6.0.0"
if [ "$(printf '%s\n' "$REQ_GCC" "$CUR_GCC" | \
     sort -V | head -n1)" = "$REQ_GCC" ]; then
  echo "GCC version ${CUR_GCC} found"
else
  echo "Need gcc version greater than ${REQ_GCC}"
  exit 1
fi
GCC_TOOLCHAIN=`which gcc | sed 's/\/bin\/gcc$//'`

# Clone LLVM Project
echo
echo "Cloning LLVM ..."
echo "git clone --depth 1 https://github.com/llvm/llvm-project.git $LLVM_SRC"
if [ "$PRINT" = false ]
then
  if [ ! -d $LLVM_SRC ]
  then
    git clone --depth 1 https://github.com/llvm/llvm-project.git $LLVM_SRC
  fi
fi

echo
echo "Building Clang ..."
echo "cmake -S $LLVM_SRC/llvm \\"
echo "      -B $CLANG_BUILD \\"
echo "      -DCMAKE_INSTALL_PREFIX=$LLVM_BIN \\"
echo "      -DCMAKE_BUILD_TYPE=\"Release\" \\"
echo "      -DLLVM_TARGETS_TO_BUILD=\"X86;NVPTX\" \\"
echo "      -DCMAKE_EXE_LINKER_FLAGS=\"-s\" \\"
echo "      -DCMAKE_C_COMPILER=$GCC_TOOLCHAIN/bin/gcc \\"
echo "      -DCMAKE_CXX_COMPILER=$GCC_TOOLCHAIN/bin/g++ \\"
echo "      -DGCC_INSTALL_PREFIX=$GCC_TOOLCHAIN \\"
echo "      -DCLANG_OPENMP_NVPTX_DEFAULT_ARCH=$ARCH \\"
echo "      -DLIBOMPTARGET_NVPTX_COMPUTE_CAPABILITIES=$CAPABILITY \\"
echo "      -DLLVM_ENABLE_PROJECTS=\"clang\" \\"
echo "      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \\"
echo "      -DLIBOMPTARGET_NVPTX_ALTERNATE_HOST_COMPILER=$GCC_TOOLCHAIN/bin/gcc"
echo "make -C $CLANG_BUILD -j$MAKE_JOBS install"
if [ "$PRINT" = false ]
then
  if [ ! -f "$CLANG_BUILD/Makefile" ]
  then
    cmake -S $LLVM_SRC/llvm \
          -B $CLANG_BUILD \
          -DCMAKE_INSTALL_PREFIX=$LLVM_BIN \
          -DCMAKE_BUILD_TYPE="Release" \
          -DLLVM_TARGETS_TO_BUILD="X86;NVPTX" \
          -DCMAKE_EXE_LINKER_FLAGS="-s" \
          -DCMAKE_C_COMPILER=$GCC_TOOLCHAIN/bin/gcc \
          -DCMAKE_CXX_COMPILER=$GCC_TOOLCHAIN/bin/g++ \
          -DGCC_INSTALL_PREFIX=$GCC_TOOLCHAIN \
          -DCLANG_OPENMP_NVPTX_DEFAULT_ARCH=$ARCH \
          -DLIBOMPTARGET_NVPTX_COMPUTE_CAPABILITIES=$CAPABILITY \
          -DLLVM_ENABLE_PROJECTS="clang" \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
          -DLIBOMPTARGET_NVPTX_ALTERNATE_HOST_COMPILER=$GCC_TOOLCHAIN/bin/gcc
  fi
  make -C $CLANG_BUILD -j $MAKE_JOBS install
fi

echo
echo "Building OpenMP ..."
echo "cmake -S $LLVM_SRC/openmp \\"
echo "      -B $OPENMP_BUILD \\"
echo "      -DCMAKE_INSTALL_PREFIX=$LLVM_BIN \\"
echo "      -DCMAKE_BUILD_TYPE=\"Release\" \\"
echo "      -DCMAKE_EXE_LINKER_FLAGS=\"-s\" \\"
echo "      -DCMAKE_C_COMPILER=$LLVM_BIN/bin/clang \\"
echo "      -DCMAKE_CXX_COMPILER=$LLVM_BIN/bin/clang++ \\"
echo "      -DLIBOMP_INSTALL_ALIASES=OFF \\"
echo "      -DLIBOMPTARGET_NVPTX_COMPUTE_CAPABILITIES=$CAPABILITY \\"
echo "      -DLIBOMPTARGET_NVPTX_ALTERNATE_HOST_COMPILER=$GCC_TOOLCHAIN/bin/gcc"
echo "make -C $OPENMP_BUILD -j$MAKE_JOBS install"
if [ "$PRINT" = false ]
then
  if [ ! -f "$OPENMP_BUILD/Makefile" ]
  then
    cmake -S $LLVM_SRC/openmp \
          -B $OPENMP_BUILD \
          -DCMAKE_INSTALL_PREFIX=$LLVM_BIN \
          -DCMAKE_BUILD_TYPE="Release" \
          -DCMAKE_EXE_LINKER_FLAGS="-s" \
          -DCMAKE_C_COMPILER=$LLVM_BIN/bin/clang \
          -DCMAKE_CXX_COMPILER=$LLVM_BIN/bin/clang++ \
          -DLIBOMP_INSTALL_ALIASES=OFF \
          -DLIBOMPTARGET_NVPTX_COMPUTE_CAPABILITIES=$CAPABILITY \
          -DLIBOMPTARGET_NVPTX_ALTERNATE_HOST_COMPILER=$GCC_TOOLCHAIN/bin/gcc
  fi
  make -C $OPENMP_BUILD -j $MAKE_JOBS install
fi
