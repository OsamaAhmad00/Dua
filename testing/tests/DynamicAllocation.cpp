#include "FileTestCasesRunner.h"

namespace dua
{

TEST(dynamic_allocation, dynamic_allocation) {
    FileTestCasesRunner("dynamic-allocation.dua").run();
}

}
