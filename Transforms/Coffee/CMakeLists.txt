if( NOT LLVM_REQUIRES_RTTI )
  if( NOT LLVM_REQUIRES_EH )
    set(LLVM_EXPORTED_SYMBOL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Coffee.exports)
  endif()
endif()


add_llvm_library( LLVMCoffee  MODULE BUILDTREE_ONLY
  Coffee.cpp

  PLUGIN_TOOL
  opt
  )
