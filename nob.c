#define NOB_IMPLEMENTATION
#include "nob.h"

#define EXE_NAME "hotreload"
#define BUILD_DIR "build/"
#define SRC_DIR "src/"

#define PLUG_LIB_NAME "plug.so"

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_DIR)) return 1;

    Nob_Cmd cmd = {0};

    nob_cmd_append(&cmd, "cc", "-I./"SRC_DIR, "-fsanitize=address", "-lraylib");
    nob_cmd_append(&cmd, "-o", BUILD_DIR""EXE_NAME);
    nob_cmd_append(&cmd, SRC_DIR"main.c", SRC_DIR"linux_hotreload.c");

    if (!nob_cmd_run(&cmd)) return 1;

    nob_cmd_append(&cmd, "cc", "-I./"SRC_DIR, "-lraylib");
    nob_cmd_append(&cmd, "-shared", "-fPIC", "-o", BUILD_DIR""PLUG_LIB_NAME);
    nob_cmd_append(&cmd, SRC_DIR"plug.c");

    if (!nob_cmd_run(&cmd)) return 1;

    return 0;
}

