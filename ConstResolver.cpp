//===- ConstResolver.cpp - Resolving constants for llvm::Constant---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/BitVector.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>

using namespace llvm;

#define DEBUG_TYPE "const-resolver"

namespace {
  struct ConstResolver : public ModulePass {
    static char ID;
    ConstResolver() : ModulePass(ID) {}
   
     bool runOnModule(Module &M) override {
 
      // Testing code. Do not change the following code.
      for (auto It = M.global_begin(), E = M.global_end(); It != E; ++It) {
           auto* G = &(*It);

           outputBitVector(getBitVector(G));
           if (G->hasInitializer()) outputBitVector(getBitVector(G->getInitializer()));
      }

      for (auto& F : M) {
           outputBitVector(getBitVector(&F));
           for (auto& B : F) {
               for (auto& I : B) {
                   for (unsigned Idx = 0, EIdx = I.getNumOperands(); Idx != EIdx; ++Idx) {
                           if (Constant* C = dyn_cast<Constant>(I.getOperand(Idx))) {
                                  outputBitVector(getBitVector(C));
                           }
                   }
               }
           }
      }
      return false;
    }

    std::shared_ptr<BitVector> getBitVector(Constant* Const) {
	 std::shared_ptr<BitVector> BV  = std::make_shared<BitVector>(0,false);
	 if (isa<Function>(Const)){
		//errs()<<"Function"<<"\n";
	// }else if(ConstantInt* CI = dyn_cast<ConstantInt>(Const)){
	//	errs()<<CI->getBitWidth()<<"\n";
	 }else if(ConstantInt* CI = dyn_cast<ConstantInt>(Const)){
		int bitwidth = CI->getBitWidth();
		int value = CI->getValue().getLimitedValue();
		int i;
		BV = std::make_shared<BitVector>(bitwidth,false);
		for (i = 0; i < bitwidth; ++i) { 
			if((CI->getValue().getLimitedValue() >> i) & 1)
				BV->set(i);
		}
	 }else if(ConstantFP* FP = dyn_cast<ConstantFP>(Const)){
	 	uint64_t Value = FP->getValueAPF().bitcastToAPInt().getLimitedValue();
	 	int i;
		BV = std::make_shared<BitVector>(64,false);
		for (i = 0; i < 64; ++i) { 
			if((Value >> i) & 1)
				BV->set(i);
		}
		//errs() <<"-----------------> FP -------------->";
	 }else if(dyn_cast<GlobalValue>(Const)){
		//errs()<<"Skip the Global Value"<<"\n";
	 }else if(dyn_cast<ConstantAggregateZero>(Const)){
		//errs()<<"Skip the Aggregate Zeor"<<"\n";
	 }else if(dyn_cast<ConstantDataSequential>(Const)){
		//errs()<<"Skip the Array"<<"\n";
	 }else if(dyn_cast<ConstantExpr>(Const)){
		//errs()<<"Skip the Expr"<<"\n";
	 }else if(dyn_cast<ConstantPointerNull>(Const)){
		//errs()<<"Skip the NULL POINTER"<<"\n";
	 }else if(ConstantStruct* CS = dyn_cast<ConstantStruct>(Const)){


	 	if (dyn_cast<Function>(CS->getOperand(1))) {
	 		//errs()<<"Function"<<"\n";
	 	}else{
	 		int x = CS->getType()->getNumElements();
	 		for(int xx=0 ; xx<x; xx++){
	 			getBitVector(dyn_cast<Constant>(CS->getOperand(xx)));
	 		}
	 		

		}

	 }else if(dyn_cast<ConstantDataSequential>(Const)){
		//errs()<<"Skip the Array"<<"\n";
	 }else if(ConstantArray* CA = dyn_cast<ConstantArray>(Const)){
 		//errs()<< CA->getOperand(0)<<"\n";
		//CA->dump();
	 }else if(dyn_cast<ConstantExpr>(Const)){
		//errs()<<"Skip the Expr"<<"\n";
	 }else if(dyn_cast<BlockAddress>(Const)){
		//errs()<<"Skip the NULL POINTER"<<"\n";
	 }else if(dyn_cast<ConstantVector>(Const)){
		//errs()<<"Skip the NULL POINTER"<<"\n";
	 }else if(UndefValue* UV = dyn_cast<UndefValue>(Const)){
		unsigned x =2;
		errs() << UV->getElementValue(x);

	 }else{
		
		errs()<<"UnCaught Cases"<<"\n";
	}
         return BV;
    }

    void outputBitVector(std::shared_ptr<BitVector> BV) {
        	int i = BV->size()-1;
		for (; i >=0; --i) { 
			if(BV->test(i)){
				//errs()<<1;
			}
			else{
				//errs()<<0;
			}
		}
		//errs() << "\n";	
		// Output BV to stdin in proper way
    }

    // TODO Writing code here (e.g. adding functions)
  };
}

char ConstResolver::ID = 0;
static RegisterPass<ConstResolver> X("const-resolver", "Resolving llvm::Constant");
