#!/bin/sh

CC=/home/mok/project/HJ/llvm_install/bin/clang-8
CPP=/home/mok/project/HJ/llvm_install/bin/clang-8
OPT=/home/mok/project/HJ/llvm_install/bin/opt
LLVM_DIS=/home/mok/project/HJ/llvm_install/bin/llvm-dis

DIR=$(pwd)
TEST_DIR="$DIR/test"
BUILD_DIR="$DIR/build/"
TEST_BUILD_DIR="$BUILD_DIR/test"
BUILD_LIBRARY="$DIR/build/lib/libLLVMKubera.so"


if [ ! -d $TEST_BUILD_DIR]; then
    echo "make build directory";
    mkdir $TEST_BUILD_DIR
else
    echo "remove and remake build directory";
    rm -rf build/test
    mkdir $TEST_BUILD_DIR
fi
$CC -c -g -emit-llvm $TEST_DIR/CWE122_Heap_Based_Buffer_Overflow__char_type_overrun_memcpy_01.c -o $TEST_BUILD_DIR/CWE122_Heap_Based_Buffer_Overflow__char_type_overrun_memcpy_01.bc -DINCLUDEMAIN
$CC -c -g -emit-llvm $TEST_DIR/CWE122_Heap_Based_Buffer_Overflow__char_type_overrun_memcpy_01_revision_input.c -o $TEST_BUILD_DIR/CWE122_Heap_Based_Buffer_Overflow__char_type_overrun_memcpy_01_revision_input.bc -DINCLUDEMAIN

$CC -c -g -emit-llvm $TEST_DIR/CWE122_Heap_Based_Buffer_Overflow__cpp_CWE129_fgets_01.cpp -o $TEST_BUILD_DIR/CWE122_Heap_Based_Buffer_Overflow__cpp_CWE129_fgets_01.bc -DINCLUDEMAIN

$CC -c -g -emit-llvm $TEST_DIR/CWE122_Heap_Based_Buffer_Overflow__sizeof_int64_t_11.c -o $TEST_BUILD_DIR/CWE122_Heap_Based_Buffer_Overflow__sizeof_int64_t_11.bc -DINCLUDEMAIN
$CC -c -g -emit-llvm $TEST_DIR/CWE122_Heap_Based_Buffer_Overflow__sizeof_int64_t_11_revision_input.c -o $TEST_BUILD_DIR/CWE122_Heap_Based_Buffer_Overflow__sizeof_int64_t_11_revision_input.bc -DINCLUDEMAIN

$CC -c -g -emit-llvm $TEST_DIR/CWE789_Uncontrolled_Mem_Alloc__malloc_char_fgets_01.c -o $TEST_BUILD_DIR/CWE789_Uncontrolled_Mem_Alloc__malloc_char_fgets_01.bc -DINCLUDEMAIN

$CC -c -g -emit-llvm $TEST_DIR/CWE124_Buffer_Underwrite__CWE839_fgets_02.c -o $TEST_BUILD_DIR/CWE124_Buffer_Underwrite__CWE839_fgets_02.bc -DINCLUDEMAIN

$CC -c -g -emit-llvm $TEST_DIR/CWE126_Buffer_Overread__CWE129_fgets_01.c -o $TEST_BUILD_DIR/CWE126_Buffer_Overread__CWE129_fgets_01.bc -DINCLUDEMAIN

$CC -c -g -emit-llvm $TEST_DIR/CWE126_Buffer_Overread__new_char_loop_13.cpp -o $TEST_BUILD_DIR/CWE126_Buffer_Overread__new_char_loop_13.bc -DINCLUDEMAIN
$CC -c -g -emit-llvm $TEST_DIR/CWE126_Buffer_Overread__new_char_loop_13_revision_input.cpp -o $TEST_BUILD_DIR/CWE126_Buffer_Overread__new_char_loop_13_revision_input.bc -DINCLUDEMAIN



