```sh
$ mkdir build && cd build
$ cmake -G Ninja \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_INSTALL_PREFIX=$HOME/falkon \
    -DNO_X11=true -DENABLE_KDE_FRAMEWORKS_INTEGRATION_PLUGIN=false ..
$ make
$ make install
```
