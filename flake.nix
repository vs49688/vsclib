{
  description = "vsclib";

  outputs = { self, nixpkgs, ... }: let
    mkPackages = { system }: let
      pkgs = import nixpkgs { inherit system; };
    in rec {
      vsclib  = pkgs.callPackage ./default.nix { version = self.lastModifiedDate; };
      default = vsclib;
    };
  in {
    packages.x86_64-linux   = mkPackages { system = "x86_64-linux"; } // {
      vsclib-musl = let
        musl64Packages = import nixpkgs {
          system = "x86_64-linux";
          crossSystem = nixpkgs.lib.systems.examples.musl64;
        };
      in musl64Packages.callPackage ./default.nix { version = self.lastModifiedDate; };

      vsclib-win64 = let
        win64Packages = import nixpkgs {
          system = "x86_64-linux";
          crossSystem = nixpkgs.lib.systems.examples.mingwW64;
        };
      in win64Packages.callPackage ./default.nix { version = self.lastModifiedDate; };
    };
    packages.aarch64-linux  = mkPackages { system = "aarch64-linux";  };
    packages.x86_64-darwin  = mkPackages { system = "x86_64-darwin";  };
    packages.aarch64-darwin = mkPackages { system = "aarch64-darwin"; };
  };
}
