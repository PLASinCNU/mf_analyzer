if( NOT LLVM_REQUIRES_RTTI )
  if( NOT LLVM_REQUIRES_EH )
    set(LLVM_EXPORTED_SYMBOL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/testHello.exports)
  endif()
endif()


add_llvm_library( LLVMTestHello  MODULE BUILDTREE_ONLY
  testHello.cpp

  PLUGIN_TOOL
  opt
  )
