#pragma once

// Used by the system tests
#ifndef TEST
int main (int argc, char **argv);
#else
int fantom_main(int argc, char **argv);
#endif

