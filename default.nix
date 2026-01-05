{ pkgs ? import <nixpkgs> { } }:

{
  telemetry = pkgs.callPackage ./telemetry.nix { };
  ws_bridge = pkgs.callPackage ./ws_bridge.nix { };
}
