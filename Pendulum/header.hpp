#pragma once

#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

void _setArgs(cl::Kernel ker, int i)
{
}

template<class A, typename... TYPE>
void _setArgs(cl::Kernel ker, int i, A var, TYPE... args)
{
	ker.setArg(i, var);
	_setArgs(ker, i+1, args...);
}

template<typename... TYPE>
void _setArgs(cl::Kernel ker, TYPE... args)
{
	_setArgs(ker, 0, args...);
}