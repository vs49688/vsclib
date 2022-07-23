{ stdenv, cmake, version }:
stdenv.mkDerivation {
  inherit version;

  pname = "vsclib";

  src = ./.;

  nativeBuildInputs = [ cmake ];

  doCheck = true;
  checkPhase = ''
    ./tests/vsclib_tests
  '';

  installPhase = ''
    touch $out
  '';
}
