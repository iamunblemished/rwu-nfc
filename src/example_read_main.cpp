/*
 * Wrapper file to build read_example.cpp from examples directory
 * This allows building the example without overwriting main.cpp
 */

#ifdef BUILD_EXAMPLE_READ
#include "../examples/read_example.cpp"
#endif
