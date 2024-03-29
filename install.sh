#!/bin/bash
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

function CREATE_MODULE_FILE() {
  if [[ $# -lt 4 ]]
  then
    echo "Wrong number of arguments in CREATE_MODULE_FILE"
    exit
  fi

  HELP=""
  WHATIS=""
  if [[ $# -gt 4 ]]
  then
    HELP="proc ModulesHelp { } {
  puts stderr \\
\"$5\"
}"
    if [[ $# -gt 4 ]]
    then
      WHATIS="module-whatis \"$6\""
    fi
  fi

  cat << EOF > $MAIN_DIR/modulefile/$2
#%Module -*- tcl -*-
##
## modulefile
##

$HELP
$WHATIS

proc prepend-include-path { dir } {
    prepend-path CMAKE_INCLUDE_PATH    \$dir
    prepend-path CPATH                 \$dir
    prepend-path CPLUS_INCLUDE_PATH    \$dir
    prepend-path C_INCLUDE_PATH        \$dir
    prepend-path INCLUDE               \$dir
    prepend-path INCLUDEPATH           \$dir
    prepend-path INCLUDE_PATH          \$dir
    prepend-path OBJC_INCLUDE_PATH     \$dir
    prepend-path OBJC_PATH             \$dir
}
proc prepend-lib-path { dir } {
    prepend-path CMAKE_LIBRARY_PATH    \$dir
    prepend-path LD_LIBRARY_PATH       \$dir
    prepend-path LD_RUN_PATH           \$dir
    prepend-path LIBRARY_PATH          \$dir
}

set   root  $3

conflict $1
prepend-include-path \$root/include
prepend-lib-path     \$root/lib:\$root/lib64
prepend-path      INFOPATH             \$root/share/info
prepend-path      MANPATH              \$root/share/man
prepend-path      PATH                 \$root/bin

setenv ${1^^}_VERSION $4
setenv ${1^^}_PATH    \$root
setenv ${1^^}_BIN     \$root/bin
setenv ${1^^}_LIB     \$root/lib
EOF
}

function ASSERT() {
  if [[ $# -ne 2 ]]
  then
    printf "\033[0;31mWrong number of arguments to ASSERT. (Usage: ASSERT <<return value>> <<error message>>\033[0m\n"
    exit 1
  fi

  if [[ $1 -ne 0 ]]
  then
    printf "\033[0;31m $2 \033[0m\n\n"
    exit 1
  fi
}

PRINT=false
MAIN_DIR=$PWD
MAKE_JOBS=4

while [ "$1" != "" ]; do
  case $1 in
      --prefix )           shift
                           PREFIX="$1"
                           if [[ ! $PREFIX = '/'* ]]; then
                             PREFIX="$PWD/$PREFIX";
                           fi
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

# Initialize path variables
LLVM_SRC=$FRAMEWORK_PATH/src/llvm
GCC_SRC=$FRAMEWORK_PATH/src/gcc
LLVM_BUILD=$FRAMEWORK_PATH/build/clang
GCC_BUILD=$FRAMEWORK_PATH/build/gcc
LLVM_BIN=$FRAMEWORK_PATH/opt/llvm
GCC_BIN=$FRAMEWORK_PATH/opt/gcc

if [ $PRINT = true ]
then
  echo "LLVM_SRC = $LLVM_SRC"
  echo "GCC_SRC  = $GCC_SRC"
  echo "LLVM_BUILD = $LLVM_BUILD"
  echo "GCC_BUILD  = $GCC_BUILD"
  echo "LLVM_BIN = $LLVM_BIN"
  echo "GCC_BIN  = $GCC_BIN"
fi

MKDIR $FRAMEWORK_PATH/src
MKDIR $FRAMEWORK_PATH/opt
MKDIR $FRAMEWORK_PATH/build
MKDIR $LLVM_BUILD
MKDIR $MAIN_DIR/modulefile

if [[ `echo ":$MODULEPATH:" | grep -c ":$MAIN_DIR/modulefile:" ` -eq 0 ]]
then
  export MODULEPATH=$MAIN_DIR/modulefile:$MODULEPATH
fi

# Check make
CUR_CMAKE=`cmake --version | head -n1 | awk '{print $3}'`
REQ_CMAKE=3.13.4
if [ "$(printf '%s\n' "$CUR_CMAKE" "$REQ_CMAKE" | \
     sort -V | head -n1)" = "$REQ_CMAKE" ];
then
  if [ "$PRINT" = true ]
  then
    echo "cmake version $CUR_CMAKE found"
  fi
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
if [ "$PRINT" = true ]
then
  echo "cuda version $CUR_CUDA found"
fi

# Build DeviceCapability
cd device
make
ARCH=`./deviceArch`
CAPABILITY=`./deviceCapability`
if [ "$PRINT" = true ]
then
  echo "CUDA ARCH=$ARCH"
  echo "CUDA COMPUTE CAPABILITY=$CAPABILITY"
fi
cd $FRAMEWORK_PATH

if [ "$INSTALL_GCC" = true ]
then
  if [ "$PRINT" = true ]
  then
    cat << EOF
Installing GCC 7.3.0 ...
cd $GCC_SRC
curl -JLO https://ftp.gnu.org/gnu/gcc/gcc-7.3.0/gcc-7.3.0.tar.xz
tar xf gcc-7.3.0.tar.xz
cd gcc-7.3.0
./contrib/download_prerequisites"
mkdir -p $GCC_BUILD && cd $GCC_BUILD
$GCC_SRC/gcc-7.3.0/configure        \\
       --prefix=$GCC_BIN            \\
       --enable-shared              \\
       --enable-languages=c,c++,lto \\
       --enable-__cxa_atexit        \\
       --enable-threads=posix       \\
       --enable-checking=release    \\
       --disable-nls                \\
       --disable-multilib           \\
       --disable-bootstrap          \\
       --disable-libssp             \\
       --disable-libgomp            \\
       --disable-libsanitizer       \\
       --disable-libstdcxx-pch      \\
       --with-system-zlib 2> $GCC_BUILD/.cmake_error > $GCC_BUILD/.cmake_log
make -j $MAKE_JOBS 2> $GCC_BUILD/.build_error > $GCC_BUILD/.build_log
make install 2> $GCC_BUILD/.install_error > $GCC_BUILD/.install_log
EOF
  else
    MKDIR $GCC_SRC
    cd $GCC_SRC
    if [ ! -f gcc-7.3.0.tar.xz ]
    then
      printf "\033[0;33mGetting gcc-7.3.0.tar.xz \033[0m"
      curl -JLO https://ftp.gnu.org/gnu/gcc/gcc-7.3.0/gcc-7.3.0.tar.xz
      printf "\033[0;33mUntar gcc-7.3.0.tar.xz \033[0m"
      tar xf gcc-7.3.0.tar.xz
      cd gcc-7.3.0
      printf "\r\033[0;33mDownloading prerequisite\033[0m"
      ./contrib/download_prerequisites 2>&1 > $GCC_SRC/.download_prerequisites.log
      ASSERT "$?" "==== download_prerequisites failed for gcc"
    fi
    printf "\033[0;33mConfiguring GCC-7.3.0\033[0m\nPlease wait.";
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
           --with-system-zlib 2> $GCC_BUILD/.cmake_error > $GCC_BUILD/.cmake_log
    ASSERT "$?" "==== Error occured in configuring GCC.\n ===== Please check cmake error log in \033[0;33m$GCC_BUILD/.cmake_error"

    printf "\r\033[0;33mBuilding GCC-7.3.0\033[0m\nPlease wait.";
    make -j $MAKE_JOBS 2> $GCC_BUILD/.build_error > $GCC_BUILD/.build_log
    ASSERT "$?" "==== Error occured in Building GCC.\n ===== Please check cmake error log in \033[0;33m$GCC_BUILD/.build_error"

    printf "\r\033[0;33mInstalling GCC-7.3.0\033[0m\nPlease wait.";
    make install 2> $GCC_BUILD/.install_error > $GCC_BUILD/.install_log
    ASSERT "$?" "==== Error occured in Installing GCC.\n ===== Please check cmake error log in \033[0;33m$GCC_BUILD/.install_error"

    printf "\rGCC-7.3.0 installed\n\n"
    CREATE_MODULE_FILE "gcc" "gcc-7.3.0" $GCC_BIN "7.3.0"  "The gcc package contains the GNU Compiler Collection version 7.3.0.
You'll need this package in order to compile C, C++, and Fortran code." "Various compilers (C, C++, Fortran, ...)"
    module unload gcc && module load gcc-7.3.0
    cd $FRAMEWORK_PATH
  fi
fi

# GCC version
CUR_GCC=`gcc -dumpversion`
REQ_GCC="6.0.0"
if [ "$(printf '%s\n' "$REQ_GCC" "$CUR_GCC" | \
     sort -V | head -n1)" != "$REQ_GCC" ]; then
  echo "Need gcc version greater than ${REQ_GCC}"
  exit 1
fi

if [ "$PRINT" = true ]
then
  echo "GCC version ${CUR_GCC} found"
fi
GCC_TOOLCHAIN=`which gcc | sed 's/\/bin\/gcc$//'`

# Clone LLVM Project
if [ "$PRINT" = true ]
then
  echo "git clone --depth 1 https://github.com/llvm/llvm-project.git $LLVM_SRC"
else
  printf "\033[0;33mCloning LLVM ... \033[0m"
  if [ ! -d $LLVM_SRC ]
  then
    git clone -b release/14.x --depth 1 https://github.com/llvm/llvm-project.git $LLVM_SRC > $MAIN_DIR/.llvm_clone_log 2> $MAIN_DIR/.llvm_clone_error
    ASSERT $? "==== Error in cloning llvm"

    # Copy the InstructionCount project to clang example directory
    cp -r $MAIN_DIR/InstructionCount $LLVM_SRC/clang/examples/InstructionCount
    echo "add_subdirectory(InstructionCount)" >> $LLVM_SRC/clang/examples/CMakeLists.txt
    echo
  else
    printf "LLVM already exists\n"
    # Copy the InstructionCount project to clang example directory
    cp -r $MAIN_DIR/InstructionCount/*.cpp $LLVM_SRC/clang/examples/InstructionCount/
    cp -r $MAIN_DIR/InstructionCount/*.h $LLVM_SRC/clang/examples/InstructionCount/
  fi
fi

if [ "$PRINT" = true ]
then
  cat << EOF
cmake -S $LLVM_SRC/llvm \\
      -B $LLVM_BUILD \\
      -DCMAKE_INSTALL_PREFIX=$LLVM_BIN \\
      -DCMAKE_BUILD_TYPE="Release" \\
      -DLLVM_TARGETS_TO_BUILD="X86;NVPTX" \\
      -DCMAKE_EXE_LINKER_FLAGS="-s" \\
      -DCMAKE_C_COMPILER=$GCC_TOOLCHAIN/bin/gcc \\
      -DCMAKE_CXX_COMPILER=$GCC_TOOLCHAIN/bin/g++ \\
      -DGCC_INSTALL_PREFIX=$GCC_TOOLCHAIN \\
      -DCLANG_OPENMP_DONE_NVPTX_DEFAULT_ARCH=$ARCH \\
      -DLIBOMPTARGET_NVPTX_COMPUTE_CAPABILITIES=$CAPABILITY \\
      -DLLVM_ENABLE_PROJECTS="clang" \\
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \\
      -DCLANG_BUILD_EXAMPLES=1 \\
      -DLLVM_ENABLE_RUNTIMES="openmp" \\
      -DOPENMP_DONE_ENABLE_LIBOMPTARGET=ON \\
      -DLIBOMPTARGET_NVPTX_ALTERNATE_HOST_COMPILER=$GCC_TOOLCHAIN/bin/gcc > $LLVM_BUILD/.cmake_log 2> $LLVM_BUILD/.cmake_error
make -C $LLVM_BUILD -j$MAKE_JOBS install
EOF
else
  if [ ! -f "$LLVM_BUILD/Makefile" ]
  then
    printf "\033[0;33mConfiguring LLVM ...\n\033[0m";
    >$LLVM_BUILD/.cmake_log
    >$LLVM_BUILD/.cmake_error
    cmake -S $LLVM_SRC/llvm \
          -B $LLVM_BUILD \
          -DCMAKE_INSTALL_PREFIX=$LLVM_BIN \
          -DCMAKE_BUILD_TYPE="Release" \
          -DLLVM_TARGETS_TO_BUILD="X86;NVPTX" \
          -DCMAKE_EXE_LINKER_FLAGS="-s" \
          -DCMAKE_C_COMPILER=$GCC_TOOLCHAIN/bin/gcc \
          -DCMAKE_CXX_COMPILER=$GCC_TOOLCHAIN/bin/g++ \
          -DGCC_INSTALL_PREFIX=$GCC_TOOLCHAIN \
          -DCLANG_OPENMP_DONE_NVPTX_DEFAULT_ARCH=$ARCH \
          -DLIBOMPTARGET_NVPTX_COMPUTE_CAPABILITIES=$CAPABILITY \
          -DLLVM_ENABLE_PROJECTS="clang" \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
          -DCLANG_BUILD_EXAMPLES=1 \
          -DLLVM_ENABLE_RUNTIMES="openmp" \
          -DOPENMP_DONE_ENABLE_LIBOMPTARGET=ON \
          -DLIBOMPTARGET_NVPTX_ALTERNATE_HOST_COMPILER=$GCC_TOOLCHAIN/bin/gcc  >> $LLVM_BUILD/.cmake_log 2>> $LLVM_BUILD/.cmake_error &
    CMAKE_PID=$!
    while :
    do
      for s in / - \\ \|
      do
        printf "\r$s Please wait... $CMAKE_PID Log file - $LLVM_BUILD/.cmake_log"
        sleep 1
        kill -0 $CMAKE_PID 2> /dev/null
        if [ $? -gt "0" ]
        then
          wait $CMAKE_PID 2> /dev/null
          RET=$?
          printf "\r                                                                                                                       \r"
          ASSERT $RET "==== Error occured.\n ===== Please check cmake error log in \033[0;33m$LLVM_BUILD/.cmake_error"
          break 2;
        fi
      done
    done
  fi

  >$LLVM_BUILD/.build_log
  >$LLVM_BUILD/.build_error
  make -C $LLVM_BUILD -j $MAKE_JOBS >> $LLVM_BUILD/.build_log 2>> $LLVM_BUILD/.build_error &
  printf "\r\033[0;33mBuilding Clang... \033[0m\n"
  PID=$!

  CLANG_DONE="false"
  OPENMP_DONE="false"
  TOTAL_PERCENTAGE=""
  while :
  do
    for s in / - \\ \|
    do
      printf "\r$s"
      sleep 1
      PERCENTAGE=`grep -a "\[*%\]" $LLVM_BUILD/.build_log | grep -v "Binary file" | tail -n 1  | cut --delimiter="]" -f 1 | cut --delimiter='[' -f 2`
      if [[ $PERCENTAGE != "" ]]
      then
        if [[ $CLANG_DONE = "false" ]]
        then
          if [[ $PERCENTAGE = "100%" ]]
          then
            CLANG_DONE="true"
            TOTAL_PERCENTAGE=`grep -c "\[*%\]" $LLVM_BUILD/.build_log`
          fi
          echo -ne " $PERCENTAGE done. Log File - $LLVM_BUILD/.build_log"
        elif [[ $OPENMP_DONE = "false" || $OPENMP_DONE = "config" ]]
        then
          x=`grep -c "Performing configure step for 'runtimes'" $LLVM_BUILD/.build_log`
          if [[ $x -gt 0 && $OPENMP_DONE = "false" ]]
          then
            OPENMP_DONE="config"
            printf "\r\033[0;33mConfiguring OpenMP... \033[0m\n";
          fi
          x=`grep -c "Performing build step for 'runtimes'" $LLVM_BUILD/.build_log`
          if [[ $x -gt 0 ]]
          then
            OPENMP_DONE="ON"
            printf "\r                                                                                                                                                             \r";
            printf "\r\033[0;33mBuilding OpenMP... \033[0m\n";
          fi
        elif [[ $OPENMP_DONE = "ON" ]]
        then
          PERCENTAGE=`grep "\[*%\]" $LLVM_BUILD/.build_log  | cut --delimiter="]" -f 1 | cut --delimiter='[' -f 2 | tail -n 1;`
          echo -ne " $PERCENTAGE done. Log File - $LLVM_BUILD/.build_log"
        fi
      fi
      kill -0 $PID 2> /dev/null
      if [ $? -gt "0" ]
      then
        wait $PID
        RET=$?
        printf "\r                                                                                                                        \r";
        ASSERT "$RET" "\033[0;31m ==== Error occured.\n ===== Please check build error log in \033[0;33m$LLVM_BUILD/.build_error \033[0m"
        break 2;
      fi
    done
  done

  >$LLVM_BUILD/.install_log
  >$LLVM_BUILD/.install_error
  make -C $LLVM_BUILD install >> $LLVM_BUILD/.install_log 2>> $LLVM_BUILD/.install_error &
  printf "\r\033[0;33mInstalling Clang... \033[0m\n"
  INSTALL_PID=$!
  while :
  do
    for s in / - \\ \|
    do
      printf "\r$s Please wait... Log file - $LLVM_BUILD/.install_log"
      sleep 1
      kill -0 $INSTALL_PID 2> /dev/null
      if [ $? -gt "0" ]
      then
        wait $INSTALL_PID
        RET=$?
        printf "\r                                                                                                                       \r"
        ASSERT "$RET" "==== Error occured.\n ===== Please check cmake error log in \033[0;33m$LLVM_BUILD/.cmake_error"
        break 2;
      fi
    done
  done

  CREATE_MODULE_FILE "clang" "clang-12.0.0" $LLVM_BIN "12.0.0" "" "LLVM Clang compiler version 12.0.0"
  echo "setenv COMPUTE_CAPABILITY $CAPABILITY" >> $MAIN_DIR/modulefile/clang-12.0.0
  printf "\r\n\033[0;32m Installation successfull\n"
  printf "\033[0m Module file to load clang is created in \033[0;33m$MAIN_DIR/modulefile/clang-12.0.0 \033[0mfile.\n"
  printf " Please load this module to start using clang. \n\n"
fi
