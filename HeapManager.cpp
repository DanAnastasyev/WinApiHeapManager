#include "HeapManager.h"
#include <exception>


void CHeapManager::Create( int _minSize, int _maxSize )
{
	// ���������� �������������
	SYSTEM_INFO siSysInfo; 
	::ZeroMemory( &siSysInfo, sizeof(SYSTEM_INFO) );
	::GetSystemInfo( &siSysInfo );
	pageSize = siSysInfo.dwPageSize;
	int dwAllocationGranularity = siSysInfo.dwAllocationGranularity;

	lpvReservedMem = (int*) ::VirtualAlloc( NULL, _maxSize, MEM_RESERVE, PAGE_NOACCESS );
	if( lpvReservedMem == NULL ) {
		throw std::exception( "Can not reserve memory" );
	}
	
	minSize = _minSize;
	maxSize = _maxSize;

	// �������� minSize �� �������� ������� ��������
	minSize = round( minSize, pageSize );
	// �������� maxSize �� �������� dwAllocationGranularity
	maxSize = round( maxSize, dwAllocationGranularity );
	if( minSize > maxSize ) {
		throw std::exception( "minSize > maxSize" );
	}
	int* lpvCommitedMem = (int*) ::VirtualAlloc( lpvReservedMem, minSize, MEM_COMMIT, PAGE_READWRITE );
	
	if( lpvCommitedMem == NULL ) {
		throw std::exception( "Can not commit memory" );
	}
	
	int pagesCount = round( maxSize, pageSize );
	pagesCount /= pageSize;
	for( size_t i = 0; i < pagesCount; ++i ) {
		pages.push_back( 0 );
	}

	// �������� � ����������� �������� - 4 �����, ��������� ��� �� ������� 
	minSize /= sizeof(int);
	maxSize /= sizeof(int);

	minBlockSize = pageSize / 4;
	mediumBlockSize = 128 * minBlockSize;

	if( maxSize == minBlockSize ) {
		smallBlocks.push_back( maxSize );
	} else if( maxSize < mediumBlockSize ) {
		mediumBlocks[0] = maxSize;
	} else {
		largeBlocks[0] = maxSize;
	}
}

void CHeapManager::findAppropriateBlock(int size, int& freeBlockPtr, int& smallBlockAddr, 
	int& mediumBlockAddr, int& largeBlockAddr)
{
	// ���� ���� ����������� ������� ����� ��������� ��������� ������
	if( size <= minBlockSize ) {
		for( int i = 0; i < smallBlocks.size(); ++i ) {
			// ���� ������ ����� ������ ��� ������ ������ (+1 ��� ������ ����� �������)
			// + ���� ������ ��� ����� ����������
			if( *(lpvReservedMem + smallBlocks[i]) >= size + 1 && 
				( freeBlockPtr == -1 || freeBlockPtr != -1 && *(lpvReservedMem + freeBlockPtr) > *(lpvReservedMem + smallBlocks[i]) ) ) 
			{
				freeBlockPtr = smallBlocks[i];
				smallBlockAddr = i;
			}
		}
	}
	// ���� ���� ����������� ������� ����� ������� ��������� ������
	if( size > minBlockSize && size <= mediumBlockSize || freeBlockPtr == -1 ) {
		int minFreeBlockSize = -1;
		for( auto it = mediumBlocks.begin(); it != mediumBlocks.end(); ++it ) {
			if( it->second >= size + 1 && ( freeBlockPtr == -1 || freeBlockPtr != -1 && minFreeBlockSize > it->second ) ) {
				mediumBlockAddr = freeBlockPtr = it->first;
				minFreeBlockSize = it->second;
			}
		}
	} 
	// ���� ���� ����������� ������� ����� ������� ��������� ������
	if( size > mediumBlockSize || freeBlockPtr == -1 ) {
		int minFreeBlockSize = -1;
		for( auto it = largeBlocks.begin(); it != largeBlocks.end(); ++it ) {
			if( it->second >= size + 1 && ( freeBlockPtr == -1 || freeBlockPtr != -1 && minFreeBlockSize > it->second ) ) {
				largeBlockAddr = freeBlockPtr = it->first;
				minFreeBlockSize = it->second;
			}
		}
	}
}


