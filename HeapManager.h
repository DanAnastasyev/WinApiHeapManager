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
	// ���������� �� ���������� �������� �����, �������� roundSize
	int round( int a, int roundSize );

	// ���� ������� ��������� ���� � �������� ������� ��������
	void findAppropriateBlock( int size, int& freeBlockPtr, int& smallBlockAddr, 
		int& mediumBlockAddr, int& largeBlockAddr );

	// ������ ��������� ��������� ������
	void changeBlockStructure( int size, int& newSize, int freeBlockPtr, int smallBlockAddr, 
		int mediumBlockAddr, int largeBlockAddr );

	// ��������� ���������� �����
	void addFreeBlock( int size, int& newSize, int freeBlockPtr, int smallBlockAddr, 
		int mediumBlockAddr, int largeBlockAddr );
	
	// ����� ���������� ��������� ��������� ������� ����
	void findSmallFreeBlock( int& beginPtr, int& endPtr, int& blockSize, int& memSize );
	void findMediumFreeBlock( int& beginPtr, int& endPtr, int& blockSize, int& memSize );
	void findLargeFreeBlock( int& beginPtr, int& endPtr, int& blockSize, int& memSize );

	int* lpvReservedMem;
	// ������� ������ � ������ ��������
	int minSize;
	// ������� ������ ���������������
	int maxSize;

	// ������ �������� � ������
	int pageSize;

	// ������������ ������ ���������� �����
	int minBlockSize;
	// ������������ ������ �������� �����
	int mediumBlockSize;

	std::vector<int> pages;
	// ��������� �����
	std::vector<int> smallBlocks;
	// ���� <�����, ������> � �������� �� 4 �� 128
	std::map<int, int> mediumBlocks;
	// ���� <�����, ������> � �������� ������ 128
	std::map<int, int> largeBlocks;

	std::set<int> allocatedBlocks;
};