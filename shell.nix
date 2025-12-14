{ pkgs ? import <nixpkgs> { } }:

pkgs.mkShell {
  name = "telemetry-dev-env";

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

    lua
    sol2

    which
    aha
    # openssl
  ];


  # Shell-specific settings
  shellHook = ''
    clip() {
      if [ "$XDG_SESSION_TYPE" = "wayland" ]; then
        cat "$@" | wl-copy
      elif [ "$XDG_SESSION_TYPE" = "x11" ]; then
        xclip -selection clipboard "$@"
      else
        echo "Not running on a recognized X11 or Wayland session." >&2
      fi
    }
    get_commit_message() {
      git_diff=$(git diff main|aha)
      echo "I need a commit message, no formatting. just plain, alphanumeric text with proper punctuation" > ./diff.prompt
      echo "\`\`\`$git_diff\`\`\`" >> ./diff.prompt
      clip ./diff.prompt
    }
    nix_hash () {
      local rev=$(git rev-parse HEAD)
      local sha256=$(nix-prefetch-url --unpack https://github.com/jpoage1/telemetry/archive/$(git rev-parse HEAD).tar.gz)
      echo "rev $rev"
      echo "sha256 $sha256"
    }

    alias build-target="cmake --build build --target"
    alias install-component="cmake --install ./build/ --component"

    alias waybard="time ./build/waybard ~/.config/conky/file-systems.txt"
    alias json="time ./build/json ~/.config/conky/file-systems.txt"
    alias lua-config="time ./build/json --config ~/.config/conky/config.lua"
    alias lua-settings="time ./build/json --settings ~/.config/conky/settings.lua"
    alias clean="cmake --build build --target clean"

    alias get-diff="git diff main|aha > diff.html && clip.sh diff.html"

    echo "Entering a Nix development shell for conkyd..."
    echo "C++ compiler available: $(which g++)"
    echo "Build system available: $(which cmake)"
    echo "pkg-config available: $(which pkg-config)"
    echo "libssh headers and library are now available for your project."

    unset NIX_ENFORCE_NO_NATIVE
  '';
}
