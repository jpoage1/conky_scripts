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
    gtkmm4
    pkg-config
    # openssl
  ];


  # Shell-specific settings
  shellHook = ''
    alias waybard="./waybard.bin ~/.config/conky/file-systems.txt"
    alias json="./json.bin ~/.config/conky/file-systems.txt"
    echo "Entering a Nix development shell for conkyd..."
    echo "C++ compiler available: $(which g++)"
    echo "Build system available: $(which cmake)"
    echo "pkg-config available: $(which pkg-config)"
    echo "libssh headers and library are now available for your project."
  '';
}
