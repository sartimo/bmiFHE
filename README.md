# bmiFHE
A simple C++ CLI to calculate the BMI using FHE and custom user inputs

## dependencies

```bmiFHE``` requires @microsoft/SEAL to be installed globally on your system.

## build

To build ```bmiFHE``` on Debian, execute the following:

```bash
mkdir -p build
cd build
cmake ..
make -j
```

## using the ```bmiFHE```

To use this application, simply call the binary and provide the required values. It will then compute the BMI according to your values securely using FHE in an encrypted ciphertext.
