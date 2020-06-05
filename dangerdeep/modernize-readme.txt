Steps to adjust existing, working code base to code modernization state.
After each step check functionality.
1. Migrate SCons -> CMake, build all executables
	DONE, later split to finer libraries
2. Use clang to modernize code automatically
3. Migrate to SDL2
4. Introduce newer system input handling
5. make use of new C++ features for widgets etc.
6. use new C++ features for XML reader etc.
7. add new sensors
8. update internal game classes
9. Finally adjust rendering
10. or earlier: get rid of all the configuration options, we use SSE and all modern stuff automatically!
