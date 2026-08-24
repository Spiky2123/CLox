//
// Created by scuta on 8/24/2026.
//

#ifndef CLOX_CHUNK_H
#define CLOX_CHUNK_H

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_RETURN,
} OpCode;

typedef struct {
    int line;
    int count;
} LineStart;

typedef struct {
    int count;
    int capacity;
    uint8_t *code;

    int lineCount;
    int lineCapacity;
    LineStart *lines;
    ValueArray constants;
} Chunk;

void initChunk(Chunk *chunk);

void freeChunk(Chunk *chunk);

void writeChunk(Chunk *chunk, uint8_t byte, int line);

int addConstant(Chunk *chunk, Value value);

int getLine(Chunk *chunk, int index);

#endif //CLOX_CHUNK_H
