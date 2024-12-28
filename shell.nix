with import <nixpkgs> {};

pkgs.mkShell {
    packages = [
        #stdenv.cc.cc.lib
        #libcxx
        cmake
        ninja
        #clang # This ovverrides clang-tools' clangd and causes problems for LSP
        clang-tools # For LSP
        libGL
        glfw
        glew
        glm
    ];

    nativeBuildInputs = [
        #pkg-config
    ];

    shellHook = ''
    '';
}