$OPT -load=$BUILD_LIBRARY $TEST_BUILD_DIR/CWE122_Heap_Based_Buffer_Overflow__char_type_overrun_memcpy_01.bc -an --num 42 --mapping-functio CWE122_Heap_Based_Buffer_Overflow__char_type_overrun_memcpy_01_bad
$OPT -load=$BUILD_LIBRARY $TEST_BUILD_DIR/CWE122_Heap_Based_Buffer_Overflow__char_type_overrun_memcpy_01_revision_input.bc -an --num 42 --mapping-functio CWE122_Heap_Based_Buffer_Overflow__char_type_overrun_memcpy_01_bad

$OPT -load=$BUILD_LIBRARY $TEST_BUILD_DIR/CWE122_Heap_Based_Buffer_Overflow__cpp_CWE129_fgets_01.bc -an --num 57 --mapping-functio bad

$OPT -load=$BUILD_LIBRARY $TEST_BUILD_DIR/CWE122_Heap_Based_Buffer_Overflow__sizeof_int64_t_11.bc -an --num 36 --mapping-functio CWE122_Heap_Based_Buffer_Overflow__sizeof_int64_t_11_bad
$OPT -load=$BUILD_LIBRARY $TEST_BUILD_DIR/CWE122_Heap_Based_Buffer_Overflow__sizeof_int64_t_11_revision_input.bc -an --num 35 --mapping-functio CWE122_Heap_Based_Buffer_Overflow__sizeof_int64_t_11_bad

$OPT -load=$BUILD_LIBRARY $TEST_BUILD_DIR/CWE789_Uncontrolled_Mem_Alloc__malloc_char_fgets_01.bc -an --num 58 --mapping-functio CWE789_Uncontrolled_Mem_Alloc__malloc_char_fgets_01_bad

$OPT -load=$BUILD_LIBRARY $TEST_BUILD_DIR/CWE124_Buffer_Underwrite__CWE839_fgets_02.bc -an --num 58 --mapping-functio CWE124_Buffer_Underwrite__CWE839_fgets_02_bad

$OPT -load=$BUILD_LIBRARY $TEST_BUILD_DIR/CWE126_Buffer_Overread__CWE129_fgets_01.bc -an --num 48 --mapping-functio CWE126_Buffer_Overread__CWE129_fgets_01_bad

$OPT -load=$BUILD_LIBRARY $TEST_BUILD_DIR/CWE126_Buffer_Overread__new_char_loop_13.bc -an --num 47 --mapping-functio bad
$OPT -load=$BUILD_LIBRARY $TEST_BUILD_DIR/CWE126_Buffer_Overread__new_char_loop_13_revision_input.bc -an --num 40 --mapping-functio bad

# $LLVM_DIS $TEST_BUILD_DIR/CWE122_Heap_Based_Buffer_Overflow__char_type_overrun_memcpy_01.bc
# $LLVM_DIS $TEST_BUILD_DIR/CWE122_Heap_Based_Buffer_Overflow__char_type_overrun_memcpy_01_revision_input.bc
# $LLVM_DIS $TEST_BUILD_DIR/WE122_Heap_Based_Buffer_Overflow__cpp_CWE129_fgets_01.bc
$LLVM_DIS $TEST_BUILD_DIR/CWE122_Heap_Based_Buffer_Overflow__sizeof_int64_t_11.bc
$LLVM_DIS $TEST_BUILD_DIR/CWE122_Heap_Based_Buffer_Overflow__sizeof_int64_t_11_revision_input.bc
# $LLVM_DIS $TEST_BUILD_DIR/CWE789_Uncontrolled_Mem_Alloc__malloc_char_fgets_01.bc
# $LLVM_DIS $TEST_BUILD_DIR/CWE124_Buffer_Underwrite__CWE839_fgets_02.bc
# $LLVM_DIS $TEST_BUILD_DIR/CWE126_Buffer_Overread__CWE129_fgets_01.bc
# $LLVM_DIS $TEST_BUILD_DIR/CWE126_Buffer_Overread__new_char_loop_13.bc
# $LLVM_DIS $TEST_BUILD_DIR/CWE126_Buffer_Overread__new_char_loop_13_revision_input.bc
