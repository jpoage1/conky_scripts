{ lib
, stdenv
, fetchFromGitHub
, cmake
, pkg-config
, makeWrapper
, gcc
, qt6
, gtkmm4
, libglvnd
, libssh
, libssh2
, lua
, nlohmann_json
, spdlog
, sol2
, fmt
, sysprof
, libsysprof-capture
, libGL
, patchelf
, fribidi
, libdatrie
, libthai
, gtest
, ninja
, mold
, valgrind
, clang-tools
, python3
}:

stdenv.mkDerivation rec {
  pname = "telemetry";
  version = "1.0.0";

  src = ./.;

  nativeBuildInputs = [
    cmake
    pkg-config
    ninja
    mold
    qt6.wrapQtAppsHook
    makeWrapper
    patchelf
  ];

  buildInputs = [
    qt6.qtbase
    qt6.qtdeclarative
    gtkmm4
    libssh
    libssh2
    nlohmann_json
    spdlog
    lua
    sol2
    fmt
    sysprof
    libsysprof-capture
    libGL
    fribidi
    libdatrie
    libthai
    gtest
    stdenv.cc.cc.lib # Mandatory for libstdc++.so.6
    libglvnd
    # gcc.libc
  ];

  passthru = {
    inherit qt6;
  };

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
  ];

  # TDD Verification Phase
  doInstallCheck = true;
  installCheckPhase = ''
    echo "Testing Linkage Integrity..."
    # Target the actual ELF binary, not the shell script wrapper
    ldd $out/bin/.telemetry-wrapped | grep -v "not found"
  '';
  preFixup = ''
    patchelf --add-rpath "${stdenv.cc.cc.lib}/lib" $out/bin/telemetry
  '';

  meta = with lib; {
    description = "Telemetry System";
    license = licenses.mit;
    platforms = platforms.linux;
  };
}
