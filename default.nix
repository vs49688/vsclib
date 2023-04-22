{ stdenv, cmake, version, clang-tools }:
stdenv.mkDerivation {
  inherit version;

  pname = "vsclib";

  src = ./.;

  nativeBuildInputs = [ cmake ];

  passthru.devTools = [
    clang-tools
  ];

  doCheck = true;
  checkPhase = ''
    ./tests/vsclib_tests
  '';

  installPhase = ''
    touch $out
  '';
}
