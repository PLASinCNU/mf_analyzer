# If we don't need RTTI or EH, there's no reason to export anything
# from the hello plugin.

add_library( LLVMKubera  MODULE
	mf.cpp
)
