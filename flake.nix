{
  description = "vsclib";

  outputs = { self, nixpkgs, ... }: let
    mkPackages = { system }: let
      pkgs = import nixpkgs { inherit system; };
    in rec {
      vsclib  = pkgs.callPackage ./default.nix { version = self.lastModifiedDate; };
      default = vsclib;

      vsclib-clang = vsclib.override { stdenv = pkgs.clangStdenv; };
    };

    mkCross = { crossSystem }: let
      crossPackages = import nixpkgs {
        inherit crossSystem;
        system = "x86_64-linux";
      };
    in crossPackages.callPackage ./default.nix { version = self.lastModifiedDate; };

    mkShells = packages: builtins.mapAttrs (k: v: v.overrideAttrs(old: {
      hardeningDisable = [ "all" ];
      nativeBuildInputs = old.nativeBuildInputs ++ v.devTools;
    })) packages;

  in {
    packages.x86_64-linux = mkPackages { system = "x86_64-linux"; } // {
      vsclib-musl    = mkCross { crossSystem = nixpkgs.lib.systems.examples.musl64;   };
      vsclib-win32   = mkCross { crossSystem = nixpkgs.lib.systems.examples.mingw32;  };
      vsclib-win64   = mkCross { crossSystem = nixpkgs.lib.systems.examples.mingwW64; };
      vsclib-aarch64 = mkCross { crossSystem = nixpkgs.lib.systems.examples.aarch64-multiplatform; };
    };

    packages.aarch64-linux  = mkPackages { system = "aarch64-linux";  };
    packages.x86_64-darwin  = mkPackages { system = "x86_64-darwin";  };
    packages.aarch64-darwin = mkPackages { system = "aarch64-darwin"; };

    devShells.x86_64-linux   = mkShells self.packages.x86_64-linux;
    devShells.aarch64-linux  = mkShells self.packages.aarch64-linux;
    devShells.x86_64-darwin  = mkShells self.packages.x86_64-darwin;
    devShells.aarch64-darwin = mkShells self.packages.x86_64-darwin;
  };
}
