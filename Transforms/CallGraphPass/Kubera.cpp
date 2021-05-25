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


using namespace llvm;
using namespace std;


// Refer this web site: https://cpp.hotexamples.com/examples/-/CallGraphSCCPass/-/cpp-callgraphsccpass-class-examples.html
namespace{
	cl::opt<int> lineNumber("num", cl::desc("Please enter crash line number"), cl::init(0));
	static cl::opt<std::string> functionName("mapping-functio", cl::desc("The function name to be mapped"), cl::init(""));


	struct Forward : public ModulePass {
			static char ID;     // Pass identification
			Forward(): ModulePass(ID) { }

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

			virtual bool runOnModule(Module& M){
				for(Function& F: M){
					doInitialization(F);
					runOnFunction(F);
				}
			}
			bool runOnBasicBlock(BasicBlock &bb) {
					eval_GENKILL(&bb);
			}

			bool runOnFunction(Function& f){
				errs () << " Forward Analysis On\n";
				for (Function::iterator bb = f.begin(); bb != f.end() ; bb++){
					BasicBlock& temp = *bb;
					runOnBasicBlock(temp);
				}
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
			}

			bool  doInitialization(Function &f) {
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


									eval_differ(bbGEN, kill);                 // GEN(X) - K
									bbGEN.insert(gen.begin(), gen.end());     // G+(GEN(X)-K)

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
					for( set<pair<Instruction *, string>>::iterator v = vars.begin(); v != vars.end(); v++){
							errs() << "Intstruction : " ;
							v->first->print(errs());
							errs() << "         variable : " << v->second << "\n" ;
					}
					errs() << "\n";
			}
	};

	struct KuberaPass : public ModulePass{
		static char ID;

		map<pair<Instruction *, string>, Instruction *> btd_table;
		map<pair<unsigned, unsigned>, pair<string, string>> analysisResultTable; //Add 2019.04.17 <<useLine, DefLine>, <Use var, Result>>
		map<BasicBlock*, set<pair<Instruction *, string>>&> USE;
		map<BasicBlock*, set<pair<Instruction *, string>>&> DEF;
		map<BasicBlock*, set<pair<Instruction *, string>>&> IN;
		map<BasicBlock*, set<pair<Instruction *, string>>&> OUT;

		Function* holeFunc;

		KuberaPass() :ModulePass(ID) {};
		void getAnalysisUsage(AnalysisUsage& AU) const {
			errs() << "Forward analysis!\n";
			AU.addRequired<Forward>();
			AU.setPreservesAll();

		};
		bool runOnModule(Module& M) override{

			if(lineNumber == 0){
				 errs() << "Please Enter crash line number\n";
				 return false;
			}

			if(functionName == "") {
				errs() << "Please Enter Function Name\n";
				return false;
			}

			errs() << "Function Name: " << functionName <<": " << lineNumber <<"\n";
			for(auto &F: M){
				if(F.isDeclaration()) continue;
				doInitialization(F);
			}

			for(auto &F: M){
				if(F.getName() == functionName) {
					holeFunc = &F;
				}
			}
			if(!holeFunc) {
				errs() << functionName << " has not found\n";
				return false;
			}
			runOnFunction(*holeFunc);
			return false;
		};

		bool runOnFunction(Function& f){
			for (Function::iterator bb = f.begin(); bb != f.end(); bb++) {
					BasicBlock &temp = *bb;
					runOnBasicBlock(temp);
			}
			eval_AllINOUT(f);
			printTable(btd_table);
		}

		bool runOnBasicBlock(BasicBlock& b){
			eval_USEDEF(&b);
		}
		bool  doInitialization(Function &f) {
				for (Function::iterator bb = f.begin(); bb != f.end(); bb++) {
						BasicBlock &temp = *bb;
						USE.insert(pair<BasicBlock*, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));
						DEF.insert(pair<BasicBlock*, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));
						IN.insert(pair<BasicBlock*, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));
						OUT.insert(pair<BasicBlock*, set<pair<Instruction *, string>>&>(&temp, * new set<pair<Instruction *, string>>()));

				}
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

				Instruction * term_inst = bb->getTerminator();
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

						// if(analysisResult == "MAYBE"){ //<<useLine, DefLine>, <Use var, Result>>
						// 		analysisResultTable.insert(pair<pair<unsigned, unsigned>, pair<string, string>>(pair<unsigned, unsigned>(useLine, defLine), pair<string, string>((iter->first).second, analysisResult)));
						// 		analysisResult = maybeReAnalysis(useLine, (iter->first).second);
						// }

						string test = (iter->second)->getOperand(0)->getName();

						if(test == ""){
								test = (iter->second)->getOperand(1)->getName();
						}



//            analysisResultTable.insert(pair<pair<unsigned, unsigned>, pair<string, string>>(pair<unsigned, unsigned>(useLine, defLine), pair<string, string>((iter->first).second, analysisResult)));

//            analysisResultTable.insert();

						errs() << "<<USE line "<< useLine << " : " << (iter->first).first << ", " << (iter->first).second << ">, DEF line "<< defLine << " : " << iter->second << ">  , <DEF var> : " << test<< "===>" << analysisResult <<"\n\n";
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


		private:  void printVars(set<pair<Instruction *, string>> vars) {
				for( set<pair<Instruction *, string>>::iterator v = vars.begin(); v != vars.end(); v++){
						errs() << "Intstruction :";
						v->first->print(errs());
						errs() << " ==> "<<getDebugLocLine(v->first)<<"         variable : " << v->second << "\n" ;
				}
				errs() << "\n";
		}
	};
}

char Forward::ID = 0;
char KuberaPass::ID=1;
static RegisterPass<Forward>   XX("fo", "Forward pass");
static RegisterPass<KuberaPass> X("ku", "Kubera pass");
