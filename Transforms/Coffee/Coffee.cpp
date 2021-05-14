#define DEBUG_TYPE "coffee"
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

using namespace llvm;
using namespace std;

namespace {
    struct Coffee : public BasicBlockPass { //BasicBlockPass
        static char ID;     // Pass identification
        Coffee(): BasicBlockPass(ID) { }
        
        map<pair<Instruction *, string>, Instruction *> btd_table;
        map<pair<unsigned, unsigned>, pair<string, string>> analysisResultTable; //Add 2019.04.17 <<useLine, DefLine>, <Use var, Result>>
        map<BasicBlock*, set<pair<Instruction *, string>>&> USE;
        map<BasicBlock*, set<pair<Instruction *, string>>&> DEF;
        map<BasicBlock*, set<pair<Instruction *, string>>&> IN;
        map<BasicBlock*, set<pair<Instruction *, string>>&> OUT;
        
        virtual bool runOnBasicBlock(BasicBlock &bb) {
            eval_USEDEF(&bb);
        }
        
        virtual bool  doInitialization(Function &f) {
            for (Function::iterator bb = f.begin(); bb != f.end(); bb++) {
                BasicBlock &temp = *bb;
                USE.insert(pair<BasicBlock*, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));
                DEF.insert(pair<BasicBlock*, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));
                IN.insert(pair<BasicBlock*, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));
                OUT.insert(pair<BasicBlock*, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));
                
            }
            return true;
        }
        
        /*****      TO DO : refactoring         *****/
        virtual bool doFinalization(Function &f) {
            eval_AllINOUT(f);
            
            errs() << "\n\nTEST : Function Name is ====> " << f.getName() << "\n\n";

            
            errs() << "******* Backward Taing Analysis Result *******\n";
            int bb_count = 0;
            for (Function::iterator bb = f.begin(); bb != f.end(); bb_count++,bb++) {
                BasicBlock &temp = *bb;
                // printUSEDEF(bb_count, bb); // optional
                //printINOUT(bb_count, &temp);
            }
            
            printTable(btd_table);
            errs() << "\n\n";
            errs() << "*********       end of the result       *********\n";
            
            return true;
        }
        

    private:  bool is_inDEF(BasicBlock * bb, string s){ // check <P,  src>  in  DEF(X)
        if (DEF.count(bb) < 1) return false; // throw exception
        
        set<pair<Instruction *, string>> & bbDEF = DEF.find(bb)->second;
        int checkIn = 0;
       
        for(set<pair<Instruction *,string>>::iterator i = bbDEF.begin() ; i != bbDEF.end(); i++){
            if((i->second).compare(s) == 0){
                checkIn++;
            }
        }
        return (checkIn >= 1) ;
    }
        
    private:  Instruction* getPC_inDEF(BasicBlock * bb, string s){ // check <P,  src>  in  DEF(X)
        set<pair<Instruction *, string>> & bbDEF = DEF.find(bb)->second;
        Instruction * tt = NULL;
        int count = 0;
        
        for(set<pair<Instruction *,string>>::iterator i = bbDEF.begin() ; i != bbDEF.end(); i++){
            if(i->second == s){
                count++;
                tt = i->first;
//                return i->first;
            }
        }
//        errs() << "\n\nDEF Count : " << count << "  Intruction ADD : "<< tt << ": " << tt->getOperand(0)->getName() <<"\n\n";
        return tt;
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
        return (opcodeName == "br" && i_oprd > 0);
        // conditional branch has multiple operands
        // only the first is considered "used", others are "labels"
        
        // to do : more cases
    }
        
    private: bool is_destOprd(Instruction * inst, unsigned int i_oprd) {
        string opcodeName = inst->getOpcodeName();
        
        return (opcodeName == "store" && i_oprd == 1);

        // store's 1'th operand (not 0'th) is "dest"
        // to do : more cases
    }
        
    private:  void eval_USEDEF(BasicBlock * bb) {
        if (USE.count(bb) < 1 || DEF.count(bb) < 1) return; // throw exception
        
        set<pair<Instruction *, string>> & bbUSE = USE.find(bb)->second;
        
        set<pair<Instruction *, string>> & bbDEF = DEF.find(bb)->second;
        
        
        for (BasicBlock::iterator inst = bb->begin(); inst != bb->end(); inst++) {
            Instruction &bbInst = *inst;
            if(is_ignorableInst(&bbInst)) continue;
            
            
            //ADD HERE RETE 2019.04.18
            if(isa<CallInst>(&bbInst)){
                CallInst *cl = cast<CallInst>(inst);
                string calledFunctionName = cl->getCalledFunction()->getName();
                errs() << "\n\n Check Called Function : " << cl->getCalledFunction()->getName() << "\n\n";
                errs() << "ADDRESS : " << &bbInst << "\n";
                errs() << "lwwwwwwwwwwwwwwww\n";
            }
            
            errs() << "check DEF \n";
            printVars(bbDEF);
            errs() << "check DEF end \n";
            
            errs() << "check USE \n";
            printVars(bbUSE);
            errs() << "check USE end \n";
            
//            string tempGetName =bbInst.getName();
            
            string tempGetName =bbInst.getOperand(0)->getName();
            errs() << "\nis Nuyll???  : " << tempGetName << "\n\n";
//            if(is_destRegister(tempGetName)){
//                errs() << "?Heeerrreeeee Tessssettttt0000 : " << tempGetName << "\n";
//                errs() << "hhh1111111\n";
////                continue;
//            }
//            errs() << "?Heeerrreeeee Tessssettttt11111 : " << tempGetName << "\n";
            
            //Add Here Rete 2019.04.17 - Null string Pass
            if(is_destOprd(&bbInst, 1)){
                errs() << "Storrrreeee!  : "<< bbInst.getOperand(1)->getName() << "\n" ;
                tempGetName = bbInst.getOperand(1)->getName();
            }
            
            pair<Instruction *,string> dest(&bbInst, tempGetName);
            
            string opcodeName = bbInst.getOpcodeName();
            unsigned int numOperands = bbInst.getNumOperands();
            
            for (int i_oprd = 0; i_oprd < numOperands; i_oprd++) {
                if (is_ignorableOprd(&bbInst, i_oprd)) break;
                
                
                string src_val = bbInst.getOperand(i_oprd)->getName();
                if(src_val == "") break;
                
                
                pair<Instruction *,string> src (&bbInst, src_val);
                
                //Add Here 2019.04.17
//                errs() << "\n   src   : " <<src_val << "\n";
                
                if(is_inDEF(bb, src_val)){ // if  (<P,  src>  in  DEF(X))  for  some  P
                    btd_table.insert(pair<pair<Instruction *, string>, Instruction *>(pair<Instruction *, string>(&bbInst, src_val), getPC_inDEF(bb, src_val)));
//                    g_table.insert(<BasicBlock*, pair<Instruction *, string>>(bb, pair<Instruction *, string(&bbInst, src_val))); // add here 19.03.29
                }
                
                //Add Here 2019.04.17
//                errs() << "\n hhhhhhheeeeelllllooo \n";
//                printTable(btd_table);
//                errs() << "\n Ennnnnnddddd \n";
                
                if (is_destOprd(&bbInst, i_oprd))
                    dest = src;
                
                if (!is_inDEF(bb, src_val)) // src  not  in  DEF(X) : USE(X)  +=  <current_pc,  src>
                    bbUSE.insert(pair<Instruction *, string>(&bbInst, src_val));
            }
            
            if(is_inDEF(bb, dest.second) && getPC_inDEF(bb, dest.second) != &bbInst){
                bbDEF.erase(make_pair(getPC_inDEF(bb, dest.second), dest.second));
            }
            bbDEF.insert(dest);     // DEF(X)  +=  <current_pc,  dest>
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
                
                set<pair<Instruction *, string>> & bbUSE = USE.find(&temp)->second;
                set<pair<Instruction *, string>> & bbDEF = DEF.find(&temp)->second;
                set<pair<Instruction *, string>> & bbIN = IN.find(&temp)->second;
                set<pair<Instruction *, string>> & bbOUT = OUT.find(&temp)->second;
                
                // OLD_IN = IN;
                set<pair<Instruction *, string>> old_in = bbIN;    // value by value copy
                
                // OUT = INs of successors
                set<BasicBlock*> * succrs = getSuccessors(&temp);
                
                for(set<BasicBlock*>::iterator succ = succrs->begin(); succ != succrs->end(); succ++){
                    set<pair<Instruction *, string>> succset = IN.find(*succ)->second;    // the successor's IN set
                    bbOUT.insert(succset.begin(), succset.end());    // add to OUT
                }
                
                set<pair<Instruction *, string>> newout_woDEF;
                set<pair<Instruction *, string>> newout_withDEF;

                
                for(set<pair<Instruction *,string>>::iterator i = bbOUT.begin() ; i != bbOUT.end(); i++){
                    // OUT_woDEF(X) = (OUT(X) - DEF(X))
                    if(!is_inDEF(&temp, i->second)){
                        newout_woDEF.insert(pair<Instruction *, string>(i->first, i->second));
                    }
                    //OUT_withDEF(X)
                    if(is_inDEF(&temp, i->second)){
                        // && getPC_inDEF(&temp, i->second) != i->first
                        newout_withDEF.insert(pair<Instruction *, string>(i->first, i->second));
                        btd_table.insert(pair<pair<Instruction *, string>, Instruction *>(pair<Instruction *, string>(i->first, i->second), getPC_inDEF(&temp, i->second)));
                    }
                }
                
//                for(set<pair<Instruction *,string>>::iterator k = newout_withDEF.begin() ; k != newout_withDEF.end(); k++){
//                    errs() << "src_val : " << k->second << "\n";
//                    errs() << "current Int : " << k->first << "   P instrction : " <<  getPC_inDEF(&temp, k->second) << ", " <<getPC_inDEF(&temp, k->second)->getOperand(0)->getName() << "\n";
//                    if (DILocation *Loc = (k->first)->getDebugLoc()) {
//                        unsigned Line = Loc->getLine();
//                        errs() << "line c : " << Line << "\n";
//                    }
//                    if (DILocation *uLoc = (getPC_inDEF(&temp, k->second))->getDebugLoc()) {
//                        unsigned useLine = uLoc->getLine();
//                        errs() << "line P : " << useLine << "\n\n";
//                    }
//                    btd_table.insert(pair<pair<Instruction *, string>, Instruction *>(pair<Instruction *, string>(k->first, k->second), getPC_inDEF(&temp, k->second)));
//                }
                
                // IN = USE(X) + OUT_woDEF(X)
                bbIN = bbUSE;                // value by value copy
                bbIN.insert(newout_woDEF.begin(), newout_woDEF.end());
                
                if (old_in != bbIN)
                    change = true;
            }
        }
    }
        
    private:  void printUSEDEF(int bb_number, BasicBlock *bb){
        if (USE.count(bb) < 1 || DEF.count(bb) < 1) return ; // throw exception
        
        errs() << "BASIC BLOCK[" << bb_number << "]" << "\n";
        
        errs() <<  "  USE  : \n";
        printVars(USE.find(bb)->second);
        
        errs() <<  "  DEF : \n";
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
    
    /*Add Here Rete. 2019.04.10*/
//    private: bool findTable(Instruction * defAdd){
    private: bool findTable(unsigned lineNum){
        map<pair<Instruction *, string>, Instruction *>::iterator iter;
        bool check = false;
        
        for(iter = btd_table.begin(); iter != btd_table.end() ; iter++){
//            if((iter->first).first == defAdd){
//                check = true;
//            }
            if (DILocation *uLoc = ((iter->first).first)->getDebugLoc()) {
                if(lineNum == uLoc->getLine()){
                    check = true;
                }
            }
        }
        
        return check;
    }
        
    private:  void printTable(map<pair<Instruction *, string>, Instruction *> table){
        errs() << "\n\n\n";
        errs() << "######################### Backward Taint Analysis Table Data #########################" << "\n";
        map<pair<Instruction *, string>, Instruction *>::iterator iter;
        unsigned useLine, defLine;
        StringRef File, Dir;
        string analysisResult = "";
        
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
            
           
            
            // Add Here Rete 2019.04.10
            string temp = (iter->first).second;
            
            analysisResult = outputAnaly(temp, defLine);
            
            if(analysisResult == "MAYBE"){ //<<useLine, DefLine>, <Use var, Result>>
                analysisResultTable.insert(pair<pair<unsigned, unsigned>, pair<string, string>>(pair<unsigned, unsigned>(useLine, defLine), pair<string, string>((iter->first).second, analysisResult)));
                analysisResult = maybeReAnalysis(useLine, (iter->first).second);
            }
            
            string test = (iter->second)->getOperand(0)->getName();
            
            if(test == ""){
                test = (iter->second)->getOperand(1)->getName();
            }
            
            
            
//            analysisResultTable.insert(pair<pair<unsigned, unsigned>, pair<string, string>>(pair<unsigned, unsigned>(useLine, defLine), pair<string, string>((iter->first).second, analysisResult)));
            
//            analysisResultTable.insert();
            
            errs() << "<<USE line "<< useLine << " : " << (iter->first).first << ", " << (iter->first).second << ">, DEF line "<< defLine << " : " << iter->second << ">  , <DEF var> : " << test<< "===>" << analysisResult <<"\n\n";
            
            defLine = NULL;
            
//            //Debugging
//            string test = (iter->second)->getOperand(0)->getName();
//            errs() << "<<USE line "<< useLine << " : " << (iter->first).first << ", " << (iter->first).second << ">, DEF Var "<< test << " : " << iter->second << " >  ===>" << analysisResult <<"\n\n";
            
//            errs() << "\n DEF ADDRESS getName : " << (iter->second)->getOperand(0)->getName() << "\n";
        }
        
//        map<pair<unsigned, unsigned>, pair<string, string>>::iterator k;
//        for(k = analysisResultTable.begin(); k != analysisResultTable.end(); k++){
//            errs() << "UseLine : " << (k->first).first << "  DefLine : "<< (k->first).second <<" USE var : " << (k->second).first << " REsult : " << (k->second).second << "\n";
//            Instruction *asd;
//
//            if(findTable((k->first).second)){
////                errs() << "DEF Line" <<  (k->first).second << " YEAH\n";
////                errs() << "USE ADD Call Fun : " << getMaybeValUSEAddr((k->first).second) << "\n";
//                asd = getMaybeValUSEAddr((k->first).second, (k->second).first);
////                errs() << " ADDR CALL GETNAME : " << asd->getOperand(0)->getName() << "\n";
//                errs() << "Analysis RESULT : ";
//                errs() << outputAnaly(asd->getOperand(0)->getName(), (k->first).second) << "\n";
//            }
//        }
        
        errs() << "#######################################################################################" << "\n";
    }
        
    //Add Here 2019.04.17
    private : string outputAnaly(string val, unsigned defLine){
        if(val.find("addr") != std::string::npos){
            return "RUI";
        }
        else if(!findTable(defLine)){
            return "NO";
        }
        else{
            return "MUI";
        }
    }
        
   //Add Here 2019.04.17
    private: Instruction* getMaybeValUSEAddr(unsigned lineNum, string useVal){
        map<pair<Instruction *, string>, Instruction *>::iterator iter;
        
        for(iter = btd_table.begin(); iter != btd_table.end() ; iter++){
            string s = (iter->first).second;
            unsigned getLine = getDebugLocLine((iter->first).first);
//            if ((getLine == lineNum) && (s.find("addr") != std::string::npos)) {
//                errs() << "addr!!!!!!!!!!!!!!!!!!\n";
//            }
            if ((getLine == lineNum) && (useVal.compare(s)==0)) {
                return (iter->first).first;
            }
        }
    }
    
        
    private: Instruction* getMaybeValDEFAddr(unsigned lineNum, string useVal){
        map<pair<Instruction *, string>, Instruction *>::iterator iter;

        for(iter = btd_table.begin(); iter != btd_table.end() ; iter++){
            string s = (iter->first).second;
            unsigned getLine = getDebugLocLine((iter->first).first);
            
            if ((getLine == lineNum) && (useVal.compare(s)==0)) {
                return (iter->second);
            }
        }
    }
    
        
    private: unsigned getDebugLocLine(Instruction* i){
        if (DILocation *uLoc = i->getDebugLoc()) {
            return uLoc->getLine();
        }
        else{
            return -1;
        }
    }
    private: string maybeReAnalysis(unsigned useLine, string useVal){
        Instruction *asd;
//        unsigned ul = useLine;
        unsigned ul = getDebugLocLine(getMaybeValDEFAddr(useLine, useVal));  //==>2
        unsigned predUl = 0;
        int check = 0;
        string revalue = "MAYBE";
        string valName = "";
        
        while(revalue == "MAYBE"){
//            if(findTable(ul)){
        
                asd = getMaybeValUSEAddr(ul, useVal);
                predUl = getDebugLocLine(asd);
//                asd = getMaybeValDEFAddr(ul, useVal);   // ==>2
//             string test = (iter->second)->getOperand(0)->getName();
//            errs() << "test : " << asd->getOperand(0)->getName() << "\n";
//            errs() << "test : " << is_tempBiteVariable(asd, useVal) << "\n";
//                errs() << "ul first : " << ul << "\n";
//                errs() << "getLine first    : " << getDebugLocLine(asd) << "\n";
//                errs() << "asd first   : "<< asd << "\n";
            
            if(is_tempBiteVariable(asd, useVal) && (pickUpVarString(predUl) != "")){
//                errs() << "hehe : " <<  pickUpVarString(predUl) << "\n\n";
                //                pickUpVarString(predUl);
                revalue = outputAnaly(pickUpVarString(predUl), ul);
                
                ul = getDebugLocLine(getMaybeValDEFAddr(predUl, pickUpVarString(predUl)));
                useVal = pickUpVarString(predUl);
//                errs() << "SET REvalue : " << revalue << "\n";
                
            }
            else{
                revalue = outputAnaly(asd->getOperand(0)->getName(), ul);
                ul = getDebugLocLine(getMaybeValDEFAddr(predUl, useVal)); //==>3
                asd = getMaybeValUSEAddr(ul, useVal); //==>3
            }
            
            
            
//            }
            
//            predUl = getDebugLocLine(asd);
            
//            errs() << "\n\nrevalu    : " << revalue << "\n";
//            errs() << "predUL : " << predUl << "\n";
//            errs() <<getMaybeValDEFAddr(predUl, useVal) << "\n";
//
//            ul = getDebugLocLine(getMaybeValDEFAddr(predUl, useVal));
//            asd = getMaybeValUSEAddr(ul, useVal);

            
//
//            errs() << "ul    : " << ul << "\n";
//            errs() << "asd     : " << asd << "\n";
//            errs() << "getLine     : " << getDebugLocLine(asd) << "\n\n\n";
            
            check++;
        }
    
        return revalue;
    }
    
    private: bool is_tempBiteVariable(Instruction * inst, string valName) {
        string opcodeName = inst->getOperand(0)->getName();
        
//        errs() << "opcode TEST : " << opcodeName << "\n";
        if(opcodeName == ""){
//            errs() << "hiiiiiiiiiii : inpu " << valName<< "  getNAme : " << inst->getOperand(1)->getName() << "\n";
            if(inst->getOperand(1)->getName() == valName){
//                errs() << "hiiiiiiiiiii" << inst->getOperand(1)->getName() << "\n";
                return true;
//                errs() << inst->getOperand(1)->getName();
            }
//            errs() << inst->getOperand(1)->getName();
        }
        return (opcodeName.find("add") != std::string::npos);
        
        // to do : more cases
    }
        
    private: string pickUpVarString(unsigned line){
        map<pair<Instruction *, string>, Instruction *>::iterator iter;
        Instruction * temp;
        string kkk = "";
        
        for(iter = btd_table.begin(); iter != btd_table.end() ; iter++){
            temp = (iter->first).first;
//            errs() << "sdfasdfadfasf    : "  << kkk << "\n";
//            if( (getDebugLocLine(temp) == line) && kkk.find("add") != std::string::npos){
            if(getDebugLocLine(temp) == line){
                kkk = (iter->first).second;
//                kkk = (iter->first).second;
//                errs() << "jwljwlkfjqwl;kvjqweovjqwleivjq;lwv    :  " << kkk << "\n";
//                if(s.find("addr") != std::string::npos){
//                    errs() << (iter->first).second;
//                }
            }
        }
        return kkk;
    }
        
     
    private:  void printVars(set<pair<Instruction *, string>> vars) {
        for( set<pair<Instruction *, string>>::iterator v = vars.begin(); v != vars.end(); v++)
            errs() << "Intstruction address : " << v->first << " ==> "<<getDebugLocLine(v->first)<<"         variable : " << v->second << "\n" ;
        errs() << "\n";
    }
    };
}

char Coffee::ID = 1;
static RegisterPass<Coffee> X("coffee", "LV Pass");
