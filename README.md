# F@ntom
F@ntom is a FOSS service that let's you control your system from your phone, web
browser or, from the pc application. It is a lightweight tool for linux machines
that have systemd and, is perfect for things such as home servers or raspberry
pis.

**F@ntom is currently under development and is expected to be finished in a long
time.**

## Project Structure
| Folder | Build Status | Content |
|---|---|---|
| fantom-server | [![Main](https://github.com/djpiper28/fantom/actions/workflows/main.yml/badge.svg)](https://github.com/djpiper28/fantom/actions/workflows/main.yml) [![Memory tests](https://github.com/djpiper28/fantom/actions/workflows/memtests.yml/badge.svg)](https://github.com/djpiper28/fantom/actions/workflows/memtests.yml) [![codecov](https://codecov.io/gh/djpiper28/fantom/branch/main/graph/badge.svg?token=FKLI93HBRN)](https://codecov.io/gh/djpiper28/fantom) | This is the fantom node/server source code. |
| ... | ... | ... |

This will be updated when development on the app, browser interface and, pc
application starts.

## fantom-server/
### Requirements

| Requirement | Usage |
|---|---|
| `jansson` | json |
| `mbedtls` | tls for mongoose (can be swapped to openssl) |
| `openssl` | crypto |
| `pthread` | threading |
| `sqlite3` | database |

### Build Requirements

| Requirement | Usage |
|---|---|
| `cmake` | Build system |
| `gcc & g++` | Compilers |
| `valgrind` | Memory analysis |
| `python3` | Scripting |
| `bash` | Scripting |
| `astyle` | Formatter |
| `black` | Formatter |
| `cmake-format` | Formatter |
| `gcovr` | Coverage tool |
| `git` | VCS |

### Building
```bash
# cd fantom-server/
mkdir -p build
cmake .. && cmake --build . -j
```

This contains a copy of [mongoose](https://github.com/cesanta/mongoose) just in-case
it goes anywhere.

### Testing
```bash
# cd fantom-server/
mkdir -p build
cmake .. -DCMAKE_BUILD_TYPE=TEST && cmake --build . -j && ctest -V
# You can also you ctest
# But ctest is less verbose with the output to console
```
