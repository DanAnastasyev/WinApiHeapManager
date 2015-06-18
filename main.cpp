#include "HeapManager.h"
#include <iostream>
#include <ctime>
#include <cstdlib>

CHeapManager heapManager;

class A {
public:
	A() {}
	~A() {}
	void* operator new(size_t size) {
		return heapManager.Alloc( size );
	}
	void* operator new[](size_t size) {
		return heapManager.Alloc( size );
	}
	void operator delete(void* p) {
		heapManager.Free( p );
	}
	void operator delete[](void *p) {
		heapManager.Free( p );
	}

	int a;
};

void test1() {
	std::cout << "Allocate big block new A[5000000]" << std::endl;
	clock_t start = clock();
	A* a = new A;
	a->a = 4;
	A* b = new A[5000000];
	for( int i = 0; i < 50000; ++i ) {
		b[i].a = i;
	}
	delete a;
	a = new A;
	a->a = 4;
	delete[] b;

	delete a;

	clock_t finish = clock();
	auto duration1 = finish - start;
	start = clock();
	int* c = new int;
	*c = 4;
	int* d = new int[5000000];
	for( int i = 0; i < 50000; ++i ) {
		d[i] = i;
	}
	delete c;
	delete[] d;
	finish = clock();
	auto duration2 = finish - start;
	std::cout << "HeapManager: " << duration1 << std::endl << "Standart new/delete: "  << duration2 << std::endl;
}

void test2() {
	std::cout << "Allocate 10000 random blocks" << std::endl;
	int n = 10000;
	std::vector<A*> myPointers(n);
	std::vector<int*> pointers(n);
	clock_t start;
	clock_t finish;
	srand(0);
	start = clock();
	for( int i = 0; i < n; ++i ) {
		myPointers[i] = new A[rand() % 2048];
	}
	finish = clock();

	auto duration1 = finish - start;

	for( int i = 0; i < n; ++i ) {
		delete[] myPointers[i];
	}

	srand(0);
	start = clock();
	for( int i = 0; i < n; ++i ) {
		pointers[i] = new int[rand( ) % 2048];
	}
	finish = clock();
	auto duration2 = finish - start;

	for( int i = 0; i < n; ++i ) {
		delete[] pointers[i];
	}

	std::cout << "HeapManager: " << duration1 << std::endl << "Standart new/delete: "  << duration2 << std::endl;
}

int main() {
	heapManager.Create( 20000, 100000000 );

	test1();
	test2();

	//int* a = (int*) heapManager.Alloc( 5000 );
	//for( int i = 0; i < 5000 / 4 + 1; ++i ) {
	//	a[i] = 5;
	//}
	//heapManager.Free( a );
	//int* b = (int*) heapManager.Alloc( 4 );
	//heapManager.Destroy();
	//for( int i = 0; i < 5000 / 4 + 1; ++i ) {
	//	a[i] = 5;
	//}

	/*A* a = new A;
	a->a = 4;
	std::cout << a->a;
	A* b = new A[5000];
	for( int i = 0; i < 5000; ++i ) {
		b[i].a = i;
	}
	delete a;*/

	heapManager.Destroy();
	return 0;
}