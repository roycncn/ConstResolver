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
#include "llvm/IR/GlobalValue.h"
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
		errs()<<"Skip Function"<<"\n";

	 }else if(ConstantInt* CI = dyn_cast<ConstantInt>(Const)){
	 	errs()<<"ConstantInt :";
	 	CI->dump();
		int bitwidth = CI->getBitWidth();
		int value = CI->getValue().getLimitedValue();
		int i;
		BV = std::make_shared<BitVector>(bitwidth,false);

		for (i = 0; i < bitwidth; ++i) { 
			if((CI->getValue().getLimitedValue() >> i) & 1)
				BV->set(i);
		}
		return BV;

	 }else if(ConstantFP* FP = dyn_cast<ConstantFP>(Const)){

	 	uint64_t Value = FP->getValueAPF().bitcastToAPInt().getLimitedValue();
	 	int i;
		BV = std::make_shared<BitVector>(64,false);
		for (i = 0; i < 64; ++i) { 
			if((Value >> i) & 1)
				BV->set(i);
		}
		return BV;

	 }else if(GlobalValue *GV = dyn_cast<GlobalValue>(Const)){

	 	if(GV->getNumOperands()!=0){
			Type* type = GV->getOperand(0)->getType();
			Constant* GVC = dyn_cast<Constant>(GV->getOperand(0));
			if(type->isIntegerTy()){
				return getBitVector(GVC);
			}else if(type->isStructTy()){
				return getBitVector(GVC);
			}else if(type->isArrayTy()){
				return getBitVector(GVC);
			}else if(type->isPointerTy()){
				return getBitVector(GVC);
			
			}else{

				GV->getOperand(0)->getType()->dump();
				errs()<<"Not Handle Type"<<"\n";
			
			}
			
	 	}
		
	 
	 }else if(dyn_cast<ConstantAggregateZero>(Const)){
		errs()<<"ConstantAggregateZero"<<"\n";
	 }else if(ConstantDataSequential *CDS = dyn_cast<ConstantDataSequential>(Const)){
	 	errs()<<"\n";
	 	CDS->dump();
		int bitwidth = CDS->getElementByteSize()*CDS->getNumElements()*8;
		int elementbitwidth = CDS->getElementByteSize()*8;
		int i = 0;
		std::vector<bool> all;
			for (int j=0;j < CDS->getNumElements();j++){
				errs()<<"Data is: "<<CDS->getElementAsInteger(j)<<"| Bit is:";
				std::vector<bool> v;
				for(int k =0;k< elementbitwidth ;++k){
					if((CDS->getElementAsInteger(j)>> k) & 1){
						v.push_back(true);
					}else{
						v.push_back(false);
					}
						
				}
				reverse(v.begin(), v.end());

				for (std::vector<bool>::iterator it = v.begin() ; it != v.end(); ++it){
    				if(*it){
						errs() << "1";
						all.push_back(true);
    				}else{
						errs() << "0";
						all.push_back(false);
    				}
				}
				errs() << "\n";

				
			}

		
		return boolv2bitv(all);
		
	 }else if(dyn_cast<ConstantPointerNull>(Const)){
			errs()<<"ConstantPointerNull"<<"\n";
	 }else if(ConstantStruct* CS = dyn_cast<ConstantStruct>(Const)){

	 	if (dyn_cast<Function>(CS->getOperand(1))) {
	 		errs()<<"Skip Function"<<"\n";
	 	}else{

	 		int elementsNum = CS->getType()->getNumElements();
	 		for(int i=0 ; i<elementsNum; i++){
	 			return getBitVector(dyn_cast<Constant>(CS->getOperand(i)));
	 		}
	 		

		}

	 }else if(ConstantExpr *CE = dyn_cast<ConstantExpr>(Const)){
	 	unsigned int result = 0;

	 	if(isa<Function>(CE->getOperand(0))){

 				errs()<<"Skip Function"<<"\n";

 		}else if (strcmp(CE->getOpcodeName(),"getelementptr")==0){
				return getBitVector(CE->getOperand(0));
 		}else if (strcmp(CE->getOpcodeName(),"bitcast")==0){
 			    return getBitVector(CE->getOperand(0));
 		}else if (strcmp(CE->getOpcodeName(),"inttoptr")==0){
				return getBitVector(CE->getOperand(0));
 		}else if (strcmp(CE->getOpcodeName(),"ptrtoint")==0){
				return getBitVector(CE->getOperand(0));

 		}else {

 				errs()<<"UnCaught Operation "<<CE->getOpcodeName()<<"\n";
 		}
	 	return BV;

	 }else if(ConstantArray* CA = dyn_cast<ConstantArray>(Const)){
	 	//->consist of first class type 
	 	Constant *CAC = dyn_cast<Constant>(CA->getOperand(0));

 		if(isa<ConstantStruct>(CAC)){

 				return getBitVector(CAC);

 		}else if(isa<ConstantExpr>(CAC)){

 				return getBitVector(CAC);

 		}else if(isa<ConstantPointerNull>(CAC)){

 				errs()<<"ConstantPointerNull"<<"\n";
 		}else {

 			errs()<<"UnCaught ConstantArray"<<"\n";
 		}


	 }else if(UndefValue* UV = dyn_cast<UndefValue>(Const)){
		errs()<<"UndefValue"<<"\n";
	 }else{
		
		errs()<<"UnCaught Cases"<<"\n";
	}

		//Default Return
	    return BV;

    }

    void outputBitVector(std::shared_ptr<BitVector> BV) {
        int i = BV->size()-1;
        if(i<=0)
        	return;
        errs()<<"**OutputBitVector: ";
		for (; i >=0; --i) { 
			if(BV->test(i)){
				errs()<<1;
			}
			else{
				errs()<<0;
			}
		}
		errs() << "**\n";	

    }

    std::shared_ptr<BitVector> boolv2bitv(std::vector<bool> all){
		std::shared_ptr<BitVector> bitv  = std::make_shared<BitVector>(all.size(),false);
		reverse(all.begin(),all.end());
		int i=0;
		for (std::vector<bool>::iterator it = all.begin() ; it != all.end(); ++it){
    		if(*it){
				bitv->set(i);
    		}
    		i++;
		}


		return bitv;
    }


  };
}

char ConstResolver::ID = 0;
static RegisterPass<ConstResolver> X("const-resolver", "Resolving llvm::Constant");
