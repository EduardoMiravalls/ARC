ARC: Automatic Reference Counting Hash Table library.

===

I started to develop this project to try to overcome a problem I had on a class assignment.

We were developing an IRC threaded server, and all it's data was globally shared among all threads. Problem was, that when deleting one of those objects, even it wasn't accessible anymore, other threads might still keep a reference to it (for example, if they're sleeping in a mutex) and they would trigger a SIGSEGV if they try to access it. And as Linus said, "Remember: if another thread can find your data structure, and you don't have a reference count on it, you almost certainly have a bug.".
So I starting coding this to try to solve that problem, but I didn't finish it on time for the assignment, so we documented the possible bug of the "disappearing structure" and handed in.

So now in my spare time I've decided to have a second look on the project and try and finish it.

Also, I've used this project to experiment a little with doxygen.


TODO:
Add some tests or examples for RCHashTable.


Usage:
* Nothing fancy, just include RCHashTable.h or RCHashTable_sync.h, create a Reference Counting Hash Table with the _new() functions, and you can start using them!

* To acquire a reference to an object use the _refinc method. To release it, use refdec.

* To limit an object's future refincs(), use the _delete function. The object will be freed once it's reference count reaches 0.

* Once you're done with the table, use the _free function.

NOTE:
The synchronized version is implemented using coarse grained locking, so it may be a little overkill. Still thinking if there's a better method without rewriting the hole thing. If you think my approach is wrong, please send me an email! After all, I'm still learning and I'd love to get some feedback.

Keep in mind that I'm not locking the object itself, only while I'm manipulating the hash table and it's reference count, so you'll have to use additional synchronization mechanisms when you're manipulating the pointer _refinc() returned.


If you discover any bugs, have any questions or suggestions, feel free to email me at edu.miravalls@hotmail.es.
