#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "myAccount"

namespace {
  struct MyAccount : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    MyAccount() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
      errs() << "Name : HyunJi Jung\n Account : 201850277\n";
      errs().write_escaped(F.getName()) << '\n';
      return false;
    }
  };
}

char MyAccount::ID = 0;
static RegisterPass<MyAccount> X("myAccount", "My Account Pass");
