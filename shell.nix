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
    spdlog
    # openssl
  ];


  # Shell-specific settings
  shellHook = ''
    alias waybard="time ./waybard.bin ~/.config/conky/file-systems.txt"
    alias json="time ./json.bin ~/.config/conky/file-systems.txt"
    echo "Entering a Nix development shell for conkyd..."
    echo "C++ compiler available: $(which g++)"
    echo "Build system available: $(which cmake)"
    echo "pkg-config available: $(which pkg-config)"
    echo "libssh headers and library are now available for your project."
    unset NIX_ENFORCE_NO_NATIVE
  '';
}
