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
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/ADT/SmallVector.h"



using namespace llvm;
using namespace std;
// To do List 2021.05.27
// Forward 분석 1단계만 되게 하기 - Forward, Function Pass로 바꾸기
// Backward로 타고 타고 JNI 진입 함수까지 분석하게 하기 - 이건 그냥 모듈 pass로
//



// Refer this web site: https://cpp.hotexamples.com/examples/-/CallGraphSCCPass/-/cpp-callgraphsccpass-class-examples.html
namespace{
	static cl::opt<int> lineNumber("num", cl::desc("Please enter crash line number"), cl::init(0));
	static cl::opt<std::string> functionName("mapping-functio", cl::desc("The function name to be mapped"), cl::init(""));
	static cl::opt<std::string> fileName("file-name", cl::desc("The file's name"), cl::init(""));
	static cl::opt<bool> test ("test", cl::desc("Enter the test"), cl::init(true));
	set<pair<Instruction *, string>> intersection(set<pair<Instruction *, string>> set1, set<pair<Instruction *, string>> set2){
		set<pair<Instruction *, string>> result;
		bool ok = false;
		for(set<pair<Instruction *, string>>::iterator iter1 = set1.begin(); iter1 != set1.end(); iter1++){
			for(set<pair<Instruction *, string>>::iterator iter2 = set2.begin(); iter2 != set2.end(); iter2++){
				if(iter1->second == iter2->second) { //중복확인
					ok = true;
					continue;
				}
			}
			if( !ok ){
				//set<pair<Instruction *, string>>* insertSet = iter1;
				ok = false;
				result.insert(*iter1);
			}
		}
		return result;
	}
	set<Instruction *> intersection(set<Instruction *> set1, set<Instruction *> set2){
		set<Instruction *> result;
		bool ok = false;
		for(set<Instruction *>::iterator iter1 = set1.begin(); iter1 != set1.end(); iter1++){
			for(set<Instruction *>::iterator iter2 = set2.begin(); iter2 != set2.end(); iter2++){
				if(iter1 == iter2) { //중복확인
					ok = true;
					continue;
				}
			}
			if( !ok ){
				//set<pair<Instruction *, string>>* insertSet = iter1;
				ok = false;
				result.insert(*iter1);
			}
		}
		return result;
	}
	set<pair<Instruction *, string>> an_union(set<pair<Instruction *, string>> set1, set<pair<Instruction *, string>> set2){
		set<pair<Instruction *, string>> result;
		result.insert(set1.begin(), set1.end());
		result.insert(set2.begin(), set2.end());
		return result;
	}

	bool isSamePairInstString(pair<Instruction *, string> op1, pair<Instruction *, string> op2)
	{
		string result = op1.second == op2.second ? "true":"false";
		errs() << "op1: " <<op1.second << "\top2: "<<op2.second;
		errs() <<"\t compare : "<< result<<"\n";
		if(op1.second == op2.second)
			return true;
		return false;
	}
	bool isRUICall(std::string funcName){
		// 함수 리스트 만들기
		// for only test;
		if(funcName.find("scanf")) return true;
		if(funcName.find("istream")) return true;
		if(funcName.find("fgets")) return true;
		if(funcName.find("fscanf")) return true;

		return false;
	}

	bool isCallUseExploit(std::string varName){
		if(varName.find("scanf")!= string::npos) return true;
		if(varName.find("istream")!= string::npos) return true;
		if(varName.find("scanf")!= string::npos) return true;
		if(varName.find("istream")!= string::npos) return true;
		if(varName.find("fgets")!= string::npos) return true;
		if(varName.find("fscanf")!= string::npos) return true;

		return false;
	}

	string getDefVar(Instruction* inst){
		if(dyn_cast<StoreInst>(inst)) return inst->getOperand(1)->getName();
		if(inst->getType()->isVoidTy()) return "";
		return inst->getName();
	}

	enum MF_TAG {
		RUI, NOT, MUI
	};

	struct Forward : public ModulePass {
		//Redesign by Seong-Kyun
		static char ID;     // Pass identification
		Forward(): ModulePass(ID) { }

		//      map<pair<Instruction *, string>, Instruction *> btd_table;
		map<BasicBlock *, Function *> functionTable;
		map<Instruction *, set<Instruction *>> GEN;
		//      map<BasicBlock *, set<pair<Instruction *, BasicBlock *>>&> KILL;
		map<Instruction*, set<Instruction *>> KILL;
		map<Instruction*, set<Instruction *>> IN;
		map<Instruction*, set<Instruction *>> OUT;

		map<Instruction*, set<Instruction*>> duChain;
		map<Instruction*, set<Instruction*>> udChain;
		set<Function* > CalledJNI;
		set<Instruction* > exploitInst; // 이걸 만들어서 여기에 속하는지만 보자

		set<Instruction* > results;
		//      map<BasicBlock*, set<pair<Instruction *, string>>&> udChain;
		//      map<BasicBlock*, set<pair<Instruction *, string>>&> duChain;
		virtual bool runOnModule(Module& M){
			initFunction(M);
			initExploitInstruction();
			errs() <<"===============Exploit Inst=============\n";
			printVars(exploitInst);
			for(set<Function*>::iterator iter = CalledJNI.begin() ; iter != CalledJNI.end() ; iter++){
				Function* func = *iter;
				doInitialization(*func);
				runOnFunction(*func);
			}

			return false;
		}

