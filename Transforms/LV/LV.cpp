#define DEBUG_TYPE "lv"
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
#include "llvm/Analysis/Interval.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/DebugInfoMetadata.h"

using namespace llvm;
using namespace std;

namespace {
    struct LV : public BasicBlockPass {
        static char ID;     // Pass identification
        LV(): BasicBlockPass(ID) { }
        
        map<BasicBlock*, set<string>&> USE;
        map<BasicBlock*, set<string>&> DEF;
        map<BasicBlock*, set<string>&> IN;
        map<BasicBlock*, set<string>&> OUT;
        
  
        virtual bool runOnBasicBlock(BasicBlock &bb) {
            eval_USEDEF(&bb);
        }
        
        virtual bool  doInitialization(Function &f) {
            for (Function::iterator bb = f.begin(); bb != f.end(); bb++) {
                BasicBlock &temp = *bb;
                USE.insert(pair<BasicBlock*, set<string>&>(&temp, * new set<string>()));
                DEF.insert(pair<BasicBlock*, set<string>&>(&temp, * new set<string>()));
                IN.insert(pair<BasicBlock*, set<string>&>(&temp,  * new set<string>()));
                OUT.insert(pair<BasicBlock*, set<string>&>(&temp, * new set<string>()));
            }
            return true;
        }
        
        
        virtual bool doFinalization(Function &f) {
            eval_AllINOUT(f);
            
            errs() << "******* Live Variable Analysis Result *******\n";
//            errs() << " tesssssssssst : " << f.getName() << "\n";
            int bb_count = 0;
            for (Function::iterator bb = f.begin(); bb != f.end(); bb_count++,bb++) {
                BasicBlock &temp = *bb;
                 printUSEDEF(bb_count, &temp); // optional
                //printINOUT(bb_count, &temp);
            }
            errs() << "*******       end of the result       *******\n";
            
            return true;
        }
        
    private:  bool is_inDEF(BasicBlock * bb, string s){
        if (DEF.count(bb) < 1) return false; // throw exception
        
        set<string> & bbDEF = DEF.find(bb)->second;
        return (bbDEF.count(s) >= 1) ;
    }
        
        
    private: bool is_ignorableInst(Instruction* inst) {
        string opcodeName = inst->getOpcodeName();
        unsigned int numOperands = inst->getNumOperands();
        
        return
        (opcodeName == "br" && numOperands == 1)
        // unconditional branch : no src or dest
        || (opcodeName == "alloca") ;
        // alloca :  no src or dest
        
    }
        
    private: bool is_ignorableOprd(Instruction * inst, unsigned int i_oprd) {
        string opcodeName = inst->getOpcodeName();
        return opcodeName == "br" && i_oprd > 0;
        // conditional branch has multiple operands
        // only the first is considered "used", others are "labels"
        
        // to do : more cases
    }
        
    private: bool is_destOprd(Instruction * inst, unsigned int i_oprd) {
        string opcodeName = inst->getOpcodeName();
        return opcodeName == "store"  && i_oprd == 1;
        // store's 1'th operand (not 0'th) is "dest"
        // to do : more cases
    }
        
