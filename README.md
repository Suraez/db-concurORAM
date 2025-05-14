### A course project that tries to implement algorithms mentioned in the paper https://www.ndss-symposium.org/wp-content/uploads/2019/02/ndss2019_07B-2_Chakraborti_paper.pdf



Tree , Blocks and Path Indices start from zero.


Dummy block is returned when you don't have block added to the position map.

Obviliousness:

    Only one block is returned to the user, now one can question the obvilousness
    but keep in mind that
    all blocks of the path are read from the postionmap then added to the stash 
    then queried the stash for retrieving the client requested block

    so this is how the obviliousness is guaranteed.




__Code Explanation__

#pragma once tells the compiler to include header files only once
std::shared_ptr helps to share global variables among the threads


std::thread t2(clientQuery, 2, 3, tree, positionMap, stash, drl, qlog);

the above loc is similar to the pthread_create() which accepts an function argument that the thread should run, 

in the above example, the thread is supposed to run the clientQuery


_Tip_:
For the preview of markdown, hit the `ctrl+shift+v` in the VSCODE



__ALGORITHMS__

1. readLogSet(id) is implemented in DRLogSet.cpp