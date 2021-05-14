//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

namespace {
  struct testHello : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
      testHello() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
      errs() << "Helloooo: ";
      errs().write_escaped(F.getName()) << '\n';
      return false;
    }
  };
}

char testHello::ID = 0;
static RegisterPass<testHello> X("testHello", "testHello: Pass");
