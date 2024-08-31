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
    systems = ["x86_64-linux" "aarch64-linux"];
  in
    flake-utils.lib.eachSystem systems (system: let
      pkgs = import nixpkgs {
        inherit system;
        overlays = [
          (self: super: {
            opencl-clhpp = super.opencl-clhpp.overrideAttrs (old: rec {
              version = "2023.02.06";
              src = self.fetchFromGitHub {
                owner = "KhronosGroup";
                repo = "OpenCL-CLHPP";
                rev = "v${version}";
                sha256 = "sha256-QOZnPc0WAdI1ZSH8rVErEkWDLxoPdzGqfwqPLa8qe9Q=";
              };
            });
          })
        ];
      };

      clang-tools = pkgs.clang-tools_17;

      devShell =
        (pkgs.mkShell.override {
          stdenv = pkgs.llvmPackages_17.stdenv;
        }) {
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
            opencl-info
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
    });
}