    private:  void eval_USEDEF(BasicBlock * bb) {
        if (USE.count(bb) < 1 || DEF.count(bb) < 1) return; // throw exception
        
        set<string> & bbUSE = USE.find(bb)->second;
        set<string> & bbDEF = DEF.find(bb)->second;
        
        
        for (BasicBlock::iterator inst = bb->begin(); inst != bb->end(); inst++) {
            Instruction &temp = *inst;
            if(is_ignorableInst(&temp)) continue;
            
            
//            errs() << " tesssssssssst : " << bb->getName() << "\n\n\n\n";
//
//            errs() << "test instruction : " << temp.getName() << "\n";
//            errs() << "instruction address : " << &temp << " \n\n\n\n";
////            DILocation *Loc = temp.getDebugLoc();
////            errs() << " testttttee222 : " << Loc << "\n";
//            //        ***heelreeee **
//            if (DILocation *Loc = temp.getDebugLoc()) { // Here I is an LLVM instruction
//                unsigned Line = Loc->getLine();
////                StringRef File = Loc->getFilename();
////                StringRef Dir = Loc->getDirectory();
//                errs() << "plxzzzzzzz : " << Line << "\n";
//            }
//            /*********/
            
            string dest = temp.getName();
            string opcodeName = temp.getOpcodeName();
            unsigned int numOperands = temp.getNumOperands();
            
            for (int i_oprd = 0; i_oprd < numOperands; i_oprd++) {
                if (is_ignorableOprd(&temp, i_oprd)) break;
                
                string src = temp.getOperand(i_oprd)->getName();
                
                if (is_destOprd(&temp, i_oprd))
                    dest = src;
                
                else if (!is_inDEF(bb, src))
                    bbUSE.insert(src);
            }
            
            bbDEF.insert(dest);
        }
    }
        
        
    private: set<BasicBlock*> * getSuccessors(BasicBlock *bb) {
        set<BasicBlock *> * succs = new set<BasicBlock*>();
        
        TerminatorInst * term_inst = bb->getTerminator();
        for (int i = 0; i < term_inst->getNumSuccessors(); i++)
            succs->insert(term_inst->getSuccessor(i));
        
        return succs;
    }
        
        
    private:  void eval_AllINOUT(Function &f){
        bool change = true;
        while (change) {
            change = false;
            for (Function::iterator bb = f.begin(); bb != f.end(); bb++) {
                BasicBlock &temp = *bb;
                if (USE.count(&temp) < 1 || DEF.count(&temp) < 1
                    || IN.count(&temp) < 1 || OUT.count(&temp) < 1) return; //exception
                
                set<string>& bbUSE = USE.find(&temp)->second;
                set<string>& bbDEF = DEF.find(&temp)->second;
                set<string>& bbIN = IN.find(&temp)->second;
                set<string>& bbOUT = OUT.find(&temp)->second;
                
                // OLD_IN = IN;
                set<string> old_in = bbIN;    // value by value copy
                
                // OUT = INs of successors
                set<BasicBlock*> * succrs = getSuccessors(&temp);

                for(set<BasicBlock*>::iterator succ = succrs->begin();
                    succ != succrs->end(); succ++){
                    set<string> succset = IN.find(*succ)->second;    // the successor's IN set
                    bbOUT.insert(succset.begin(), succset.end());    // add to OUT
                }
                
                // IN(X) = USE(X) + (OUT(X) - DEF(X))
                set<string>  newout;         // newout = OUT - DEF
                set_difference(bbOUT.begin(), bbOUT.end(),
                               bbDEF.begin(), bbDEF.end(),
                               std::inserter(newout, newout.end()));
                
                // IN = USE + (OUT-DEF)
                bbIN = bbUSE;                // value by value copy
                bbIN.insert(newout.begin(), newout.end() );
                
                if (old_in != bbIN)
                    change = true;
            }
        }
    }
        
        
    private:  void printVars(set<string> vars) {
        for( set<string>::iterator v = vars.begin(); v != vars.end(); v++)
            errs() << *v << "  " ;
        errs() << "\n";
    }
        
    private:  void printUSEDEF(int bb_number, BasicBlock *bb){
        if (USE.count(bb) < 1 || DEF.count(bb) < 1) return ; // throw exception
        
        errs() << "BASIC BLOCK[" << bb_number << "]" << "\n";
        
        errs() <<  "  USE  : ";
        printVars(USE.find(bb)->second);
        
        errs() <<  "  DEF : ";
        printVars(DEF.find(bb)->second);
        
    }
        
    private:  void printINOUT(int bb_number, BasicBlock *bb){
        if (IN.count(bb) < 1 || OUT.count(bb) < 1) return ; // throw exception
        
        errs() << "BASIC BLOCK[" << bb_number << "]" << "\n";
        
        errs() <<  "  IN  : ";
        printVars(IN.find(bb)->second);
        
        errs() <<  "  OUT : ";
        printVars(OUT.find(bb)->second);
        
    }
    };
}

char LV::ID = 1;
static RegisterPass<LV> X("lv", "LV Pass");
