{
    depfiles_format = "gcc",
    depfiles = "lexer.o: src/lexer.c src/lexer.h src/common.h src/gcstring.h\
",
    files = {
        "src/lexer.c"
    },
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-fvisibility=hidden",
            "-Wall",
            "-Wextra",
            "-Werror",
            "-O3",
            "-std=c11",
            "-Idat",
            "-Ibuild/.gens/stack_calc/linux/x86_64/release/platform/windows/idl",
            "-Ithird_party/cimgui",
            "-Ibuild/.gens/cimgui/linux/x86_64/release/platform/windows/idl",
            "-Ithird_party/rlImGui",
            "-Ibuild/.gens/rlimgui/linux/x86_64/release/platform/windows/idl",
            "-DCIMGUI_DEFINE_ENUMS_AND_STRUCTS",
            "-isystem",
            "/home/tom/.xmake/packages/b/bdwgc/v8.2.6/2bbea4dad3df48319defa746f5bd80e4/include",
            "-isystem",
            "/home/tom/.xmake/packages/l/libatomic_ops/7.8.2/fe5fcc9e882e45c7a8e040e869caee57/include",
            "-isystem",
            "/home/tom/.xmake/packages/r/raylib/5.5/efaf75c5cc7042868d9f9fc4196a080a/include",
            "-isystem",
            "/usr/include/X11/dri",
            "-DNDEBUG"
        }
    }
}