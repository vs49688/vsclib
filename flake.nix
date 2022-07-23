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
    packages.x86_64-linux   = mkPackages { system = "x86_64-linux";   };
    packages.aarch64-linux  = mkPackages { system = "aarch64-linux";  };
    packages.x86_64-darwin  = mkPackages { system = "x86_64-darwin";  };
    packages.aarch64-darwin = mkPackages { system = "aarch64-darwin"; };
  };
}
