#include <iostream>
#include <map>
#include <set>
#include <algorithm>
#include "llvm/IR/DebugInfo.h"
#include "llvm/Pass.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"  
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h" 
#include "llvm/Analysis/Interval.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/User.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/Analysis/TargetLibraryInfo.h" 
#include "llvm/Support/Casting.h" 
#include "llvm/IR/GlobalValue.h" 
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/CallGraphSCCPass.h"

using namespace llvm;
using namespace std;


// Refer this web site: https://cpp.hotexamples.com/examples/-/CallGraphSCCPass/-/cpp-callgraphsccpass-class-examples.html
namespace{
	struct KuberaPass : public CallGraphSCCPass{
		static char ID; 

		KueraPass() :CallGraphSCCPass(ID) {}

		bool runOnSCC(CallGraphSCC& SCC) override{
			errs() << " --- Enter Call Graph SCC ---\n";
			for (CallGraphSCC::iterator I = SCC.begin(), E = SCC.end();I != E; ++I) {
				if (Function *F = (*I)->getFunction()) {
					{
						errs() << F->getName() <<"\n";
					}
				}
			}
			errs() << " --- end of CallGraphSCC ---\n";
			return false;
		}
	}
