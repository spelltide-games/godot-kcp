#!/usr/bin/env python

env = SConscript("godot-cpp/SConstruct")

env.Append(
    CPPPATH=[
        "src",
        ".",
        "kcp",
        "godot-cpp/include",
        "godot-cpp/gen/include",
        "godot-cpp/gdextension",
    ]
)
kcp_object = env.SharedObject(target="bin/ikcp", source="kcp/ikcp.c")
sources = Glob("src/*.cpp") + [kcp_object]

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "bin/libgodot-kcp.{}.{}.{}.dylib".format(
            env["platform"], env["target"], env["arch"]
        ),
        source=sources,
    )
elif env["platform"] == "ios":
    if env["ios_simulator"]:
        library = env.StaticLibrary(
            "bin/libgodot-kcp.{}.{}.simulator.a".format(env["platform"], env["target"]),
            source=sources,
        )
    else:
        library = env.StaticLibrary(
            "bin/libgodot-kcp.{}.{}.a".format(env["platform"], env["target"]),
            source=sources,
        )
else:
    library = env.SharedLibrary(
        "bin/libgodot-kcp{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

env.NoCache(library)
Default(library)
