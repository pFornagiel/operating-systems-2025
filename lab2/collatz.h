#pragma once
#ifndef COLLATZ_H
#define COLLATZ_H

int collatz_conjecture(int input);
int test_collatz_convergence(int input, int max_iter, int *steps);

#endif