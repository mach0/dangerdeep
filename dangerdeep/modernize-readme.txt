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

clang tidy modernizers
DONE
    modernize-use-override
    modernize-replace-auto-ptr
TODO
    modernize-avoid-bind
    modernize-avoid-c-arrays
    modernize-concat-nested-namespaces
    modernize-deprecated-headers
    modernize-deprecated-ios-base-aliases
    modernize-loop-convert
    modernize-make-shared
    modernize-make-unique
    modernize-pass-by-value
    modernize-raw-string-literal
    modernize-redundant-void-arg
    modernize-replace-random-shuffle
    modernize-return-braced-init-list
    modernize-shrink-to-fit
    modernize-unary-static-assert
    modernize-use-auto
    modernize-use-bool-literals
    modernize-use-default-member-init
    modernize-use-emplace
    modernize-use-equals-default
    modernize-use-equals-delete
    modernize-use-nodiscard
    modernize-use-noexcept
    modernize-use-nullptr
    modernize-use-trailing-return-type
    modernize-use-transparent-functors
    modernize-use-uncaught-exceptions
    modernize-use-using
