        bool runOnModule(Module &M) override {
            ...
            for (auto &f: M) {
                doInitialization(f);
                for (auto &BB: f) {

                    runOnBasicBlock(BB);
                    
                    for (auto &I: BB) {
                        ...
                    }
                    doFinalization(f);
                }
            }
        }

