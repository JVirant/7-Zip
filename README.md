The whole purpose of this git is just to create some plugins to add new archive support in 7-Zip.

# Plugins
## Build requirements
I'm not sure of everything needed to build 7zip sources. For now, I identified:
- Windows: Visual Studio 2022 with C++ support and nmake
- Linux: gcc, g++ and gnu make
- MacOS: seems fine with Xcode installed

You can check the DOC/readme.txt file for more informations.

## MPAK (Dark Age of Camelot packages: mpk, npk)
### Windows
You can build the dll in `CPP/7zip/Archive/Mpk`, just run nmake from a Visual Studio command prompt.
### Linux
You can build the shared libraby in `CPP/7zip/Archive/Mpk` with `make -f makefile.gcc -j`
### MacOS (arm64)
You can build the shared libraby in `CPP/7zip/Archive/Mpk` with `make -f ../../compl_mac_arm64.mak -j`
### MacOS (x64)
You can build the shared libraby in `CPP/7zip/Archive/Mpk` with `make -f ../../compl_mac_x64.mak -j`

# Thanks

It's based on git source of https://github.com/mcmilk/7-Zip which itself is based on work of https://sourceforge.net/p/sevenzip.

Thanks everyone for 7-Zip!

## See Also

[7-Zip Homepage](https://www.7-zip.org/) \
[7-Zip @ sf.net](https://sourceforge.net/p/sevenzip/) \
[7-Zip mcmilk's GitHub](https://github.com/mcmilk/7-Zip)
