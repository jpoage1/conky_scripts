{ pkgs ? import <nixpkgs> { } }:
let
  telemetry-pkg = pkgs.callPackage ./package.nix { };
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
  libs = pkgs.lib.makeLibraryPath telemetry-pkg.buildInputs;
in
pkgs.mkShell {
  name = "telemetry-dev-env";

  inputsFrom = [ telemetry-pkg ];

  nativeBuildInputs = with pkgs;[
    # Compiler
    gcc
    gnumake
    valgrind
    clang-tools
    python3
    ninja
    mold

    # Build system
    cmake
    pkg-config
    qt6.wrapQtAppsHook
    makeWrapper
    which
    aha
    dev-help
    patchelf
  ];

  # Shell-specific settings
  shellHook = ''
    export NIX_LDFLAGS="-rpath ${libs} $NIX_LDFLAGS"

    # Set up the runtime environment for the current session
    export LD_LIBRARY_PATH="${libs}:$LD_LIBRARY_PATH"

    # Set the Qt Plugin Path (needed for platform integration, e.g. xcb/wayland)
    export QT_PLUGIN_PATH="${telemetry-pkg.qt6.qtbase}/${telemetry-pkg.qt6.qtbase.qtPluginPrefix}"

    # Set the QML Import Path (needed for QtQuick.Controls, etc.)
    export QML2_IMPORT_PATH="${telemetry-pkg.qt6.qtdeclarative}/${telemetry-pkg.qt6.qtbase.qtQmlPrefix}"

    export CMAKE_PREFIX_PATH="${pkgs.lib.makeSearchPath "lib/cmake" telemetry-pkg.buildInputs}:$CMAKE_PREFIX_PATH"
    export PKG_CONFIG_PATH="${pkgs.lib.makeSearchPath "lib/pkgconfig" telemetry-pkg.buildInputs}:$PKG_CONFIG_PATH"

    clip() {
      if [ "$XDG_SESSION_TYPE" = "wayland" ]; then
        cat "$@" | wl-copy
      elif [ "$XDG_SESSION_TYPE" = "x11" ]; then
        xclip -selection clipboard "$@"
      else
        echo "Not running on a recognized X11 or Wayland session." >&2
      fi
    }
    commit_message() {
      local branch=''\${1:-main}
      git_diff=$(git diff --cached)
      if [ -z "$git_diff" ]; then
        echo "Error: No staged changes found. Did you 'git add'?"
        return 1
      fi
      msg="Analyze the provided diff. Identify the functional logic delta. Ignore all previous conversational history and prior commit messages. Describe the atomic changes in the added and removed lines. Output only the alphanumeric message text. No headers, no markdown, no filler. Terminate immediately after delivery."
      # msg="I need a commit message, no formatting. just plain, alphanumeric text with proper punctuation. Do not ask follow up questions. Please do not repeat previous commit messages."
      prompt=("''\$msg" "\`\`\`''\$git_diff\`\`\`")
      printf "%s\n" "''\${prompt[@]}" | clip
    }
    nix_hash () {
      local rev=$(git rev-parse HEAD)
      local sha256=$(nix-prefetch-url --unpack https://github.com/jpoage1/telemetry/archive/$(git rev-parse HEAD).tar.gz)
      echo "rev $rev"
      echo "sha256 $sha256"
    }

    alias build="cmake --build build"
    alias build-target="cmake --build build --target"
    alias build-telemetry="cmake --build build --target telemetry"
    alias install-component="cmake --install ./build/ --component"
    alias install="cmake --install build --prefix $HOME/.config/telemetry"
    alias install="cmake --install build --prefix $HOME/.config/telemetry/bin"

    alias waybard="time ./build/waybard ~/.config/telemetry/file-systems.txt"
    alias json="time ./build/json ~/.config/telemetry/file-systems.txt"
    alias lua-config="time ./build/json --config ~/.config/telemetry/config.lua"
    alias lua-settings="time ./build/json --settings ~/.config/telemetry/settings.lua"
    alias clean="cmake --build build --target clean"

    alias get-diff="git diff main|aha > diff.html && clip.sh diff.html"

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
