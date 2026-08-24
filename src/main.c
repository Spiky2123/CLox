#include "common.h"
#include "chunk.h"
#include "debug.h"

int main(int argc, const char *argv[]) {
    Chunk chunk;
    initChunk(&chunk);
    writeChunk(&chunk, OP_RETURN, 123);

    for (int i = 0; i < 300; i++) {
        writeConstant(&chunk, i, 123);
    }

    disassembleChunk(&chunk, "test chunk");
    freeChunk(&chunk);
    return 0;
}
