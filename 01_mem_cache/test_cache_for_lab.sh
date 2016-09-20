#!/bin/bash

sh test_cache.sh 10 1000 1 1100 2>log_attempts_10_from_1000_to_1100_step_1.txt
sh test_cache.sh 10 1000 10 2000 2>log_attempts_10_from_1000_to_2000_step_10.txt