		void printUDChain() {
			for(map<Instruction*, set<Instruction*>>::iterator chainIter = udChain.begin(); chainIter != udChain.end(); chainIter++){
				set<Instruction*> defSet = chainIter->second;
				errs() << "USE : ";
				Instruction* useInst = chainIter->first;
				useInst->print(errs());
				errs() << "\n";
				for(set<Instruction*>::iterator defIter = defSet.begin() ; defIter != defSet.end() ; defIter++){
					Instruction* useInst = *defIter;
					errs() <<"\t";
					useInst->print(errs());
					errs() <<"\n";
				}
			}
		}
		void printTable(	map<Instruction*, set<Instruction *>>& table){
			for(map<Instruction*, set<Instruction*>>::iterator chainIter = table.begin(); chainIter != table.end(); chainIter++){
				set<Instruction*> defSet = chainIter->second;
				errs() << "Instruction : ";
				Instruction* useInst = chainIter->first;
				useInst->print(errs());
				errs() << "\nIN:\n";

				for(set<Instruction*>::iterator defIter = defSet.begin() ; defIter != defSet.end() ; defIter++){
					Instruction* useInst = *defIter;
					errs() <<"\t";
					useInst->print(errs());
					errs() <<"\n";
				}
			}
		}
		void initFunction(Module& M){
			if(true){
				for(Function& F: M){
					if(F.getName().find("main") != string::npos) {
						CalledJNI.insert(&F);
						continue;
					}
					for(Instruction& I: instructions(F)){
						if(CallInst* ci = dyn_cast<CallInst>(&I)) {
							if(isCallUseExploit(ci->getCalledFunction()->getName())) {
								// errs() << "Inserted in Called JNI : " << F.getName() << "\t Instruction: ";
								// ci->print(errs());
								// errs() <<"\n";
								CalledJNI.insert(&F);
							}
						}
					}
				}
			}
		}

	public:
		bool checkVulnerable(Instruction* calleeInst){
			//여기서 확인하는 작업을 하자
			set<Instruction*> taintedInsts = getDefSet(calleeInst);
			// errs() << "==================== Tainted Insts ================ \n";
			// printVars(taintedInsts);
			for(Instruction* inst: taintedInsts){
					if(exploitInst.find(inst) != exploitInst.end()) return true;
			}
			return false;
		}

		bool checkInFunction(Function* function){
			if(function->getName() =="main") return true;
			return CalledJNI.find(function)!=CalledJNI.end() ;
		}

	private:

		bool runOnFunction(Function& f){
			for (Function::iterator bb = f.begin(); bb != f.end() ; bb++){
				BasicBlock& temp = *bb;
				runOnBasicBlock(temp);
			}
			eval_AllINOUT(f);
			make_udchain(f);
			//printUDChain();
			// printTable(KILL);
			//          make_udchain(f);
		}

		bool  doInitialization(Function &f) {
			for (Instruction& I : instructions(f)) {
				//              udChain.insert(pair<BasicBlock *, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));
				//              duChain.insert(pair<BasicBlock *, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));
				//              KILL.insert(pair<BasicBlock *, Instruction *>(&temp, NULL));
				GEN.insert(pair<Instruction *, set<Instruction *>&>(&I, * new set<Instruction *>()));
				KILL.insert(pair<Instruction *, set<Instruction *>&>(&I, * new set<Instruction *>()));
				//              KILL.insert(pair<BasicBlock *, set<pair<Instruction *, BasicBlock *>>&>(&temp, * new set<pair<Instruction *, BasicBlock *>>()));
				IN.insert(pair<Instruction*, set<Instruction *>&>(&I, * new set<Instruction *>()));
				OUT.insert(pair<Instruction*, set<Instruction *>&>(&I, * new set<Instruction *>()));

			}
			return true;
		}

	private:
		void initExploitInstruction(){
			// 여기서 메인일때랑 다른 시스템콜일 때랑 나누기
			for(set<Function*>::iterator iter = CalledJNI.begin(); iter != CalledJNI.end(); iter++){
				Function* F = *iter;
					//메인 또는 JNI 함수 일때 for(Instruction& I : instructions(*F)){
				for(Instruction& I : instructions(*F)){
					if(isJNIFunction(F) && isa<StoreInst>(&I)){
						if(I.getOperand(1)->getName().find("addr")) exploitInst.insert(&I);
					}
					if(CallInst* ci = dyn_cast<CallInst>(&I)){
						if(isCallUseExploit(ci->getCalledFunction()->getName())) exploitInst.insert(&I);
					}
				}
			}
		}

		bool isJNIFunction(Function* f){
			// is JNI funciton? or is main function?
			return f->getName().find("main") != string::npos;
		}

		bool is_ignorableInst(Instruction* inst) {
			if(isa<AllocaInst>(inst)) return true;
			if(isa<CallInst>(inst)) {
				CallInst* ci = dyn_cast<CallInst>(inst);
				if(ci->getCalledFunction()->getName()=="llvm.dbg.declare") return true;
			}
			string opcodeName = inst->getOpcodeName();
			unsigned int numOperands = inst->getNumOperands();

			return
			(opcodeName == "br" && numOperands == 1)
			// unconditional branch : no src or dest
			|| (opcodeName == "alloca") ;
			// alloca :  no src or dest

		}



		bool is_ignorableOprd(Instruction * inst, unsigned int i_oprd) {
			string opcodeName = inst->getOpcodeName();

			return opcodeName == "br" && i_oprd > 0;
			// conditional branch has multiple operands
			// only the first is considered "used", others are "labels"

			// to do : more cases
		}

		bool is_destOprd(Instruction * inst, unsigned int i_oprd) {
			string opcodeName = inst->getOpcodeName();

			return opcodeName == "store" && i_oprd == 1;
			// store's 1'th operand (not 0'th) is "dest"
			// to do : more cases
		}

