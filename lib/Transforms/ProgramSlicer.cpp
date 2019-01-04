#include "Transforms/ProgramSlicer.h"

#include "Analysis/Partitioner.h"
#include "Analysis/ProgramPartitionAnalysis.h"
#include "Utils/Utils.h"
#include "Utils/Logger.h"
#include "Utils/Statistics.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/PassRegistry.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include <algorithm>
#include <iterator>

namespace vazgen {

ProgramSlicer::ProgramSlicer(llvm::Module* M,
                             Slice slice,
                             Logger& logger)
    : m_module(M)
    , m_slice(slice)
    , m_slicedModule(nullptr)
    , m_logger(logger)
{
}

bool ProgramSlicer::slice()
{
    bool modified = false;
    std::unordered_set<std::string> function_names;
    std::vector<std::pair<llvm::Function*, llvm::Function*>> function_mapping;
    for (auto currentF : m_slice) {
        if (!currentF) {
            continue;
        }
        if (currentF->isDeclaration()) {
            continue;
        }
        if (currentF->getName() == "main") {
            m_logger.warn("Function main in the partition. Can not slice main away");
            continue;
        }
        modified = true;
        function_names.insert(currentF->getName());
    }
    if (!modified) {
        return modified;
    }
    createSliceModule(function_names);
    return modified;
}

llvm::Function* ProgramSlicer::createFunctionDeclaration(llvm::Function* originalF)
{
    auto* functionType = originalF->getFunctionType();
    const std::string function_name = originalF->getName();
    const std::string clone_name = function_name + "_clone";
    return llvm::dyn_cast<llvm::Function>(m_module->getOrInsertFunction(clone_name, functionType));
}

bool ProgramSlicer::changeFunctionUses(llvm::Function* originalF, llvm::Function* cloneF)
{
    for (auto it = originalF->user_begin(); it != originalF->user_end(); ++it) {
        if (auto* callInst = llvm::dyn_cast<llvm::CallInst>(*it)) {
            callInst->setCalledFunction(cloneF);
        } else if (auto* invokeInst = llvm::dyn_cast<llvm::InvokeInst>(*it)) {
            invokeInst->setCalledFunction(cloneF);
        } else {
            (*it)->replaceUsesOfWith(originalF, cloneF);
        }
    }
    return true;
}

void ProgramSlicer::createSliceModule(const std::unordered_set<std::string>& function_names)
{
    llvm::ValueToValueMapTy value_to_value_map;
    m_slicedModule =  llvm::CloneModule(m_module, value_to_value_map,
                [&function_names] (const llvm::GlobalValue* glob) {
                    return function_names.find(glob->getName()) != function_names.end();
                }
                );
    m_slicedModule->setModuleIdentifier("slice");
    for (auto it = m_slicedModule->begin(); it != m_slicedModule->end(); ++it) {
        if (it->isDeclaration() && it->user_empty()) {
            auto* new_F = &*it;
            ++it;
            new_F->dropAllReferences();
            new_F->eraseFromParent();
        }
    }
}

char ProgramSlicerPass::ID = 0;

void ProgramSlicerPass::getAnalysisUsage(llvm::AnalysisUsage& AU) const
{
    AU.addRequired<ProgramPartitionAnalysis>();
    AU.setPreservesAll();
}

bool ProgramSlicerPass::runOnModule(llvm::Module& M)
{
    Logger logger("program-partitioning");
    logger.setLevel(vazgen::Logger::INFO);

    bool modified = sliceForPartition(logger, M, true);
    modified |= sliceForPartition(logger, M, false);
    return modified;
}

bool ProgramSlicerPass::sliceForPartition(Logger& logger, llvm::Module& M, bool enclave)
{
    Partition partition;
    std::string sliceName;
    if (enclave) {
        partition = getAnalysis<ProgramPartitionAnalysis>().getProgramPartition().getSecurePartition();
        sliceName = "enclave_slice.bc";
    } else {
        partition = getAnalysis<ProgramPartitionAnalysis>().getProgramPartition().getInsecurePartition();
        sliceName = "app_slice.bc";
    }
    auto partitionFs = partition.getPartition();
    ProgramSlicer::Slice slice(partitionFs.size());
    std::copy(partitionFs.begin(), partitionFs.end(), std::back_inserter(slice));
    m_slicer.reset(new ProgramSlicer(&M, slice, logger));
    bool modified = m_slicer->slice();
    if (modified) {
        logger.info("Slice done\n");
        Utils::saveModule(m_slicer->getSlicedModule().get(), sliceName);
    } else {
        logger.info("No slice\n");
    }
    return modified;
}

static llvm::RegisterPass<ProgramSlicerPass> X("slice-partition","Slice program for a partitioning");

} // namespace vazgen
