How to compile

```
git clone https://github.com/Sembiance/solitaire-deal-generator
cd solitaire-deal-generator
mkdir build
cd build
cmake ..
make
```

How to run

```
cd solitaire-deal-generator/build/src
./sim --help
```

All games have default values for iterations and maximumDepth. You can override these by passing command line arguments to sim. See sim --help.

Example
```
sim --seed 1 FreeCell
```

