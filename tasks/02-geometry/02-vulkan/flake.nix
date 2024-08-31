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
        overlays = [];
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
            range-v3
            fmt
            glm
            spdlog
            glfw3
            vulkan-headers
            vulkan-validation-layers
            vulkan-loader
            vulkan-tools
            shaderc
          ];
        };
    in {
      devShells = {
        default = devShell;
      };

      formatter = pkgs.alejandra;
    });
}
