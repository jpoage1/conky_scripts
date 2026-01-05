{ pkgs, lib, python3 }:

pkgs.stdenv.mkDerivation {
  pname = "telemetry-ws-bridge";
  version = "1.0.0";
  src = ./bridge; # Point to your python source directory

  buildInputs = [
    (python3.withPackages (ps: with ps; [ 
      websockets 
    ]))
  ];

  installPhase = ''
    mkdir -p $out/bin
    cp __main__.py $out/bin/telemetry-bridge
    chmod +x $out/bin/telemetry-bridge
  '';
}
