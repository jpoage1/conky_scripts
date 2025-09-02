{ pkgs ? import <nixpkgs> { } }:

pkgs.mkShell {
  name = "conkyd-dev-env";

  # C++ Toolchain and Build System
  buildInputs = with pkgs; [
    # Compiler
    gcc

    # Build system
    cmake
    libssh
    libssh2
    nlohmann_json
    # openssl
  ];


  # Shell-specific settings
  shellHook = ''
    echo "Entering a Nix development shell for conkyd..."
    echo "C++ compiler available: $(which g++)"
    echo "Build system available: $(which cmake)"
    echo "libssh headers and library are now available for your project."
  '';
}
