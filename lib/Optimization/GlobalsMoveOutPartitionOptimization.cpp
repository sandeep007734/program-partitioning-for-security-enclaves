#include "Optimization/GlobalsMoveOutPartitionOptimization.h"

#include "PDG/PDG/PDG.h"

namespace vazgen {

GlobalsMoveOutPartitionOptimization::
GlobalsMoveOutPartitionOptimization(Partition& partition, PDGType pdg)
    : PartitionOptimization(partition, pdg)
{
}

void GlobalsMoveOutPartitionOptimization::run()
{
    // TODO: think if this optimization is needed anyways
}

} // namespace vazgen

