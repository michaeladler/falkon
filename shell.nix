{ pkgs ? import <nixpkgs> { } }:

with pkgs;

with xorg;

with qt5;

mkShell {

  buildInputs = [
    libpthreadstubs
    qtsvg qttools qtwebengine qtwayland
    kdeFrameworks.karchive
    zigpass
  ] ++ [ ];

  nativeBuildInputs = [ cmake extra-cmake-modules pkgconfig qmake qttools ];

  enableParallelBuilding = true;

}
