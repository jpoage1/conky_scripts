{ pkgs ? import <nixpkgs> { } }:
let
  project = pkgs.callPackage ./default.nix { };
  telemetry-pkg = project.telemetry;
  ws-bridge = project.ws_bridge;
  help-command = "telemetry-help";
  dev-help = pkgs.writeShellScriptBin help-command ''
    # Using ANSI escape codes directly for maximum compatibility
    G="\033[1;32m"
    Y="\033[1;33m"
    B="\033[1;34m"
    BOLD="\033[1m"
    R="\033[0m"

    echo -e "''${G}============================================================''${R}"
    echo -e "''${BOLD} TELEMETRY DEVELOPMENT HELP ''${R}"
    echo -e "''${G}============================================================''${R}"
    echo ""
    echo -e "''${Y}[ BUILD COMMANDS ]''${R}"
    echo -e " ''${BOLD}build''${R} : Builds all targets"
    echo -e " ''${BOLD}build-telemetry''${R} : Builds the telemetry target"
    echo -e " ''${BOLD}build-target <T>''${R} : Build specific CMake target (e.g. rootwars)"
    echo -e " ''${BOLD}clean''${R} : Runs cmake --build clean"
    echo ""
    echo -e "''${Y}[ RUNTIME TESTS ]''${R}"
    echo -e " ''${BOLD}lua-config''${R} : Run using standard telemetry lua config"
    echo -e " ''${BOLD}lua-settings''${R} : Run using settings-specific lua path"
    echo -e " ''${BOLD}waybard''${R} : Launch the Waybar-compatible data stream"
    echo -e " ''${BOLD}json''${R} : Raw JSON metrics dump for file systems"
    echo ""
    echo -e "''${Y}[ GIT & HASHING ]''${R}"
    echo -e " ''${BOLD}get_commit_message''${R} : HTML diff to clipboard for LLM"
    echo -e " ''${BOLD}nix_hash''${R} : Get SHA256 of current HEAD"
    echo -e " ''${BOLD}get-diff''${R} : Generate and clip HTML diff"
    echo ""
    echo -e "''${Y}[ DIRECTORIES ]''${R}"
    echo -e " ''${BOLD}Config Path''${R} : ~/.config/telemetry/"
    echo -e " ''${BOLD}Build Path''${R} : ./build/"
    echo ""
    echo -e "''${G}============================================================''${R}"
    echo -e "''${DIM}Type ''${R}''${BOLD}${help-command}''${R}''${DIM} at any time to see this message again.''${R}"
  '';
  libs = pkgs.lib.makeLibraryPath telemetry-pkg.buildInputs ++ pkgs.lib.makeLibraryPath ws-bridge.buildInputs;
in
pkgs.mkShell {
  name = "telemetry-dev-env";

  inputsFrom = [
    telemetry-pkg
    ws-bridge
  ];

  # C++ Toolchain and Build System
  nativeBuildInputs = with pkgs; [
    dev-help
    yazi
    tmux
  ];


  # Shell-specific settings
  shellHook = ''
    clean_qt_cache() {
      rm -rf ~/.cache/qmlcache/
      rm -rf ~/.cache/*telemetry*
      rm -rf ~/.cache/*widgets*
    }
    build() {
      local cwd=$(pwd)
      mkdir -p "$BUILD_DIR"
      cd "$BUILD_DIR"
      clean_qt_cache
      cmake ..
      cmake --build "$BUILD_DIR"
      cd "$cwd"
    }

    PROJECT_DIR=$(pwd)

    PATH="$PROJECT_DIR/bin:$PATH"
    export BUILD_DIR="$PROJECT_DIR/build"
    
    alias build-target="clean_qt_cache; cmake --build \"$BUILD_DIR\" --target"
    alias install-component="cmake --install \"$BUILD_DIR\" --component"
    alias install="cmake --install \"$BUILD_DIR\""

    alias waybard="time \"$BUILD_DIR/telemetry\" ~/.config/telemetry/filesystems.txt"
    alias json="time \"$BUILD_DIR/telemetry\" ~/.config/telemetry/filesystems.txt"
    alias widgets="\"$BUILD_DIR/telemetry\" --config ~/.config/telemetry/widgets.lua"
    alias lua-config="time \"$BUILD_DIR/telemetry\" --config ~/.config/telemetry/config.lua"
    alias lua-settings="time \"$BUILD_DIR/telemetry\" --settings ~/.config/telemetry/settings.lua"
    alias clean="clean_qt_cache; cmake --build \"$BUILD_DIR\" --target clean"

    echo "Entering a Nix development shell for telemetry..."
    echo "C++ compiler available: $(which g++)"
    echo "Build system available: $(which cmake)"
    echo "pkg-config available: $(which pkg-config)"
    echo "libssh headers and library are now available for your project."

    unset NIX_ENFORCE_NO_NATIVE
    ${help-command}
    PS1="$PS1 telemetry shell) "
  '';
}
