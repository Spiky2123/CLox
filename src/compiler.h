//
// Created by scuta on 8/26/2026.
//

#ifndef CLOX_COMPILER_H
#define CLOX_COMPILER_H

#include "object.h"
#include "vm.h"
#include "chunk.h"

ObjFunction* compile(const char *source);
void markCompilerRoots();

#endif //CLOX_COMPILER_H
