cmake_minimum_required(VERSION 3.20)
project(chain LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
add_executable(chain 
    main.cpp
    tridiagonals_solver.cpp    
)
