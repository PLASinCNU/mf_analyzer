//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

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
#include "llvm/IR/Instruction.h" // Add Here Rete 2019.04.18
#include "llvm/Analysis/Interval.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/Analysis/TargetLibraryInfo.h" //Add Here Rete 2019.04.18
#include "llvm/Support/Casting.h" //Add Here Rete 2019.04.18
#include "llvm/IR/GlobalValue.h" //Add Here Rete 2019.04.18
#include "llvm/Analysis/CFG.h"


using namespace llvm;
using namespace std;

#define DEBUG_TYPE "Forward"


namespace {
  // Hello - The first implementation, without getAnalysisUsage.
  struct Forward : public BasicBlockPass {
      static char ID;     // Pass identification
      Forward(): BasicBlockPass(ID) { }
      
//      map<pair<Instruction *, string>, Instruction *> btd_table;
      map<BasicBlock *, Function *> functionTable;
      map<BasicBlock *, set<pair<Instruction *, string>>&> GEN;
//      map<BasicBlock *, set<pair<Instruction *, BasicBlock *>>&> KILL;
      map<BasicBlock *, set<pair<Instruction *, string>>&> KILL;
      map<BasicBlock*, set<pair<Instruction *, string>>&> IN;
      map<BasicBlock*, set<pair<Instruction *, string>>&> OUT;
      
      set<pair<Instruction *, string>> duChain;
      set<pair<Instruction *, string>> udChain;
//      map<BasicBlock*, set<pair<Instruction *, string>>&> udChain;
//      map<BasicBlock*, set<pair<Instruction *, string>>&> duChain;
      set<string> variables;
      
