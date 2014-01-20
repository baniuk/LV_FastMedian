#pragma once
#include <stdio.h>
#include <crtdbg.h>
#include <string.h>
#include "C_Matrix_Container.h"

#define MAX_ENTRY 256
#define DUMPALL_DOUBLE 1
#define C_MATRIX 2
#define C_IMAGE 3
#define DUMPALL_FLOAT 4
#define DUMPALL_UINT16 5

class C_DumpAll
{
public:
	C_DumpAll(const char* filename);
	void AddEntry(const double* data, unsigned int size, char* name);
	void AddEntry(float* data, unsigned int size, char* name);
	void AddEntry(C_Matrix_Container* data, char* name);
	void AddEntry(const unsigned short* data,unsigned int size, char* name);
	~C_DumpAll(void);
private:
	FILE *stream;
	bool openfileerror;
	int lastpozindex;				//index ostatniego wpisu w offset
	unsigned long offset[MAX_ENTRY]; //offsety kolejnych wpis�w
};
