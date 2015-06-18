#include <Windows.h>
#include <vector>
#include <map>
#include <set>
#include <iostream>

class CHeapManager {
public:
	CHeapManager(): minSize(0), maxSize(0) {}

	void CHeapManager::Create(int minSize, int maxSize);

	void* CHeapManager::Alloc(int size);

	void CHeapManager::Free(void* mem);

	void CHeapManager::Destroy();

private:
	// Округление до ближайшего большего числа, кратного roundSize
	int round( int a, int roundSize );

	// Ищет больший свободный блок с наиболее близким размером
	void findAppropriateBlock( int size, int& freeBlockPtr, int& smallBlockAddr, 
		int& mediumBlockAddr, int& largeBlockAddr );

	// Меняем структуру свободных блоков
	void changeBlockStructure( int size, int& newSize, int freeBlockPtr, int smallBlockAddr, 
		int mediumBlockAddr, int largeBlockAddr );

	// Добавляем отрезанный кусок
	void addFreeBlock( int size, int& newSize, int freeBlockPtr, int smallBlockAddr, 
		int mediumBlockAddr, int largeBlockAddr );
	
	// Найти подходящий маленький свободный смежный блок
	void findSmallFreeBlock( int& beginPtr, int& endPtr, int& blockSize, int& memSize );
	void findMediumFreeBlock( int& beginPtr, int& endPtr, int& blockSize, int& memSize );
	void findLargeFreeBlock( int& beginPtr, int& endPtr, int& blockSize, int& memSize );

	int* lpvReservedMem;
	// сколько памяти в начале выделено
	int minSize;
	// сколько памяти зарезервировано
	int maxSize;

	// размер страницы в памяти
	int pageSize;

	// Максимальный размер маленького блока
	int minBlockSize;
	// Максимальный размер среднего блока
	int mediumBlockSize;

	std::vector<int> pages;
	// свободные блоки
	std::vector<int> smallBlocks;
	// пары <адрес, размер> с размером от 4 до 128
	std::map<int, int> mediumBlocks;
	// пары <адрес, размер> с размером больше 128
	std::map<int, int> largeBlocks;

	std::set<int> allocatedBlocks;
};