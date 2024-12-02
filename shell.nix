let
  pkgs = import <nixpkgs> {};
in pkgs.mkShell {
  packages = with pkgs; [
      cmake
      clang-tools
      gdb
      libcxx
      glfw
      glew
      libGL
      stdenv.cc.cc.lib
      imgui
      glm
      ninja
  ];

  shellHook = ''
    # Debugger path
    export GDB_PATH="${pkgs.gdb}/bin/gdb"

    export IMGUI_PATH="${pkgs.imgui}/include/imgui/"

    # System headers
    export CPLUS_INCLUDE_PATH="${pkgs.imgui}/include/imgui/:${pkgs.libcxx}/include/c++/v1:$CPLUS_INCLUDE_PATH"
    export CPATH="${pkgs.gcc.libc}/include:${pkgs.libcxx}/include/c++/v1:$CPATH"

    # Compiler resource dir (for system headers)
    export CLANG_RESOURCE_DIR="$(clang -print-resource-dir)"

    # Add OpenGL libs to path
    export LD_LIBRARY_PATH="${pkgs.libGL}/lib:$LD_LIBRARY_PATH"
  '';

  nativeBuildInputs = with pkgs; [
    pkg-config
    libcxx.dev
  ];
}
