#pragma once

#include <types/Type.hpp>

namespace dua
{

struct NonConcreteType : public Type
{
    virtual const Type* get_concrete_type() const = 0;
};

// A type that evaluates to the same type given that the
//  fields don't change
struct StaticType : public NonConcreteType {};

// A type that can evaluate to different fields depending
//  on the current context
struct DynamicType : public NonConcreteType {};

}