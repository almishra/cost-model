usage()
{
  echo "usage: ./install [[[--prefix <PREFIX> ] [-gcc | --with-gcc]] | [-h]]"
}


MAKE_JOBS=4
while [ "$1" != "" ]; do
  case $1 in  
      --prefix )           shift
                           PREFIX="$1"
                           ;;
      -gcc | --with-gcc )  INSTALL_GCC=true
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

cd $FRAMEWORK_PATH
mkdir src opt build

if [ "$INSTALL_GCC" = true ]
then
  cd $GCC_SRC
  curl -JLO https://ftp.gnu.org/gnu/gcc/gcc-7.3.0/gcc-7.3.0.tar.xz
  tar xf gcc-7.3.0.tar.xz
  cd gcc-7.3.0
  ./contrib/download_prerequisites
  mkdir $GCC_BUILD
  cd $GCC_BUILD
  $GCC_SRC/gcc-7.3.0/configure \
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
  make -j $MAKE_JOBS install
  sed -i 's/<<<ROOT>>>/$GCC_BIN/' $GCC_SRC/module/gcc-7.3.0
  export MODULEPATH=$GCC_SRC/module:$MODULEPATH
  module load gcc-7.3.0
  cd $FRAMEWORK_PATH
fi

# Check make
CUR_CMAKE=`cmake --version | head -n1 | awk '{print $3}'`
REQ_CMAKE=3.13.4
if [ "$(printf '%s\n' "$CUR_CMAKE" "$REQ_CMAKE" | sort -V | head -n1)" = "$REQ_CMAKE" ]; 
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
CUR_CUDA=`nvcc --version | grep release | awk -F', ' '{print $3}' | sed 's/^.//'`
echo "cuda version $CUR_CUDA found"

# Build DeviceCapability
make -f makeDeviceCapability.mk
ARCH=`./deviceArch`
CAPABILITY=`./deviceCapability`

echo "ARCH=$ARCH"
echo "CAPABILITY=$CAPABILITY"

# GCC version
CUR_GCC=`gcc -dumpversion`
REQ_GCC="6.0.0"
if [ "$(printf '%s\n' "$REQ_GCC" "$CUR_GCC" | sort -V | head -n1)" = "$REQ_GCC" ]; then
  echo "GCC version ${CUR_GCC} found"
else
  echo "Need gcc version greater than ${REQ_GCC}"
  exit 1
fi
GCC_TOOLCHAIN=`which gcc | sed 's/\/bin\/gcc$//'`

# Clone Project
git clone --depth 1 https://github.com/llvm/llvm-project.git $LLVM_SRC

if [ ! -d $CLANG_BUILD ]
then
  mkdir -p $CLANG_BUILD 
fi
if [ ! -d $OPENMP_BUILD ]
then
  mkdir -p $OPENMP_BUILD
fi

echo "cmake -S $LLVM_SRC/llvm -B CLANG_BUILD -DCMAKE_INSTALL_PREFIX=$BIN -DCMAKE_BUILD_TYPE=\"Release\" -DLLVM_TARGETS_TO_BUILD=\"X86;NVPTX\" -DCMAKE_EXE_LINKER_FLAGS=\"-s\" -DCMAKE_C_COMPILER=$GCC_TOOLCHAIN/bin/gcc -DCMAKE_CXX_COMPILER=$GCC_TOOLCHAIN/bin/g++ -DGCC_INSTALL_PREFIX=$GCC_TOOLCHAIN -DLLVM_ENABLE_PROJECTS=\"clang\" -DLIBOMP_INSTALL_ALIASES=OFF -DCLANG_OPENMP_NVPTX_DEFAULT_ARCH=$ARCH -DLIBOMPTARGET_NVPTX_COMPUTE_CAPABILITIES=$CAPABILITY -DLIBOMPTARGET_NVPTX_ALTERNATE_HOST_COMPILER=$GCC_TOOLCHAIN/bin/gcc"
cmake -S $LLVM_SRC/llvm \
    -B $CLANG_BUILD \
    -DCMAKE_INSTALL_PREFIX=$BIN \
    -DCMAKE_BUILD_TYPE="Release" \
    -DLLVM_TARGETS_TO_BUILD="X86;NVPTX" \
    -DCMAKE_EXE_LINKER_FLAGS="-s" \
    -DCMAKE_C_COMPILER=$GCC_TOOLCHAIN/bin/gcc \
    -DCMAKE_CXX_COMPILER=$GCC_TOOLCHAIN/bin/g++ \
    -DGCC_INSTALL_PREFIX=$GCC_TOOLCHAIN \
    -DLLVM_ENABLE_PROJECTS="clang" \
    -DLIBOMPTARGET_NVPTX_ALTERNATE_HOST_COMPILER=$GCC_TOOLCHAIN/bin/gcc
make -C $CLANG_BUILD -j28 install

echo "cmake -S $LLVM_SRC/openmp -B OPENMP_BUILD -DCMAKE_INSTALL_PREFIX=$BIN -DCMAKE_BUILD_TYPE=\"Release\" -DCMAKE_EXE_LINKER_FLAGS=\"-s\" -DCMAKE_C_COMPILER=$BIN/bin/clang -DCMAKE_CXX_COMPILER=$BIN/bin/clang++ -DLIBOMP_INSTALL_ALIASES=OFF -DLIBOMPTARGET_NVPTX_COMPUTE_CAPABILITIES=$CAPABILITY -DLIBOMPTARGET_NVPTX_ALTERNATE_HOST_COMPILER=$GCC_TOOLCHAIN/bin/gcc"
cmake -S $LLVM_SRC/openmp \
    -B $OPENMP_BUILD \
    -DCMAKE_INSTALL_PREFIX=$BIN \
    -DCMAKE_BUILD_TYPE="Release" \
    -DCMAKE_EXE_LINKER_FLAGS="-s" \
    -DCMAKE_C_COMPILER=$BIN/bin/clang \
    -DCMAKE_CXX_COMPILER=$BIN/bin/clang++ \
    -DLIBOMP_INSTALL_ALIASES=OFF \
    -DLIBOMPTARGET_NVPTX_COMPUTE_CAPABILITIES=$CAPABILITY \
    -DLIBOMPTARGET_NVPTX_ALTERNATE_HOST_COMPILER=$GCC_TOOLCHAIN/bin/gcc
make -C $OPENMP_BUILD -j28 install
