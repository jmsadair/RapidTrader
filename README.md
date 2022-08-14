![workflow](https://github.com/jmsadair/RapidTrader/actions/workflows/linux.yml/badge.svg)
![workflow](https://github.com/jmsadair/RapidTrader/actions/workflows/macos.yml/badge.svg)
[![codecov](https://codecov.io/gh/jmsadair/RapidTrader/branch/dev/graph/badge.svg?token=4QQI0QDVYM)](https://codecov.io/gh/jmsadair/RapidTrader)

# RapidTrader

RapidTrader is a low-latency, high-throughput order matching system that is optimised for high frequency trading. 
This system is capable of processing upwards of 2-4 million order insertions per second and offers support for the following order types:
- Limit
- Market
- Stop
- Trailing Stop

## Requirements
- Linux or MacOS
- gcc or clang
- [CMake](https://cmake.org/download/)
- [boost](https://github.com/boostorg/boost)

Note that this project may be able to run on other platforms or compilers, but only the ones listed above have been tested at this point. 

## Building
To build and test the project, run the following commands: 
```
  mkdir build && cd build

  cmake ..

  make -j

  make test
```
