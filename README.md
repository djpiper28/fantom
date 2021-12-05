# F@ntom
F@ntom is a FOSS service that let's you control your systemd from your phone, web
browser or, from the pc application.

**F@ntom is currently under development and is expected to be finished in a long
time.**

## Project Structure
| Folder | Content |
|---|---|
| fantom-server | this is the fantom server source code. |
| ... | ... |

This will be updated when development on the app, browser interface and, pc
application starts.

## fantom-server/
### Building
```bash
# cd fantom-server/
mkdir -p build
cmake .. && cmake --build . -j
```

### Testing
```bash
# cd fantom-server/
mkdir -p build
cmake .. -DCMAKE_BUILD_TYPE=TEST && cmake --build . -j && make coverage
# You can also you ctest
# But ctest is less verbose with the output
```

