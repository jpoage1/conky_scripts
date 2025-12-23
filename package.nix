{ lib
, stdenv
, fetchFromGitHub
, cmake
, pkg-config
, gtkmm4
, libssh
, lua
, nlohmann_json
, spdlog
, sol2
}:

stdenv.mkDerivation rec {
  pname = "telemetry";
  version = "1.0.0";

  # OPTION A: Fetch from GitHub (Cleanest, Reproducible)
  # You need to fill in the 'rev' (commit hash) and 'sha256'.
  # Run 'nix-prefetch-url --unpack https://github.com/jpoage1/telemetry/archive/main.tar.gz' to get the SHA.
  src = fetchFromGitHub {
    owner = "jpoage1";
    repo = "telemetry";
    rev = "main";
    sha256 = "0000000000000000000000000000000000000000000000000000"; # REPLACE THIS
  };

  # OPTION B: Use local directory (If you are actively editing telemetry code)
  # Comment out Option A and uncomment this if you cloned it manually into ./telemetry
  # src = ./telemetry;

  nativeBuildInputs = [
    cmake
    pkg-config
  ];

  buildInputs = [
    gtkmm4
    libssh
    lua
    nlohmann_json
    spdlog
    sol2
  ];

  # The CMakeLists.txt defines the 'json' target we need
  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
  ];

  # This ensures the 'json' binary is installed to $out/bin/json
  # Your CMakeLists.txt already has: install(TARGETS json ... DESTINATION bin)
  doInstall = true;
}
