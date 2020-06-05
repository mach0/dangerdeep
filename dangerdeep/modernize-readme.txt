Steps to adjust existing, working code base to code modernization state.
===================================================================================================
After each step check functionality.

1. Migrate SCons -> CMake, build all executables
	DONE, later split to finer libraries
2. Use clang to modernize code automatically
	DONE for modernizer checks (more error checks may be sensible)
3. Use C++11 thread/mutex/condition_variable
	DONE
4. Compare basic classes between branches for differences!
	Like angle,vector etc., but we need to rerun the automatic code
	modernization again after copying classes!
	use meld with directories to compare.
	Some classes are also obsolete - logbook.cpp
	xml difficut
	angle,vector,...?
	Note that old code constructs like typedef are reintroduced...
4. Introduce newer system input handling (needs SDL2?)
5. make use of new C++ features for widgets etc.
6. use new C++ features for XML reader etc.
7. add new sensors
8. update internal game classes
8a. Copy new classes like gpu interface to master branch so they can be
	tested and used by standalone apps
8b. Divide code into separate libraries better
9a. Migrate to SDL2
9. Finally adjust rendering
10. or earlier: get rid of all the configuration options, we use SSE and all modern stuff automatically!
11. Turn on -Wall


Notes
===================================================================================================

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
    modernize-return-braced-init-list
    modernize-pass-by-value
    modernize-loop-convert
    llvm-include-order
    modernize-use-auto
DO NOT USE    
    modernize-use-trailing-return-type
CHECK
    There is a long list of clang tidy checks that could also be applied!
TRIED BUT NO EFFECT
	performance-move-const-arg
	performance-unnecessary-copy-initialization
CALL
	./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-use-override,modernize-replace-auto-ptr,modernize-avoid-bind,modernize-concat-nested-namespaces,modernize-deprecated-headers,modernize-deprecated-ios-base-aliases,modernize-replace-random-shuffle,modernize-raw-string-literal,modernize-use-transparent-functors,modernize-use-uncaught-exceptions,modernize-make-shared,modernize-redundant-void-arg,modernize-use-bool-literals,modernize-shrink-to-fit,modernize-unary-static-assert,modernize-use-nodiscard,modernize-use-noexcept,modernize-avoid-c-arrays,modernize-use-emplace,modernize-use-equals-default,modernize-use-equals-delete,modernize-use-using,modernize-make-unique,modernize-use-nullptr,modernize-use-default-member-init,modernize-return-braced-init-list,modernize-pass-by-value,modernize-loop-convert,llvm-include-order,modernize-use-auto' -fix

FILES TAKEN FROM CODEMODERNIZATION BRANCH
