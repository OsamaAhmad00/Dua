#include "FileTestCasesRunner.hpp"

namespace dua
{

TEST(typename_operator, typename_operator) {
    FileTestCasesRunner("typename-operator.dua").run();
}

}
