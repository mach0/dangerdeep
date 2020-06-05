Steps to adjust existing, working code base to code modernization state.
After each step check functionality.
1. Migrate SCons -> CMake, build all executables
	DONE, later split to finer libraries
2. Use clang to modernize code automatically
3. Migrate to SDL2
3b. Use C++11 threads etc.
4. Introduce newer system input handling
5. make use of new C++ features for widgets etc.
6. use new C++ features for XML reader etc.
7. add new sensors
8. update internal game classes
9. Finally adjust rendering
10. or earlier: get rid of all the configuration options, we use SSE and all modern stuff automatically!
11. Turn on -Wall

clang tidy modernizers
DONE
    modernize-use-override
    modernize-replace-auto-ptr
    modernize-avoid-bind
    modernize-concat-nested-namespaces
    modernize-deprecated-headers
    modernize-deprecated-ios-base-aliases
    modernize-replace-random-shuffle
    modernize-raw-string-literal
    modernize-use-transparent-functors
    modernize-use-uncaught-exceptions
    modernize-make-shared
    modernize-redundant-void-arg
    modernize-use-bool-literals
    modernize-shrink-to-fit
    modernize-unary-static-assert
    modernize-use-nodiscard
    modernize-use-noexcept
    modernize-avoid-c-arrays
    modernize-use-emplace
    modernize-use-equals-default
    modernize-use-equals-delete
    modernize-use-using
    modernize-make-unique
    modernize-use-nullptr
    modernize-use-default-member-init
TODO
    modernize-return-braced-init-list
    modernize-pass-by-value
    modernize-loop-convert
    modernize-use-auto
DO NOT USE    
    modernize-use-trailing-return-type