void CHeapManager::changeBlockStructure( int size, int& newSize, int freeBlockPtr, 
	int smallBlockAddr, int mediumBlockAddr, int largeBlockAddr )
{
	// �������� �� ���������� �����, � ������� ����� �������� ������
	if( smallBlockAddr != -1 ) {
		newSize = *(lpvReservedMem + smallBlocks[smallBlockAddr]) - size - 1;
		smallBlocks.erase( smallBlocks.begin() + smallBlockAddr );
	} else if( mediumBlockAddr != -1 ){
		newSize = mediumBlocks[mediumBlockAddr] - size - 1;
		mediumBlocks.erase( mediumBlockAddr );
	} else {
		newSize = largeBlocks[largeBlockAddr] - size - 1;
		largeBlocks.erase( largeBlockAddr );
	}
}

void CHeapManager::addFreeBlock( int size, int& newSize, int freeBlockPtr, 
	int smallBlockAddr, int mediumBlockAddr, int largeBlockAddr )
{
	if( newSize > 0 && newSize <= minBlockSize ) {
		// ����� ��������������� ����� + 1 - ������ �����
		// + size - ������ �����
		// + 1 - ������ ���������� �����
		smallBlocks.push_back( smallBlockAddr + size + 1 );
		*(lpvReservedMem + smallBlockAddr + size + 1) = newSize;
	} else if( newSize > minBlockSize && newSize <= mediumBlockSize ) {
		mediumBlocks[mediumBlockAddr + size + 1] = newSize;
	} else if( newSize > mediumBlockSize ) {
		largeBlocks[largeBlockAddr + size + 1] = newSize;
	}
}


void* CHeapManager::Alloc( int size )
{
	size = round( size, 4 );
	size /= sizeof(int);

	// ���� ��������� ���� � �������� ����������� ���������
	int freeBlockPtr = -1;
	int smallBlockAddr = -1;
	int mediumBlockAddr = -1;
	int largeBlockAddr = -1;

	// ���� ���������� ����
	findAppropriateBlock( size, freeBlockPtr, smallBlockAddr, mediumBlockAddr, largeBlockAddr );
	
	// ������ ��������� ��������� ������
	int newSize = -1;
	changeBlockStructure( size, newSize, freeBlockPtr, smallBlockAddr, mediumBlockAddr, largeBlockAddr );

	// ��������� ���������� �����
	addFreeBlock( size, newSize, freeBlockPtr, smallBlockAddr, mediumBlockAddr, largeBlockAddr );

	// ����������� ������� ����� ������ �� ��������
	for( int i = (freeBlockPtr / minBlockSize); i <= ((freeBlockPtr + size + 1) / minBlockSize); ++i ) {
		if( pages[i] == 0 ) {
			::VirtualAlloc( lpvReservedMem + i * minBlockSize, pageSize, MEM_COMMIT, PAGE_READWRITE );
		}
		++pages[i];
	}

	*(lpvReservedMem + freeBlockPtr) = size;
	allocatedBlocks.insert( freeBlockPtr );

	return lpvReservedMem + freeBlockPtr + 1;
}


void CHeapManager::findSmallFreeBlock( int& beginPtr, int& endPtr, int& blockSize, int& memSize )
{
	// ���� ���� ����� ����� ��� ����� ���
	for( int i = 0; i < smallBlocks.size(); ++i ) {
		if( smallBlocks[i] == endPtr ) {
			blockSize += *(lpvReservedMem + smallBlocks[i]);
			endPtr += *(lpvReservedMem + smallBlocks[i]);
			smallBlocks.erase( smallBlocks.begin() + i );
		} else if( smallBlocks[i] + *(lpvReservedMem + smallBlocks[i]) == beginPtr ) {
			blockSize += *(lpvReservedMem + smallBlocks[i]);
			beginPtr -= *(lpvReservedMem + smallBlocks[i]);
			smallBlocks.erase( smallBlocks.begin() + i );
		}
	}
}

