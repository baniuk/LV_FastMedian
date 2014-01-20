/**
 * \file    LV_FasrMedian_Caller.cpp
 * \brief	Wywo³uje funkcje z biblioteki LV_FastMEdian.dll
 * \author  PB
 * \date    2014/01/20
 */

#include "stdafx.h"
#include "C_Matrix_Container.h"
#include "C_DumpAll.h"

typedef void (*p_med_filt)(UINT16*, UINT16*, UINT16, UINT16); 

using namespace std;

/**
 * G³ówna funkcja - ³aduje bibliotekê oraz uruchamia medianê.
 */ 
int _tmain(int argc, _TCHAR* argv[])
{
	HINSTANCE hinstLib; 
	BOOL ret;
	unsigned short *input_image;	// input image
	unsigned short *output_image;
	// WskaŸnik na funkcje z DLLa
	p_med_filt med_filt; 
	hinstLib = LoadLibrary(TEXT("LV_FastMedian.dll")); 
	if(hinstLib==NULL)
	{
		cerr << "Error in LoadLibrary" << endl;
		return 1;
	}
	else
		cout << "Library loaded" << endl;
	// funkcje
	med_filt = (p_med_filt)GetProcAddress(hinstLib, "med_filt"); 
	if(med_filt==NULL)
	{
		cerr << "Error in GetProcAddress" << endl;
		return 1;
	}
	else
		cout << "Addres of med_filt: " << med_filt << endl;
	// Przygotowanie danych dla funkcji
	// Tymczasowe readbinary i konwersja do formatu dlla
	C_MATRIX_LOAD(tmp_input_image,"testimag1.dat"); // load test file
	tmp_input_image.Normalize(0,1);	// normalization
	input_image = new unsigned short[tmp_input_image.GetNumofElements()];
	if(input_image==NULL)
	{
		cerr << "Error in memory allocation" << endl;
		return 1;
	}
	for(unsigned int a=0;a<tmp_input_image.GetNumofElements();a++)
		input_image[a] = static_cast<unsigned short>(floor(65535*tmp_input_image.data[a]+0.5));
	
	// wywo³anie funkcji
	output_image = new unsigned short[tmp_input_image.GetNumofElements()];
	if(output_image==NULL)
	{
		cerr << "Error in memory allocation" << endl;
		return 1;
	}
	med_filt(input_image,output_image,tmp_input_image._rows,tmp_input_image._cols);
	// nagranie wyjœcia
	C_DumpAll dump("test_out.dat");
	C_Matrix_Container out;
	out.AllocateData(tmp_input_image._rows,tmp_input_image._cols);
	for(unsigned int a=0;a<tmp_input_image.GetNumofElements();a++)
		out.data[a] = static_cast<double>(output_image[a]);
	dump.AddEntry(&out,"outimage");
	dump.AddEntry(&tmp_input_image,"inimage");

	// zwalnianie biblioteki
	ret = FreeLibrary(hinstLib);
	if(ret==NULL)
	{
		cerr << "Error in FreeLibrary" << endl;
		SAFE_DELETE(input_image);
		SAFE_DELETE(output_image);
		return 1;
	}
	SAFE_DELETE(input_image);
	SAFE_DELETE(output_image);
	return 0;
}
