if( NOT LLVM_REQUIRES_RTTI )
  if( NOT LLVM_REQUIRES_EH )
    set(LLVM_EXPORTED_SYMBOL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/LV.exports)
  endif()
endif()


add_llvm_loadable_module( LLVMClv
  LV.cpp

  PLUGIN_TOOL
  opt
  )