void CHeapManager::findMediumFreeBlock( int& beginPtr, int& endPtr, int& blockSize, int& memSize )
{
	// ������ � ������� > beginPtr
	auto neighbourPtr = mediumBlocks.lower_bound( beginPtr );
	// ������� ���� ����� ���������� ����� ����� �����
	if( neighbourPtr != mediumBlocks.end() && neighbourPtr->first == endPtr ) {
		blockSize += neighbourPtr->second;
		endPtr += neighbourPtr->second;
		mediumBlocks.erase( neighbourPtr );
	}
	// C������ ���� ����� ���� ����� ������ �� ����
	neighbourPtr = mediumBlocks.lower_bound( beginPtr );
	if( !mediumBlocks.empty() ) {
		--neighbourPtr;
		// ��� ����� + ����� ���������� ���� + 1 = ������ ����� �����
		if( neighbourPtr != mediumBlocks.end() && neighbourPtr->first + neighbourPtr->second == beginPtr ) {
			blockSize += neighbourPtr->second;
			beginPtr -= neighbourPtr->second;
			mediumBlocks.erase( neighbourPtr );
		}
	}
}


void CHeapManager::findLargeFreeBlock( int& beginPtr, int& endPtr, int& blockSize, int& memSize )
{
	// ������ � ������� > beginPtr
	auto neighbourPtr = largeBlocks.lower_bound( beginPtr );
	// ������� ���� ����� ���������� ����� ����� �����
	if( neighbourPtr != largeBlocks.end() && neighbourPtr->first == endPtr ) {
		blockSize += neighbourPtr->second;
		endPtr += neighbourPtr->second;
		largeBlocks.erase( neighbourPtr );
	}
	// C������ ���� ����� ���� ����� ������ �� ����
	neighbourPtr = largeBlocks.lower_bound( beginPtr );
	if( !largeBlocks.empty() ) {
		--neighbourPtr;
		// ��� ����� + ����� ���������� ���� + 1 = ������ ����� �����
		if( neighbourPtr != largeBlocks.end() &&  neighbourPtr->first + neighbourPtr->second == beginPtr ) {
			blockSize += neighbourPtr->second;
			beginPtr -= neighbourPtr->second;
			largeBlocks.erase( neighbourPtr );
		}
	}
}


void CHeapManager::Free( void* mem )
{
	int* ref = (int*) mem;
	int beginPtr = ref - lpvReservedMem - 1; // ��������� �� ������ ���������� �����
	int endPtr = beginPtr + *(ref - 1) + 1;  // ��������� �� ����� ���������� �����
	int blockSize = *(ref - 1) + 1;			 // ������ + �������� ��� ����� ���������� ���� + ����� ���������� ����
	int memSize = blockSize;				 // ������ ��������� ������

	// ���� ������� ��������� ����
	findSmallFreeBlock( beginPtr, endPtr, blockSize, memSize );

	// ���� ������� ������� ����
	findMediumFreeBlock( beginPtr, endPtr, blockSize, memSize );

	// ������� ������� ����
	findLargeFreeBlock( beginPtr, endPtr, blockSize, memSize );

	// ��������� ����� ����
	if( blockSize <= minBlockSize ) {
		smallBlocks.push_back( beginPtr );
		*(lpvReservedMem + beginPtr) = blockSize;
	} else if( blockSize > minBlockSize && blockSize <= mediumBlockSize ) {
		mediumBlocks[beginPtr] = blockSize;
	} else {
		largeBlocks[beginPtr] = blockSize;
	}

	// ������� �������������� ��������
	for( int i = (ref - lpvReservedMem - 1) / minBlockSize;
		i <= ((ref - lpvReservedMem - 1 + memSize) / minBlockSize) && i < pages.size(); 
		++i) 
	{
		--pages[i];
		if( pages[i] == 0 && i > (minSize / minBlockSize) ) {
			::VirtualFree( lpvReservedMem + i * minBlockSize, pageSize, MEM_DECOMMIT );
		}
	}
	// ������� ���� �� ������ �������������
	allocatedBlocks.erase( ref - lpvReservedMem - 1 );
}

void CHeapManager::Destroy()
{
	// ������� ������ �� ��������� ������ ����� ��������� ����
	for( auto it = allocatedBlocks.begin(); it != allocatedBlocks.end(); ++it ) {
		std::cout << (lpvReservedMem + *it) << " " << *(lpvReservedMem + *it) << std::endl;
	}
	::VirtualFree( lpvReservedMem, 0, MEM_RELEASE );
}


int CHeapManager::round( int a, int roundSize ) 
{
	return ( a > ( (a / roundSize) * roundSize ) ) ? ( (a + (roundSize) ) / roundSize) * roundSize : a;
}