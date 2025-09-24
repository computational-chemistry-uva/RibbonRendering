with import <nixpkgs> {};

pkgs.mkShell {
    packages = [
    ];

    buildInputs = [
        libGL
        glfw
        glew
        glm
        #stdenv.cc.cc.lib
        #libcxx
    ];

    nativeBuildInputs = [
        #clang # This ovverrides clang-tools' clangd and causes problems for LSP
        clang-tools # For LSP
        cmake
        ninja
        #pkg-config
        gdb
    ];

    shellHook = ''
    '';
}
