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

# Clone Project
#git clone --depth 1 https://github.com/llvm/llvm-project.git 

FRAMEWORK_PATH=$PWD
LLVM_SRC=$FRAMEWORK_PATH/llvm-project
CLANG_BUILD=$FRAMEWORK_PATH/clang-build
OPENMP_BUILD=$FRAMEWORK_PATH/openmp-build
BIN=$FRAMEWORK_PATH/llvm-bin

if [ ! -d $CLANG_BUILD ]
then
  mkdir $CLANG_BUILD 
fi
if [ ! -d $OPENMP_BUILD ]
then
  mkdir $OPENMP_BUILD
fi