      virtual bool runOnBasicBlock(BasicBlock &bb) {
          eval_GENKILL(&bb);
      }
      virtual bool  doInitialization(Function &f) {
          for (Function::iterator bb = f.begin(); bb != f.end(); bb++) {
              BasicBlock &temp = *bb;
              functionTable.insert(pair<BasicBlock *, Function *>(&temp, &f));
//              udChain.insert(pair<BasicBlock *, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));
//              duChain.insert(pair<BasicBlock *, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));
              variables.insert("");
              GEN.insert(pair<BasicBlock *, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));
//              KILL.insert(pair<BasicBlock *, Instruction *>(&temp, NULL));
              KILL.insert(pair<BasicBlock *, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));
//              KILL.insert(pair<BasicBlock *, set<pair<Instruction *, BasicBlock *>>&>(&temp, * new set<pair<Instruction *, BasicBlock *>>()));
              IN.insert(pair<BasicBlock*, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));
              OUT.insert(pair<BasicBlock*, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));
              
          }
          return true;
      }
      
      /*****      TO DO : refactoring         *****/
      virtual bool doFinalization(Function &f) {
          eval_AllINOUT(f);
//          make_udchain(f);
          make_duchain(f);
          for(map<BasicBlock *, set<pair<Instruction *, string>>&>::iterator i = GEN.begin(); i != GEN.end() ; i++){
              errs() << "Basic Block : " << i->first << "\n";
              
              set<pair<Instruction *, string>> tmp = i->second;

              for(set<pair<Instruction *, string>>::iterator j = tmp.begin(); j != tmp.end(); j++){
                  unsigned line;
                  if((j->first)->getDebugLoc()){
                      DILocation *uLoc = (j->first)->getDebugLoc();
                      line = uLoc->getLine();
                  }
                  errs() << "GEN set : <BasicBlock * : " << i->first << " < Instruction * : " << j->first << ", name : "<< j->second << ", Line : "<< line<<" > > \n";
              }
          }
          
          errs() << "\n\n\n\n";
          
          for(map<BasicBlock *, set<pair<Instruction *, string>>&>::iterator i = KILL.begin(); i != KILL.end() ; i++){
              errs() << "KILL Basic Block : " << i->first << "\n";

              set<pair<Instruction *, string>> tmp = i->second;

//              errs() << "tmp is NULL? : " << tmp.empty() << "\n";
              for(set<pair<Instruction *, string>>::iterator j = tmp.begin(); j != tmp.end(); j++){
                  unsigned line;
                  if((j->first)->getDebugLoc()){
                      DILocation *uLoc = (j->first)->getDebugLoc();
                      line = uLoc->getLine();
                  }
                  errs() << "<BasicBlock * : " << i->first << ", < Instruction * : " << j->first << ", name : "<< j->second << ", Line : "<< line<<" > > \n";
              }
          }
          errs() << "\n\n\n\n";
          
          for(map<BasicBlock *, set<pair<Instruction *, string>>&>::iterator i = IN.begin(); i != IN.end() ; i++){
              errs() << "IN Basic Block : " << i->first << "\n";
              set<pair<Instruction *, string>> tmp = i->second;
              
//              errs() << "in  is NULL? : " << tmp.empty() << "\n";
              for(set<pair<Instruction *, string>>::iterator j = tmp.begin(); j != tmp.end(); j++){
                  unsigned line;
                  if((j->first)->getDebugLoc()){
                      DILocation *uLoc = (j->first)->getDebugLoc();
                      line = uLoc->getLine();
                  }
                  errs() << "<BasicBlock * : " << i->first << ", < Instruction * : " << j->first << ", name : "<< j->second << ", Line : "<< line<<" > > \n";
              }
          }
          errs() << "\n\n\n\n";
          
          for(map<BasicBlock *, set<pair<Instruction *, string>>&>::iterator i = OUT.begin(); i != OUT.end() ; i++){
              errs() << "OUT Basic Block : " << i->first << "\n";
              set<pair<Instruction *, string>> tmp = i->second;
              
              //              errs() << "in  is NULL? : " << tmp.empty() << "\n";
              for(set<pair<Instruction *, string>>::iterator j = tmp.begin(); j != tmp.end(); j++){
                  unsigned line;
                  if((j->first)->getDebugLoc()){
                      DILocation *uLoc = (j->first)->getDebugLoc();
                      line = uLoc->getLine();
                  }
                  errs() << "<BasicBlock * : " << i->first << ", < Instruction * : " << j->first << ", name : "<< j->second << ", Line : "<< line<<" > > \n";
              }
          }
          errs() << "\n\n\n\n";
          
          errs() << "IS size :: "<< duChain.size() << "\n";
          errs() << "IS NUM :: "<< duChain.empty() << "\n";
          
          for(set<pair<Instruction *, string>>::iterator ll = duChain.begin(); ll!= duChain.begin(); ll++){
              errs() << "IS NUM :: ";
              errs() << "Instruction : " << ll->first << ", TAG : " << ll->second << "\n";
          }
          
          errs() << "******* Backward Taing Analysis Result *******\n";
          int bb_count = 0;
          for (Function::iterator bb = f.begin(); bb != f.end(); bb_count++,bb++) {
              BasicBlock &temp = *bb;
              // printUSEDEF(bb_count, bb); // optional
//              printINOUT(bb_count, &temp);
          }
          
          //            printTable(btd_table);
          errs() << "\n\n";
          errs() << "*********       end of the result       *********\n";
          
          return true;
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
      
      return opcodeName == "store" && i_oprd == 1;
      // store's 1'th operand (not 0'th) is "dest"
      // to do : more cases
  }
      
  private:  void eval_differ(set<pair<Instruction *, string>> & obj, set<pair<Instruction *, string>> & eraseValue){
      if(obj.empty() || eraseValue.empty()){ // 11.30. ADD erase empty CHECK
          return;
      }
      else{
          set<pair<Instruction *, string>> newObj;
          for(set<pair<Instruction *, string>>::iterator i = obj.begin(); i != obj.end(); i++){
              for(set<pair<Instruction *, string>>::iterator j = eraseValue.begin(); j != eraseValue.end(); j++ ){
                  if((i->first == j->first) && (i->second == j->second)){
                      continue;
                  }
                  else{
                      newObj.insert(pair<Instruction *, string>(i->first, i->second));
                  }
              }
          }
          obj = newObj;
      }
  }
      
  private: string getValueName(Instruction *i){
      string name = "";
      
      if(is_destOprd(i, 1)){
          name = i->getOperand(1)->getName();
      }
      else{
          name = i->getOperand(0)->getName();
      }
      
      return name;
  }
      
  private:  void eval_GENKILL(BasicBlock * bb) {
      if (GEN.count(bb) < 1 || KILL.count(bb) < 1) return; // throw exception
      
      set<pair<Instruction *, string>> & bbGEN = GEN.find(bb)->second;
      set<pair<Instruction *, string>> & bbKILL = KILL.find(bb)->second;
      
      for (BasicBlock::iterator inst = bb->begin(); inst != bb->end(); inst++) {
          set<pair<Instruction *, string>> gen;
          set<pair<Instruction *, string>> kill;
          Instruction &bbInst = *inst;
          
          if(is_ignorableInst(&bbInst)) continue;
        
          string valueName = getValueName(&bbInst);
          
          
//          errs() << "\n\n";
//          if(is_destOprd(&bbInst, 1)){
//              errs() << "store := "<< valueName <<"\n";
//              if((&bbInst)->getDebugLoc()){
//                  DILocation *uLoc = (&bbInst)->getDebugLoc();
//                  errs() << "LINE : " <<uLoc->getLine() << "\n";
//              }
//          }
          
//          errs() << "value Name : " << valueName << "\n\n"; // 2019.11.26 TODO : cmp, add2, retval 제거 필요

          if(valueName != ""){
              gen.insert(pair<Instruction *, string>(&bbInst, valueName));                            // G = op
//              errs() << "bb : " << bb << "\n";
//              errs() << "gen : " << &bbInst <<", "<< valueName << "\n";
              variables.insert(valueName);
              Function *f = bbInst.getFunction();
              
              for(auto &gb : *f){
                  for(auto &i : gb){
                      string compareName = getValueName(&i);

                      if(&gb != bb){ // 같은 베이직 블록은 검사를 안해야하는지?
                          if(is_destOprd(&i, 1)){
                              if(valueName == compareName && (&i == &bbInst)){
                                  continue;
                              }
                              if(valueName == compareName && compareName != ""){
                                  kill.insert(pair<Instruction *, string>(&i, compareName));               // K = All ops define dest
                              }
                          }
                      }
                  }
              }
//              errs() << "\n\n";
//              errs() << "kill is empty? : " << kill.empty() << "\n";
//              for(set<pair<Instruction *, string>>::iterator j = kill.begin(); j != kill.end(); j++){
//                  errs() << "kill set : < Instruction * : " << j->first << ", name : "<< j->second <<" >\n";
//              }
              
              eval_differ(bbGEN, kill);                 // GEN(X) - K
              bbGEN.insert(gen.begin(), gen.end());     // G+(GEN(X)-K)
//
//              for(set<pair<Instruction *, string>>::iterator j = bbGEN.begin(); j != bbGEN.end(); j++){
//                  errs() << "bbGEN set : < Instruction * : " << j->first << ", name : "<< j->second <<" >\n";
//              }
//
              eval_differ(bbKILL, gen);                 //KILL(X) - G
              bbKILL.insert(kill.begin(), kill.end());  //K+(KILL(X)-G)
          }
      }
  }
      
  private: set<BasicBlock*> * getSuccessors(BasicBlock *bb) {
      set<BasicBlock *> * succs = new set<BasicBlock*>();
      Instruction * term_inst = bb->getTerminator();
      
      for (int i = 0; i < term_inst->getNumSuccessors(); i++)
          succs->insert(term_inst->getSuccessor(i));

      return succs;
  }
      
  private: set<BasicBlock*> * getPredecessor(BasicBlock *bb) {
      set<BasicBlock *> * pred = new set<BasicBlock*>();
      
      for (auto i = pred_begin(bb); i != pred_end(bb); i++){
          pred->insert(*i);
      }
      return pred;
  }
  private:  void eval_AllINOUT(Function &f){
      bool change = true;
      
      while (change) {
          change = false;
          for (Function::iterator bb = f.begin(); bb != f.end(); bb++) {
              BasicBlock &temp = *bb;
              set<BasicBlock*> * preds = getPredecessor(&temp);
              
              errs() << "#############function BB : " << &temp << "\n\n";
              if (GEN.count(&temp) < 1 || KILL.count(&temp) < 1 || IN.count(&temp) < 1 || OUT.count(&temp) < 1) return; //exception
              
              set<pair<Instruction *, string>> & bbGEN = GEN.find(&temp)->second;
              set<pair<Instruction *, string>> & bbKILL = KILL.find(&temp)->second;
              set<pair<Instruction *, string>> & bbIN = IN.find(&temp)->second;
              set<pair<Instruction *, string>> & bbOUT = OUT.find(&temp)->second;
              
//              errs()<<"BASICBLOCK PREDCESSOR CHECK 11111: "  <<preds->empty() << " is empty \n";
              if(preds->empty()){                               //2019.11.29. First Basicblock IN is empty, Not exist predecessor
                  bbOUT.insert(bbGEN.begin(), bbGEN.end());
//                  errs()<<"bbOUT : "  <<bbOUT.empty() << " is empty \n";
              }
              
              // OLD_OUT = OUT;
              set<pair <Instruction *, string>> old_out = bbOUT;    // value by value copy
              
//              errs() << "\n\n";
//              // IN = OUTs of successors
//              set<BasicBlock*> * succrs = getSuccessors(&temp);
              
              //2019.11.28.
              for(set<BasicBlock*>::iterator pred = preds->begin(); pred != preds->end(); pred++){
                  set<pair<Instruction *, string>> & predset = OUT.find(*pred)->second;    // the predecessor's OUT set
                  bbIN.insert(predset.begin(), predset.end());                          // add to OUT predecessor
//                  errs() << "IN SUCCESSOR : " << bbIN.empty() << " is EMPTY? \n\n\n";
              }
              
              /*2019.11.27*/
              set<pair<Instruction *, string>> cal_OUT;
              
              eval_differ(bbIN, bbKILL);                    // IN(X)-KILL(X)
              cal_OUT.insert(bbGEN.begin(), bbGEN.end());
              cal_OUT.insert(bbIN.begin(), bbIN.end());
              bbOUT = cal_OUT;                              //OUT(X) = GEN(X)+(IN(X)-KILL(X))
              
              if (old_out != bbOUT){
                  change = true;
              }
          }
      }
  }
  
  private: void make_udchain(Function &f){
      Function *fAdd = &f;
      
      for(auto &bb : *fAdd){
          for(auto &i : bb){
              Instruction * inst = &i;
              string opcodeName = inst->getOpcodeName();
              unsigned currLine;
              
              if((inst)->getDebugLoc()){
                  DILocation *uLoc = (inst)->getDebugLoc();
                  currLine = uLoc->getLine();
              }
              
              if(opcodeName == "load"){
                  string valueName = getValueName(inst);
//                  errs()<< "로드 변수 : " << valueName <<"로드 변수 라인 위치 : "<< currLine << "\n\n";
                  for(auto &bb2 : *fAdd){
                      if(&bb != &bb2){
                          set<pair<Instruction *, string>> tempIN = IN.find(&bb2)->second;
                          
                          for(set<pair<Instruction *, string>>::iterator j = tempIN.begin(); j!= tempIN.end(); j++){
                              Instruction * inInst = j->first;
                              string inOpcodeName = inInst->getOpcodeName();
                              unsigned iLine;
                              
                              if((inInst)->getDebugLoc()){
                                  DILocation *uLoc = (inInst)->getDebugLoc();
                                  iLine = uLoc->getLine();
                              }
                              if(inOpcodeName == "store" && valueName == j->second){
                                  if(iLine < currLine){
                                      if(j->second == "jni" || j->second == "fd"){
                                          udChain.insert(pair<Instruction *, string>(inst, "j"));
                                          udChain.insert(pair<Instruction *, string>(inInst, "j"));
//                                          errs() << "현재 instruction : " << inst << ", 정의된 변수 이름 : " << j->second << "정의된 변수 위치 : " << iLine<< "\n";
                                      }
                                  }
                              }
                          }
                      }
                  }
              }
          }
      }
              
  }
      
  private: void check_isSame(Function * f, Instruction *addr){
      for(auto &bb: *f){
          set<pair<Instruction *, string>> tempIN = IN.find(&bb)->second;
          set<pair<Instruction *, string>> tempOUT = OUT.find(&bb)->second;
          
          for(set<pair<Instruction *, string>>::iterator in = tempIN.begin(); in!= tempIN.end(); in++){
              Instruction * inInst = in->first;
              
          }
          for(auto &i: bb){
              
              
              Instruction * inst = &i;
              string opcodeName = inst->getOpcodeName();
              unsigned currLine;
              
              if((inst)->getDebugLoc()){
                  DILocation *uLoc = (inst)->getDebugLoc();
                  currLine = uLoc->getLine();
              }
            
          }
      }
  }
  private: void make_duchain(Function &f){        //refactoring 필요
      Function *fAdd = &f;
      for(auto &bb : *fAdd){
          for(auto &i : bb){
              Instruction * inst = &i;
              string opcodeName = inst->getOpcodeName();
              unsigned currLine;
              
              if((inst)->getDebugLoc()){
                  DILocation *uLoc = (inst)->getDebugLoc();
                  currLine = uLoc->getLine();
              }
              
              if(opcodeName == "store"){
                  string valueName = getValueName(inst);
                  
                  errs() << "\ninstruction : " << inst << "\n";
                  errs()<< "store 변수 : " << valueName <<" , store 변수 라인 위치 : "<< currLine << "\n\n";
                  for(auto &bb2 : *fAdd){
                      if(&bb != &bb2){
                          set<pair<Instruction *, string>> tempIN = IN.find(&bb2)->second;
                          for(set<pair<Instruction *, string>>::iterator j = tempIN.begin(); j!= tempIN.end(); j++){
                              Instruction * inInst = j->first;
                              string inOpcodeName = inInst->getOpcodeName();
                              unsigned iLine;
                              
                              if((inInst)->getDebugLoc()){
                                  DILocation *uLoc = (inInst)->getDebugLoc();
                                  iLine = uLoc->getLine();
                              }
                              if(inOpcodeName == "load" && valueName == j->second){
                                  if(iLine > currLine){
                                      if(j->second == "jni" || j->second == "fd"){
//                                          duChain.insert(pair<Instruction *, string>(inst, "j"));
                                          errs() << "현재 instruction : " << inst << ", 로드된 변수 이름 : " << j->second << " 로드된 변수 위치 : " << iLine<< "\n";
                                      }
                                  }
                              }
                          }
                      }
                  }
              }
          }
      }
  }
      
  private:  void printINOUT(int bb_number, BasicBlock *bb){
      if (IN.count(bb) < 1 || OUT.count(bb) < 1) return ; // throw exception
      
      errs() << "BASIC BLOCK[" << bb_number << "]" << "\n";
      
      errs() <<  "  IN  : ";
      printVars(IN.find(bb)->second);
      
      errs() <<  "  OUT : ";
      printVars(OUT.find(bb)->second);
      
  }
      
  private:  void printTable(map<pair<Instruction *, string>, Instruction *> table){
      errs() << "\n\n\n";
      errs() << "######################### Backward Taint Analysis Table Data #########################" << "\n";
      map<pair<Instruction *, string>, Instruction *>::iterator iter;
      unsigned useLine, defLine;
      StringRef File, Dir;
      
      for(iter = table.begin() ; iter != table.end() ; iter++ ){
          if (DILocation *uLoc = ((iter->first).first)->getDebugLoc()) {
              useLine = uLoc->getLine();
          }
          if (DILocation *Loc = (iter->second)->getDebugLoc()) { // Here I is an LLVM instruction
              defLine = Loc->getLine();
              File = Loc->getFilename();
              Dir = Loc->getDirectory();
          }
          
          string src_val = (iter->second)->getOperand(0)->getName();
          
          errs() << "<<USE line "<< useLine << " : " << (iter->first).first << ", " << (iter->first).second << ">, DEF line "<< defLine << " : " << iter->second << " > \n\n";
      }
      errs() << "#######################################################################################" << "\n";
  }
      
  private:  void printVars(set<pair<Instruction *, string>> vars) {
      for( set<pair<Instruction *, string>>::iterator v = vars.begin(); v != vars.end(); v++)
          errs() << "Intstruction address : " << v->first << "         variable : " << v->second << "\n" ;
      errs() << "\n";
  }
  };
}

char Forward::ID = 0;
static RegisterPass<Forward> X("Forward", "RD Pass");
