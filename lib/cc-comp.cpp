//------------------------------------------------------------------------------
// bb_toposort_sccs LLVM sample. Demonstrates:
//
// * How to implement DFS & topological sort over the control-flow graph (CFG)
//   of a function.
// * How to use po_iterator for post-order iteration over basic blocks.
// * How to use scc_iterator for post-order iteration over strongly-connected
//   components in the graph of basic blocks.
// //------------------------------------------------------------------------------
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
#include <vector>

using namespace llvm;

class CC_Metric : public FunctionPass 
{
    public:
        CC_Metric()
            : FunctionPass(ID), cc(0) {}
        
        virtual bool runOnFunction(Function &F) 
        {
            cc = 0;
            for (Function::iterator bb = F.begin(); bb != F.end(); bb++) 
            {
                dfs_metadata[&(*bb)] = DFS_Color::white;
            }

            dfs(&(*F.begin()));
            outs() << F.getName() << ": Cyclomatic Complexity = " << cc << "\n";
            dfs_metadata.clear();
            return false;
        }
    
    private:
        
        void dfs(BasicBlock* bb) 
        {
            dfs_metadata[bb] = DFS_Color::black;

            if (succ_empty(bb)) 
            {
                cc++;
            } else {
                for (BasicBlock* successor : successors(&(*bb))) 
                {
                    if (dfs_metadata[successor] != DFS_Color::black) {
                        dfs(successor);
                    } else {
                        cc++;
                    }
                }
            }
        }

        enum class DFS_Color {
            white = 0,
            black
        };

        std::map<BasicBlock*, DFS_Color> dfs_metadata;
        unsigned int cc;

        static char ID;
};

char CC_Metric::ID = 0;

int main(int argc, char **argv) 
{
    if (argc < 2) 
    {
        // Using very basic command-line argument parsing here...
        errs() << "Usage: " << argv[0] << " <IR file>\n";
        return 1;
    }

    // Parse the input LLVM IR file into a module.
    SMDiagnostic Err;
    LLVMContext Context;
    std::unique_ptr<Module> Mod(parseIRFile(argv[1], Err, Context));
    if (!Mod) {
        Err.print(argv[0], errs());
        return 1;
    }

    // Create a pass manager and fill it with the passes we want to run.
    legacy::PassManager PM;
    PM.add(new CC_Metric());
    PM.run(*Mod);

    return 0;
}