	private:
		void eval_differ(set<Instruction *> & obj, set<Instruction *> & eraseValue){
			if(obj.empty() || eraseValue.empty()){ // 11.30. ADD erase empty CHECK
				return;
			}
			else{
				set<Instruction *> newObj;
				for(set<Instruction *>::iterator i = obj.begin(); i != obj.end(); i++){
					for(set<Instruction *>::iterator j = eraseValue.begin(); j != eraseValue.end(); j++ ){
						if(i == j){
							continue;
						}
						else{
							Instruction* inst = *i;
							newObj.insert(inst);
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

	private:
		void runOnBasicBlock(BasicBlock &bb) {
			eval_GENKILL(&bb);
		}
		bool isSameDest(Instruction* inst1, Instruction* inst2){
			// 여기서
			if(StoreInst* si1 = dyn_cast<StoreInst>(inst1)){
				if(StoreInst* si2 = dyn_cast<StoreInst>(inst2)) {
					if(si1->getOperand(1)->getName() == si2->getOperand(1)->getName()) return true;
				}
			}
			return false;
		}
		void eval_GENKILL(BasicBlock * bb) {
			Function* F = bb->getParent();
			for (BasicBlock::iterator inst = bb->begin(); inst != bb->end(); inst++) {
				set<Instruction *> gen;
				set<Instruction *> kill;
				Instruction &bbInst = *inst;
				if (GEN.count(&bbInst) < 1 || KILL.count(&bbInst) < 1) return; // throw exception

				set<Instruction *> & bbGEN = GEN.find(&bbInst)->second;
				set<Instruction *> & bbKILL = KILL.find(&bbInst)->second;


				if(is_ignorableInst(&bbInst)) continue;

				bbGEN.insert(&bbInst);                            // G = op
				//              errs() << "bb : " << bb << "\n";
				//              errs() << "gen : " << &bbInst <<", "<< valueName << "\n";
				Function *f = bbInst.getFunction();
				for(Instruction& killI : instructions(*f)){
					if(killI.getParent() != bb ){
						if(isSameDest(&bbInst, &killI)) bbKILL.insert(&killI);
					}
				}

				// eval_differ(bbGEN, kill);                 // GEN(X) - K
				// bbGEN.insert(gen.begin(), gen.end());     // G+(GEN(X)-K)
				//
				// //
				// eval_differ(bbKILL, gen);                 //KILL(X) - G
				bbKILL.insert(kill.begin(), kill.end());  //K+(KILL(X)-G)
			}

		}

		set<BasicBlock*> * getSuccessors(BasicBlock *bb) {
			set<BasicBlock *> * succs = new set<BasicBlock*>();
			Instruction * term_inst = bb->getTerminator();

			for (int i = 0; i < term_inst->getNumSuccessors(); i++)
			succs->insert(term_inst->getSuccessor(i));

			return succs;
		}

		set<BasicBlock*> * getPredecessor(BasicBlock *bb) {
			set<BasicBlock *> * pred = new set<BasicBlock*>();

			for (auto i = pred_begin(bb); i != pred_end(bb); i++){
				pred->insert(*i);
			}
			return pred;
		}

		void eval_AllINOUT(Function &f){
			bool change = true;

			while (change) {
				change = false;
				for (Function::iterator bb = f.begin(); bb != f.end(); bb++) {
					BasicBlock &tempBB = *bb;
					set<BasicBlock*> * preds = getPredecessor(&tempBB);
					for(BasicBlock::iterator bbIter = tempBB.begin() ; bbIter != tempBB.end(); bbIter++){
						Instruction& temp = *bbIter;
						if (GEN.count(&temp) < 1 || KILL.count(&temp) < 1 || IN.count(&temp) < 1 || OUT.count(&temp) < 1) return; //exception

						set<Instruction *> & bbGEN = GEN.find(&temp)->second;
						set<Instruction *> & bbKILL = KILL.find(&temp)->second;
						set<Instruction *> & bbIN = IN.find(&temp)->second;
						set<Instruction *> & bbOUT = OUT.find(&temp)->second;

						set<Instruction *> old_out = bbOUT;    // value by value copy
						//              errs()<<"BASICBLOCK PREDCESSOR CHECK 11111: "  <<preds->empty() << " is empty \n";
						if(preds->empty()){                               //2019.11.29. First Basicblock IN is empty, Not exist predecessor
							bbOUT.insert(bbGEN.begin(), bbGEN.end());
							//                  errs()<<"bbOUT : "  <<bbOUT.empty() << " is empty \n";
						}

						// OLD_OUT = OUT;


						//2019.11.28.
						if(bbIter != tempBB.begin()){
							Instruction* beforeInst = temp.getPrevNonDebugInstruction();
							set<Instruction *> beforeOut = OUT.find(beforeInst)->second;
							bbIN.insert(beforeOut.begin(), beforeOut.end());
						}
						for(set<BasicBlock*>::iterator pred = preds->begin(); bbIter == tempBB.begin()&&pred != preds->end(); pred++){
							set<Instruction *> & predset = OUT.find((*pred)->getTerminator())->second;    // the predecessor's OUT set
							bbIN.insert(predset.begin(), predset.end());                          // add to OUT predecessor
							//                  errs() << "IN SUCCESSOR : " << bbIN.empty() << " is EMPTY? \n\n\n";
						}

						/*2019.11.27*/
						set<Instruction *> cal_OUT;

						set<Instruction*> inMinusKILL = intersection(bbIN, bbKILL);                    // IN(X)-KILL(X)
						cal_OUT.insert(bbGEN.begin(), bbGEN.end());
						cal_OUT.insert(inMinusKILL.begin(), inMinusKILL.end());
						bbOUT = cal_OUT;                              //OUT(X) = GEN(X)+(IN(X)-KILL(X))

						if (old_out != bbOUT){
							change = true;
						}
					}
				}
			}
		}

	private:
		void make_udchain(Function &F){
			//내가 다시 만들 예정
			for(Instruction& I: instructions(F)){
				set<Instruction*> inSet = IN.find(&I)->second;

				if(is_ignorableInst(&I)){
					continue;
				}
				udChain.insert(pair<Instruction*, set<Instruction*>&>(&I, * new set<Instruction*>()));
				set<Instruction*>& defSet = udChain.find(&I)->second;
				if(isa<StoreInst>(&I)){
					if(Instruction* use = dyn_cast<Instruction>(I.getOperand(0))) defSet.insert(use);
				}
				else {
					for(int i = 0 ; i < I.getNumOperands() ; i++){
						if(Instruction* use = dyn_cast<Instruction>(I.getOperand(i))){
							if(isa<AllocaInst>(use)) {
								// 이 Load 의 def 는 StoreInstruction 임
								for(Instruction* inInst: inSet){
									if(StoreInst* inSi = dyn_cast<StoreInst>(inInst)){
										if (inSi->getOperand(1)->getName() == I.getOperand(i)->getName())
											defSet.insert(inSi);
									}
								}
							}
							else defSet.insert(use);
						}
					}
				}
			}
		}
		void makeExploitInstSet(){
			set<Instruction*> resultSet;
			for(set<Instruction*>::iterator iter = exploitInst.begin(); iter != exploitInst.end(); iter++){
				Instruction* inst = *iter;
				set<Instruction*> defSet = getDefSet(inst);
				resultSet.insert(inst);
				for(set<Instruction*>::iterator defIter = defSet.begin(); defIter != defSet.end() ; defIter++){
					Instruction* defInst = *defIter;
					resultSet.insert(defInst);
				}
			}
			results = resultSet;
			printVars(results);
		}
		set<Instruction*> getDefSet(Instruction* i){
			//get deep def set
			SmallVector<Instruction*, 20> workList;
			map<Instruction*, unsigned> instDepth;
			set<Instruction*> resultSet;
			set<Instruction*> defSet = udChain.find(i)->second;
			insertWorkList(workList, defSet);

			while(!workList.empty()){
				Instruction* back = workList.pop_back_val();
				resultSet.insert(back);
				if(instDepth.find(back)!= instDepth.end()){
					continue;
				} else{
					instDepth.insert(pair<Instruction*, unsigned>(back, 1));
					set<Instruction*> newDefSet = udChain.find(back)->second;
					insertWorkList(workList, newDefSet);
				}
			}
			return resultSet;
		}


		void insertWorkList(SmallVector<Instruction*, 20>& workList, set<Instruction*>& workSet){
			for(set<Instruction*>::iterator it = workSet.begin(); it!=workSet.end(); it++){
				Instruction* inst = *it;
				workList.push_back(inst);
			}
		}


		private:  void printINOUT(int bb_number, BasicBlock *bb){
		//	if (IN.count(bb) < 1 || OUT.count(bb) < 1) return ; // throw exception

			errs() << "BASIC BLOCK[" << bb_number << "]" << "\n";

			// errs() <<  "  IN  : ";
			// printVars(IN.find(bb)->second);

			// errs() <<  "  OUT : ";
			// printVars(OUT.find(bb)->second);

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

		private:  void printVars(set<Instruction *> vars) {
			for( set<Instruction *>::iterator v = vars.begin(); v != vars.end(); v++){
				Instruction* inst = *v;
				errs() << "Intstruction : " ;
				inst->print(errs());
				errs() << "         variable : " << inst->getName() << "\n" ;
			}
			errs() << "\n";
		}
	};

	struct BackwardPass : public FunctionPass{
		static char ID;

		//map< pair<use_pc, var>, def_pc>
		map<pair<Instruction *, string>, Instruction *> btd_table;
		map<pair<unsigned, unsigned>, pair<string, string>> analysisResultTable; //Add 2019.04.17 <<useLine, DefLine>, <Use var, Result>>
		map<BasicBlock*, set<pair<Instruction *, string>>&> USE;
		map<BasicBlock*, set<pair<Instruction *, string>>&> DEF;
		map<BasicBlock*, set<pair<Instruction *, string>>&> IN;
		map<BasicBlock*, set<pair<Instruction *, string>>&> OUT;
		map<Function*, int> functionDepth;

		vector<Function*> workList;
		Forward* FoResult;
		Function* holeFunc;
		set<Instruction*> inputInsts;

		BackwardPass() :FunctionPass(ID) {};

	public:
		map<pair<Instruction *, string>, Instruction *> getBTDTable() { return btd_table;}
		set<pair<Instruction *, string>> getFunctionIn(Function* f) { return IN.find(&f->getEntryBlock())->second;}
		bool runOnFunction(Function& f) override{
			doInitialization(f);
			for (Function::iterator bb = f.begin(); bb != f.end(); bb++) {
				BasicBlock &temp = *bb;
				runOnBasicBlock(temp);
			}
			eval_AllINOUT(f);
			eval_btd_table(f);
;
			// int bb_count = 0;
			// for (Function::iterator bb = f.begin(); bb != f.end(); bb_count++,bb++) {
			// 	BasicBlock &temp = *bb;
			// 	printUSEDEF(bb_count, dyn_cast<BasicBlock>(bb));
			// 	printINOUT(bb_count, dyn_cast<BasicBlock>(bb)); // optional
			// 	printINOUT(bb_count, &temp);
			// }

			//printTable(btd_table);

			// 결과 받는 분석하기

		}

		bool runOnBasicBlock(BasicBlock& b){
			eval_USEDEF(&b);
		}

		bool  doInitialization(Function &f)  {
			for (Function::iterator bb = f.begin(); bb != f.end(); bb++) {
				BasicBlock &temp = *bb;
				USE.insert(pair<BasicBlock*, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));
				DEF.insert(pair<BasicBlock*, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));
				IN.insert(pair<BasicBlock*, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));
				OUT.insert(pair<BasicBlock*, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));
			}
			findInput(f);
			return true;
		}

		private:  bool is_inDEF(BasicBlock * bb, string s){ // check <P,  src>  in  DEF(X)
			if (DEF.count(bb) < 1) return false; // throw exception

			set<pair<Instruction *, string>> & bbDEF = DEF.find(bb)->second;
			int checkIn = 0;

			for(set<pair<Instruction *,string>>::iterator i = bbDEF.begin() ; i != bbDEF.end(); i++){
				if(i->second == s){
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

			if(isa<AllocaInst>(inst)) return true;
			if(isa<CallInst>(inst)) {
				CallInst* ci = dyn_cast<CallInst>(inst);
				if(ci->getCalledFunction()->getName()=="llvm.dbg.declare") return true;
			}
			return
			(opcodeName == "br" && numOperands == 1)
			// unconditional branch : no src or dest
			|| (opcodeName == "alloca") ;
			// alloca :  no src or dest

		}

		public :
			bool isMUIInst(Instruction* inst){
				set<Instruction*> taintedInsts = getTaintedInstructions(inst);


				//처음에는 functiond의 in으로 했으나 in으로 하면 함수내에서 선언된 포인터에 문제가 생김
				//어찌보면 alias 문제
				//그래서 함수의 인자 파악하는것으로 만들기
//				set<pair<Instruction *, string>> funcIn = getFunctionIn(inst->getParent()->getParent());

				for(set<Instruction *>::iterator tIter = taintedInsts.begin(); tIter!=taintedInsts.end(); tIter++){
					Instruction* inInst = *tIter;
					if(inputInsts.find(inInst)!=inputInsts.end()) return true;
				}
				return false;
			}
		private:
			void findInput(Function& F){
				BasicBlock& BB = F.getEntryBlock();
				for(Instruction& I: BB){
					if(isa<StoreInst>(&I)){
						if(I.getOperand(1)->getName().find("addr")!=string::npos) inputInsts.insert(&I);
					}
				}
			}
			set<Instruction*> getTaintedInstructions(Instruction* inst){
				//
				set<Instruction*> result;
				SmallVector<Instruction*, 20> workIList;

				workIList.push_back(inst);

				while(!workIList.empty()){

					Instruction* iterInst = workIList.pop_back_val();
					if(result.count(iterInst) == 1) continue;
					if(!iterInst) continue;
					result.insert(iterInst);

					if(isa<StoreInst>(iterInst)){
						if(isa<Constant>(iterInst->getOperand(0))) continue;
						if(iterInst->getOperand(0)->getName() == "") continue;
						if(btd_table.find(pair<Instruction*, string>(iterInst, iterInst->getOperand(0)->getName())) == btd_table.end()) continue;
						Instruction* defInst = btd_table.find(pair<Instruction*, string>(iterInst, iterInst->getOperand(0)->getName()))->second;
						workIList.push_back(defInst);
					} else {
						for(int i = 0 ; i < iterInst->getNumOperands(); i++){
							if(isa<Constant>(iterInst->getOperand(i))) continue;
							if(iterInst->getOperand(i)->getName() == "") continue;
							if(btd_table.find(pair<Instruction*, string>(iterInst, iterInst->getOperand(i)->getName())) == btd_table.end()) continue;
							Instruction* defInst = btd_table.find(pair<Instruction*, string>(iterInst, iterInst->getOperand(i)->getName()))->second;
							workIList.push_back(defInst);
						}
					}

				}
				return result;
			}

		private: bool is_ignorableOprd(Instruction * inst, unsigned int i_oprd) {
			string opcodeName = inst->getOpcodeName();
			return (opcodeName == "br" && i_oprd > 0);

		}

		private: bool is_destOprd(Instruction * inst, unsigned int i_oprd) {
			string opcodeName = inst->getOpcodeName();

			return (opcodeName == "store" && i_oprd == 1);

		}

		private:
			void eval_USEDEF(BasicBlock * bb) {
			if (USE.count(bb) < 1 || DEF.count(bb) < 1) return; // throw exception

			set<pair<Instruction *, string>> & bbUSE = USE.find(bb)->second;

			set<pair<Instruction *, string>> & bbDEF = DEF.find(bb)->second;


			for (BasicBlock::iterator inst = bb->begin(); inst != bb->end(); inst++) {
				Instruction &bbInst = *inst;
				if(is_ignorableInst(&bbInst)) continue;

				//ADD HERE RETE 2019.04.18
				if(isa<CallInst>(&bbInst)){
					//Call Value 이름 바꾸기
					CallInst *cl = cast<CallInst>(inst);

					if( cl->getCalledFunction()->isDebugInfoForProfiling()) continue;
					string calledFunctionName = cl->getCalledFunction()->getName();
					if(cl->getType()->isVoidTy()) continue;
					cl->setName(calledFunctionName + ".return");

					// if(isRUICall(calledFunctionName)) {
					// 	cl->setName();
					// }
					for(int i = 1 ; i < cl->arg_size() ; i++){
						Value* arg = cl->getArgOperand(i);
						if( Constant* c_arg = dyn_cast<Constant>(arg)) continue;
						if( arg->getName() == "") continue;
						if(is_inDEF(bb, arg->getName())){
							btd_table.insert(pair<pair<Instruction *, string>, Instruction *>(pair<Instruction *, string>(&bbInst, arg->getName()),
							getPC_inDEF(bb, arg->getName())));
						}
						else if (!is_inDEF(bb, arg->getName()))
							bbUSE.insert(pair<Instruction*, string>(&bbInst, arg->getName()));
					}
				}

				if(LoadInst* li = dyn_cast<LoadInst>(&bbInst)){

					if (li->getName() =="") li->setName(li->getOperand(0)->getName()+".temp");
					pair<Instruction *,string> src (&bbInst, li->getOperand(0)->getName());
					pair<Instruction *,string> dst (&bbInst, li->getName());

					if(is_inDEF(bb, li->getOperand(0)->getName())){ // if  (<P,  src>  in  DEF(X))  for  some  P
						btd_table.insert(pair<pair<Instruction *, string>, Instruction *>(pair<Instruction *, string>(&bbInst, li->getOperand(0)->getName()),
						getPC_inDEF(bb, li->getOperand(0)->getName())));
						//                    g_table.insert(<BasicBlock*, pair<Instruction *, string>>(bb, pair<Instruction *, string(&bbInst, src_val))); // add here 19.03.29
					} else if(!is_inDEF(bb, li->getOperand(0)->getName()))
						bbUSE.insert(src);
					bbDEF.insert(dst);
				} else if(StoreInst* si = dyn_cast<StoreInst>(&bbInst)){
					Value* op1 = si->getOperand(0); //source
					Value* op2 = si->getOperand(1); //dst

					if(isa<Constant>(op1)) continue; // constant면 할필요 없음
					if(op1->getName() == "") op1->setName("store.value.tmp");
					if(op2->getName() == "") op2->setName("store.ptr.tmp");
					pair<Instruction *,string> src (&bbInst, op1->getName());
					pair<Instruction *,string> dst (&bbInst, op2->getName());
					bbDEF.insert(dst);

					if(!is_inDEF(bb, op1->getName()))
						bbUSE.insert(src);
					else if(is_inDEF(bb, si->getOperand(0)->getName())){ // if  (<P,  src>  in  DEF(X))  for  some  P

						btd_table.insert(pair<pair<Instruction *, string>, Instruction *>(pair<Instruction *, string>(&bbInst, si->getOperand(0)->getName()),
						getPC_inDEF(bb, si->getOperand(0)->getName())));
					}
				} else {
					//Add Here Rete 2019.04.17 - Null string Pass
 					if (isa<BranchInst>(&bbInst)) {
 						continue;
 					}
					string opcodeName = bbInst.getOpcodeName();
					unsigned int numOperands = bbInst.getNumOperands();

					for (int i_oprd = 0; i_oprd < numOperands; i_oprd++) {
						string src_val = bbInst.getOperand(i_oprd)->getName();
						if(src_val == "") continue;
						pair<Instruction *,string> src (&bbInst, src_val);

						if(is_inDEF(bb, src_val)){ // if  (<P,  src>  in  DEF(X))  for  some  P
							btd_table.insert(pair<pair<Instruction *, string>, Instruction *>(pair<Instruction *, string>(&bbInst, src_val), getPC_inDEF(bb, src_val)));
							//                    g_table.insert(<BasicBlock*, pair<Instruction *, string>>(bb, pair<Instruction *, string(&bbInst, src_val))); // add here 19.03.29
						} else if (!is_inDEF(bb, src_val)){ // src  not  in  DEF(X) : USE(X)  +=  <current_pc,  src>
							bbUSE.insert(pair<Instruction *, string>(&bbInst, src_val));
						}
					}
					if(!bbInst.getType()->isVoidTy() ){
						if(bbInst.getName() == "") bbInst.setName(string(bbInst.getOpcodeName())+".temp");
						bbDEF.insert(pair<Instruction*, string>(&bbInst, bbInst.getName()));     // DEF(X)  +=  <current_pc,  dest>
					}
				}
				if(bbInst.getType()->isVoidTy() ) continue;
				if(bbInst.getName() == "") bbInst.setName(string(bbInst.getOpcodeName())+".temp");
				bbDEF.insert(pair<Instruction*, string>(&bbInst, bbInst.getName()));     // DEF(X)  +=  <current_pc,  dest>
				if(is_inDEF(bb, bbInst.getName()) && isOtherDef(&bbInst)){
					bbDEF.erase(make_pair(&bbInst, bbInst.getName()));
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

		private:
		bool isOtherDef(Instruction* inst){
			//Other Def is on only StoreInst.
			if(!isa<StoreInst>(inst)) return false;
			bool result = false;
			int pos;
			int i = 0;
			for ( BasicBlock::iterator iter = inst->getParent()->begin(); iter != inst->getParent()->end(); iter++)
			{
				Instruction& I = *iter;
				if( inst == &I){
					pos = i;
					i++;
					continue;
				}
				if(StoreInst* si = dyn_cast<StoreInst>(&I)){
					if(si->getOperand(1) == inst->getOperand(1)){
						result = pos < i ||result;
					}
				}
				i++;
			}
			return result;
		}

		void eval_btd_table(Function &F){
			//use def chain
			for(Instruction& I: instructions(F)){
				set<pair<Instruction *, string>> & bbIN = IN.find(I.getParent())->second;
				if(StoreInst* si = dyn_cast<StoreInst>(&I)){
					if(I.getOperand(0)->getName() == "") continue;

					if(!is_inDEF(I.getParent(), I.getOperand(0)->getName())){
						Instruction* def = findInstructionsInSet(bbIN, I.getOperand(0)->getName());
						if(def!=NULL) insert_btd_table(&I, I.getOperand(0)->getName(), def);
					}
				} else{
					for(int i = 0 ; i < I.getNumOperands() ; i++){
						if(I.getOperand(i)->getName() == "") continue;
						if(!is_inDEF(I.getParent(), I.getOperand(i)->getName())){
							if(isa<AllocaInst>(I.getOperand(i))){
								Instruction* def = findInstructionsInSet(bbIN, I.getOperand(i)->getName());
								if(def!=NULL) insert_btd_table(&I, I.getOperand(i)->getName(), def);
							}
						}
					}
				}
			}
		}

		void insert_btd_table(Instruction* use_pc, string opName, Instruction* def_pc){
				btd_table.insert(pair<pair<Instruction *, string>, Instruction *>(pair<Instruction *, string>(use_pc, opName), def_pc));
		}

		Instruction* findInstructionsInSet(set<pair<Instruction *, string>> & findSet, string name){
			//set<Instruction*> result;
			for(set<pair<Instruction *, string>>::iterator iter = findSet.begin(); iter !=findSet.end(); iter++){
				if(iter->second == name) return iter->first;
			}
			return NULL;
		}

		void eval_AllINOUT(Function &f){
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
					set<pair<Instruction *, string>> USEOUT;
					set_union(bbOUT.begin(), bbOUT.end(), bbUSE.begin(), bbUSE.end(),
						inserter(USEOUT, USEOUT.begin()));
					// set_difference(USEOUT.begin(), USEOUT.end(), bbDEF.begin(), bbDEF.end(),
					// 	inserter(newout_woDEF, newout_woDEF.begin()), isSamePairInstString);
					newout_woDEF = intersection(bbOUT, bbDEF);

					for(set<pair<Instruction *,string>>::iterator i = bbOUT.begin() ; i != bbOUT.end(); i++){
						if(is_inDEF(&temp, i->second)){
							// && getPC_inDEF(&temp, i->second) != i->first
							newout_withDEF.insert(pair<Instruction *, string>(i->first, i->second));
							btd_table.insert(pair<pair<Instruction *, string>, Instruction *>(pair<Instruction *, string>(i->first, i->second), getPC_inDEF(&temp, i->second)));
						}
					}
					bbIN = an_union(bbUSE, newout_woDEF);

					if (old_in != bbIN)
					change = true;
				}
			}
		}

		private:  void printUSEDEF(int bb_number, BasicBlock *bb){
			if (USE.count(bb) < 1 || DEF.count(bb) < 1) return ; // throw exception

			errs() << "BASIC BLOCK[" << bb->getName() << "]" << "\n";

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
				if (DILocation *uLoc = ((iter->first).first)->getDebugLoc()) {
					if(lineNum == uLoc->getLine()){
						check = true;
					}
				}
			}

			return check;
		}

		private:
			void printTable(map<pair<Instruction *, string>, Instruction *> table){
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

				string test = (iter->second)->getOperand(0)->getName();

				if(test == ""){
					test = (iter->second)->getOperand(1)->getName();
				}

				errs() << "<<USE line "<< useLine << " : "  ;
				(iter->first).first->print(errs());
				errs() << ", USE VAR:"  <<(iter->first).second;
				errs() << ">, DEF line "<< defLine << " : ";
				iter->second->print(errs());
				errs() << ">  , <DEF var> : " << test<< "===>" << analysisResult <<"\n\n";
				if(analysisResult == "MAYBE"){ //<<useLine, DefLine>, <Use var, Result>>
					analysisResultTable.insert(pair<pair<unsigned, unsigned>, pair<string, string>>(pair<unsigned, unsigned>(useLine, defLine), pair<string, string>((iter->first).second, analysisResult)));
					analysisResult = maybeReAnalysis(useLine, (iter->first).second);
				}

				defLine = NULL;

				//            //Debugging
				//            string test = (iter->second)->getOperand(0)->getName();
				//            errs() << "<<USE line "<< useLine << " : " << (iter->first).first << ", " << (iter->first).second << ">, DEF Var "<< test << " : " << iter->second << " >  ===>" << analysisResult <<"\n\n";

				//            errs() << "\n DEF ADDRESS getName : " << (iter->second)->getOperand(0)->getName() << "\n";
			}

			errs() << "########################################test###############################################" << "\n";
		}

		//Add Here 2019.04.17
		private : string outputAnaly(string val, unsigned defLine){
			if(val.find("addr") != std::string::npos){
				return "MUI";
			}
			else if(!findTable(defLine)){
				return "NO";
			}
			// else if (val.find("")){
			// 		return "MUI";
			// }
			return "NO";
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
				if(getDebugLocLine(temp) == line){
					kkk = (iter->first).second;
				}
			}
			return kkk;
		}


		private:
		void printVars(set<pair<Instruction *, string>> vars) {
			for( set<pair<Instruction *, string>>::iterator v = vars.begin(); v != vars.end(); v++){
				errs() << "Intstruction :";
				v->first->print(errs());
				errs() << " ==> "<<getDebugLocLine(v->first)<<"         variable : " << v->second << "\n" ;
			}
			errs() << "\n";
		}
		void printVars(set<Instruction *> vars) {
			for( set<Instruction *>::iterator v = vars.begin(); v != vars.end(); v++){
				Instruction* inst = *v;
				errs() << "Intstruction :";
				inst->print(errs());
				errs() << " ==> "<<getDebugLocLine(inst)<<"\n" ;
			}
			errs() << "\n";
		}
	};

	struct MFPass: public ModulePass{
		static char ID;

		set<pair<Function*, Instruction*>>functionDepth;

		Function* holeFunc;
		Forward* FoResult;
		BackwardPass* KP;

		string FileName;
		set<Instruction*> inputInsts;

		MFPass() :ModulePass(ID) {};
		void getAnalysisUsage(AnalysisUsage& AU) const {
			// AU.addRequired<Forward>();
			AU.addRequired<Forward>();
			AU.addRequired<BackwardPass>();
			AU.setPreservesAll();
		};
		bool runOnModule(Module& M) override{
			// Forward 분석결과 저장함
			// workList 방식으로 작동함
			// 일단 취약점이 존재하는 함수를 workList에 넣음
			//
			holeFunc = NULL;
			unsigned rate = 0; // 0 is NO;
			if(lineNumber == 0){
				errs() << "Please Enter crash line number\n";
				return false;
			}

			if(functionName == "") {
				errs() << "Please Enter Function Name\n";
				return false;
			}

			errs() << "Function Name: " << functionName <<": " << lineNumber <<"\n";

			FoResult = &getAnalysis<Forward>();

			for(auto &F: M){
				if(F.getName() == functionName ) holeFunc = &F;
				else if(F.getName().find(functionName) != string::npos) {
					holeFunc = &F;
				}
				if(holeFunc == NULL) {
					errs() <<"Pass could not found function!\n";
					return false;
				}
			}
			if(!holeFunc) {
				errs() << functionName << " has not found\n";
				return false;
			}
			findInputInsts();
			if (FoResult->checkInFunction(holeFunc)){
				// this case only by holeFunc == JNI function or function including system call (Read etc)
				errs() << "this case only by holeFunc == JNI \n";
				rate = 1;
				for(set<Instruction*>::iterator iter = inputInsts.begin(); iter != inputInsts.end(); iter++){
					if (FoResult->checkVulnerable(*iter)) rate = 2;
				}
			}
			SmallVector<pair<Function*, Instruction*>, 20> workList;

			for(set<Instruction*>::iterator iter = inputInsts.begin(); iter!= inputInsts.end(); iter++)
				workList.push_back(pair<Function*, Instruction*>(holeFunc, *iter));


			while(!workList.empty()) {
				// 조건을 빼논 이유 for 문에서 workList.end()를 처음에 받아놓고 조건검사할 때 다시 안갖고옴 ㅡㅡ

				pair<Function*, Instruction*> pairIter = workList.pop_back_val();
				Function* func = pairIter.first;

				Instruction* inst = pairIter.second;


				bool inputRelavant = runOnFunction(*func, *inst);
				if(!inputRelavant)  errs() << "This Function is not input Relavant ::: " <<func->getName() <<"\n";
				for( Function::user_iterator ui = func->user_begin(); inputRelavant &&ui != func->user_end(); ui++)
				{
					if(CallInst* ci = dyn_cast<CallInst>(*ui)){
						if(FoResult->checkInFunction(ci->getParent()->getParent())) {
							// caller argument 관계를 봐야지
							rate = 1;
							if (FoResult->checkVulnerable(dyn_cast<Instruction>(ci))){
								rate = 2;
							}
						}
						if( functionDepth.find(pair<Function*, Instruction*>(ci->getParent()->getParent(), dyn_cast<Instruction>(ci))) != functionDepth.end())	continue;
						functionDepth.insert(pair<Function*, Instruction*>(ci->getParent()->getParent(), dyn_cast<Instruction>(ci)));
						workList.push_back(pair<Function*, Instruction*> (ci->getParent()->getParent(), dyn_cast<Instruction>(ci)));
					}
					if(rate == 2) break;
				}
			}
			printRating(rate);
			errs() << "========Analyze END=========\n\n\n";
			return false;
		};

		private :
		void findInputInsts() {
			for(Instruction& I: instructions(*holeFunc)){
				DILocation *uLoc = I.getDebugLoc();
				if(!uLoc) continue;
				if(uLoc->getLine() == lineNumber){
					FileName = uLoc->getFilename();
					inputInsts.insert(&I);
				}
			}
		}
		void printRating(unsigned rate){
			if(rate == 0 ){
				errs() <<" The line "<< lineNumber <<" of "<<holeFunc->getName()<<" of "<< FileName <<" is NO\n";
			} else if (rate == 1 ){
				errs() <<" The line "<< lineNumber <<" of "<<holeFunc->getName()<<" of " << FileName <<" is MUI\n";
			} else if (rate == 2 ) {
				errs() <<" The line "<< lineNumber <<" of "<<holeFunc->getName()<<" of " << FileName <<" is RUI\n";
			}
		}
		string outputAnaly(string val, unsigned defLine){
			if(val.find("addr") != std::string::npos){
				return "MUI";
			}
			return "NO";
		}

		bool runOnFunction(Function& F, set<Instruction*>& insts){
			bool result = false;
			KP = &getAnalysis<BackwardPass>(F);
			for(set<Instruction*>::iterator iter = insts.begin(); iter!=insts.end(); iter++){
				Instruction* inst = *iter;
				result = result || analyzeResult(*inst);
			}
			return result;
		}
		bool runOnFunction(Function& F, Instruction& I) {
			// 결과는 분석을 이어가야할지 아니면 멈춰야할지를 분석함
			//      < use_pc,       var   > , def_pc
			KP = &getAnalysis<BackwardPass>(F);
			return analyzeResult(I);
		}

		bool analyzeResult(Instruction& I){
			// 결과가 RUI거나 No면 True, MUI면 false
			errs()<<"Analyzer Result : " ;
			I.print(errs());
			errs() <<"\n";
			Instruction* vulnerableInst = &I;
			bool result = KP->isMUIInst(&I);
			string resultstring = result ? "True": "False";
			return KP->isMUIInst(&I);
		}

		pair<Instruction*, string> findUse(Instruction* inst){
			// 굳이 이걸 쓰는 이유는 StoreInst 때문에
			return pair<Instruction *, string>(inst, getDefVar(inst));
		}
		void analyzieRating(){

		}
	};
}

char Forward::ID = 0;
char BackwardPass::ID = 1;
char AnantaPass::ID = 2;
static RegisterPass<Forward>   XX("fo", "Forward pass");
static RegisterPass<BackwardPass> X("ba", "Backward pass");
static RegisterPass<MFPass> XXX("mf", "MF pass");
