#include "AST/ASTNode.hpp"
#include "types/PointerType.hpp"
#include "types/NullType.hpp"

namespace dua
{

class NullPointerNode : public ASTNode
{

public:

    NullPointerNode(ModuleCompiler* compiler) { this->compiler = compiler; };

    Value eval() override { return compiler->create_value(llvm::ConstantPointerNull::get(builder().getInt1Ty()->getPointerTo()), get_type()); }

    const Type* get_type() override { return compiler->create_type<PointerType>(compiler->create_type<NullType>()); }
};

}
