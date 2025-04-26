#include "Engine.h"

int main(int argc, char* argv[]) {
    Engine engine;
    if (!engine.Init()) return -1;

    engine.Run();
    return 0;
}
    