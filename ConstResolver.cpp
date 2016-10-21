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
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>

using namespace llvm;

#define DEBUG_TYPE "const-resolver"

namespace {
  struct ConstResolver : public ModulePass {
    static char ID;
    ConstResolver() : ModulePass(ID) {}
    int total_count =0 ;
    int int_count=0;
    int nullp_count=0;
    int global_count=0;
    int function_count=0;
    int misc_count=0;
    int fp_count=0;
    int expr_count=0;
    bool runOnModule(Module &M) override {
      for (auto& F : M) {
           for (auto& B : F) {
               for (auto& I : B) {
                   for (unsigned Idx = 0, EIdx = I.getNumOperands(); Idx != EIdx; ++Idx) {
                           if (Constant* Const = dyn_cast<Constant>(I.getOperand(Idx))) {
                                  total_count++;
                                    if (isa<Function>(Const)) {
                                        function_count++;
                                    } else {
                                       if (ConstantInt* CI = (dyn_cast<ConstantInt>(Const))){
                                            int_count++;
                                       }else if (ConstantPointerNull* CNP = (dyn_cast<ConstantPointerNull>(Const))) {
                                            nullp_count++;                                  
                                       }else if (GlobalValue* GV = (dyn_cast<GlobalValue>(Const))) {
                                            global_count++;
                                       }else if (ConstantExpr* CE = (dyn_cast<ConstantExpr>(Const))) {
                                            expr_count++;
                                       }else if (ConstantFP* CF = (dyn_cast<ConstantFP>(Const))) {
                                            expr_count++;
                                       }else {
                                          errs() << "Misc No." <<misc_count<<"'s content is "<< "\n";
                                          (dyn_cast<Value>(Const))->dump(); 
                                            misc_count++;
                                       }
                                       
                                                  
                             }
 
                         }
                   }
               }
           }
      }

      // Testing code. Do not change the following code.
      for (auto It = M.global_begin(), E = M.global_end(); It != E; ++It) {
           auto* G = &(*It);

           outputBitVector(getBitVector(G));
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
     errs() << "Total is: " << total_count << "\n";
     errs() << "int_count is: " << int_count << "\n";
     errs() << "fp_count is: " << fp_count << "\n";
     errs() << "function_count is: " << function_count << "\n";
     errs() << "nullp_count is: " << nullp_count << "\n";
     errs() << "global_count is: " << global_count << "\n";
     errs() << "expr_count is: " << expr_count << "\n";
     errs() << "misc_count is: " << misc_count << "\n";
      return false;
    }

    std::shared_ptr<BitVector> getBitVector(Constant* Const) {

         // TODO Replace the return statement if necessary
         return std::make_shared<BitVector>(64, false);
    }

    void outputBitVector(std::shared_ptr<BitVector> BV) {
         // TODO Writing code here
         // Output BV to stdin in proper way
    }

    // TODO Writing code here (e.g. adding functions)
  };
}

char ConstResolver::ID = 0;
static RegisterPass<ConstResolver> X("const-resolver", "Resolving llvm::Constant");
