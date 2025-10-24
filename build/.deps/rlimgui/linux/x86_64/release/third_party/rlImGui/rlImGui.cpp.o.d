{
    files = {
        "third_party/rlImGui/rlImGui.cpp"
    },
    depfiles_format = "gcc",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-fvisibility=hidden",
            "-fvisibility-inlines-hidden",
            "-w",
            "-O3",
            "-Ithird_party/cimgui/imgui",
            "-Ithird_party/rlImGui",
            "-Ibuild/.gens/rlimgui/linux/x86_64/release/platform/windows/idl",
            "-isystem",
            "/home/tom/.xmake/packages/r/raylib/5.5/efaf75c5cc7042868d9f9fc4196a080a/include",
            "-isystem",
            "/usr/include/X11/dri",
            "-DNDEBUG"
        }
    },
    depfiles = "rlImGui.o: third_party/rlImGui/rlImGui.cpp third_party/rlImGui/rlImGui.h  third_party/rlImGui/extras/IconsFontAwesome6.h  third_party/rlImGui/imgui_impl_raylib.h third_party/cimgui/imgui/imgui.h  third_party/cimgui/imgui/imconfig.h  third_party/rlImGui/extras/FA6FreeSolidFontData.h\
"
}