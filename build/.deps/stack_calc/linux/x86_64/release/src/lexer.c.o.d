{
    depfiles_format = "gcc",
    files = {
        "src/lexer.c"
    },
    depfiles = "build/.objs/stack_calc/linux/x86_64/release/src/__cpp_lexer.c.c:   src/lexer.c src/lexer.h src/common.h src/gcstring.h src/gcstructures.h\
",
    values = {
        "/usr/bin/clang",
        {
            "-Qunused-arguments",
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
            "/home/tom/.xmake/packages/b/bdwgc/v8.2.6/b8d70da35948460da0129f6c01f3c83b/include",
            "-isystem",
            "/home/tom/.xmake/packages/l/libatomic_ops/7.8.2/74feaeb625d74d0ab0a313e347b22d0a/include",
            "-isystem",
            "/home/tom/.xmake/packages/r/raylib/5.5/e384f591f7554392a29c0679d0c21ad8/include",
            "-isystem",
            "/usr/include/X11/dri",
            "-DNDEBUG"
        }
    }
}