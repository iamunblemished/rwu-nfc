/*
 * Wrapper file to build write_example.cpp from examples directory
 * This allows building the example without overwriting main.cpp
 */

#ifdef BUILD_EXAMPLE_WRITE
#include "../examples/write_example.cpp"
#endif
