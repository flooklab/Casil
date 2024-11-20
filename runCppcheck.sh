#!/bin/bash

cppcheck --check-level=exhaustive --enable=all --std=c++20 --inline-suppr --suppress=missingIncludeSystem --suppress=unusedFunction -I ./ ./casil
cppcheck --check-level=exhaustive --enable=all --std=c++20 --inline-suppr --suppress=missingIncludeSystem --suppress=unusedFunction -I ./ ./pycasil
cppcheck --check-level=exhaustive --enable=all --std=c++20 --inline-suppr --suppress=missingIncludeSystem --suppress=unusedFunction -I ./ ./example/casil
cppcheck --check-level=exhaustive --enable=all --std=c++20 --inline-suppr --suppress=missingIncludeSystem --suppress=unusedFunction --suppress=ctuOneDefinitionRuleViolation --library=boost -I ./ ./tests
