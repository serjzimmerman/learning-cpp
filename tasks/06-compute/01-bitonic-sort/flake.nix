{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    ...
  } @ inputs: let
    systems = [
      "x86_64-linux"
      "aarch64-linux"
    ];
  in
    flake-utils.lib.eachSystem systems (
      system: let
        pkgs = import nixpkgs {
          inherit system;
        };

        clang-tools = pkgs.clang-tools_17;

        devShell =
          (pkgs.mkShell.override {
            stdenv = pkgs.llvmPackages_17.stdenv;
          })
          {
            nativeBuildInputs = with pkgs; [
              cmake
              python311
              ninja
              clang-tools
              gtest
              boost
              flex
              bison
              just
              range-v3
              fmt
              llvmPackages_17.openmp
              opencl-clhpp
              clinfo
              opencl-headers
              ocl-icd
              eigen
            ];
          };
      in {
        devShells = {
          default = devShell;
        };

        formatter = pkgs.alejandra;
      }
    );
}
