/**
 * \file    LV_FasrMedian.cpp
 * \brief	Implementacja szybkiej mediany oparta o poprzedni projekt.
 * \author  PB
 * \date    2012/09/08
 */

#include "stdafx.h"

#ifndef SAFE_DELETE
	#define SAFE_DELETE(p)       { if(p) { delete[] (p);     (p)=NULL; } }
#endif

/** 
 * Zwraca warto�� z tablicy na pozycji [r,k], przy za�ozeniu �e tablica jest
 * interpretowana jako 2D
 * \param[in] image		struktura opisuj�ca obraz
 * \param[in] r			rz�d
 * \param[in] k			kolumna
 * \return warto�� na pozycji tab[r,k]
 * \remarks W obrazie dane uk�adane sa rzedami, zgodnie z C_MatrixContainer
 * 
*/
inline unsigned short getPoint(OBRAZ *image, unsigned int r, unsigned int k)
{
	_ASSERT(r*image->cols+k<image->tabsize);
	return image->tab[r*image->cols+k];
}

/** 
 * Zwraca median� z wektora warto�ci
 * \param[in] tab		tablica warto�ci
 * \param[in] tabsize		rozmiar tablicy
 * \return warto�� mediany
*/
unsigned short getMedian(const unsigned short *tab, unsigned int tabsize)
{
	std::vector<unsigned int> v( tab, tab+tabsize );
	std::nth_element( v.begin(), v.begin()+tabsize/2,v.end() );
	return v[tabsize/2];
}

/** 
 * Kopiuje okno o rozmiarze 2*halfmedian+1 z obrazka wej�ciowego do tablicy out.
 * Kopiuje rz�dami jak na obrazie oryginalnym dodatkowow tworzy histogram
 * \param[in] input_image		obraz wej�ciowy
 * \param[in] mask				nieparzysty rozmiar maski
 * \param[in] current_row		indeks rz�du dla kt�rego wyci�� okno - indeks w kt�rym jest �rodek okna
 * \param[in] current_col		indeks kolumny dla kt�rej wyci�� okno - indeks w kt�rym jest �rodek okna
 * \param[out] out				tablica wyj�ciowa o rozmiarze maski
 * \param[out] hist				tablica z histogramem okna
 * \remarks Funkcja czy�ci tablic� hist. Je�li okno wystaje poza obszar obrazu to wpisywane s� 0 do okna
 * \todo T� funkcj� mo�na przyspieszy� u�ywaj�c kopiowania ca�ych rz�d�w !!
*/
void CopyWindow( OBRAZ *input_image, unsigned short mask, unsigned int current_row, unsigned int current_col, unsigned short *out, unsigned int *hist )
{
	int wr,wk;			// indeksy wewn�trz okna
	unsigned short bok_maski = (mask-1)/2;
	unsigned int l;	// licznik
	memset(hist, 0, GRAYSCALE*sizeof(unsigned int));	// szybkie zerowanie pami�ci histogramu (dla kazdego rz�du procedura dzia�a od pocz�tku)
//	_ASSERT(current_row-bok_maski<current_row);	// wykrywanie przepe�nienia, je�li z�y punkt startowy
//	_ASSERT(current_col-bok_maski<current_col);	// wykrywanie przepe�nienia, je�li z�y punkt startowy
//	_ASSERT(current_row+bok_maski<input_image->rows);	// ko�c�wka obrazu
//	_ASSERT(current_col+bok_maski<input_image->cols);	// ko�c�wka obrazu
	for (wr = static_cast<int>(current_row)-bok_maski,l=0;wr<static_cast<int>(current_row)+bok_maski+1;wr++)	
		for (wk = static_cast<int>(current_col)-bok_maski;wk<static_cast<int>(current_col)+bok_maski+1;wk++)
			{
				if(wr<0 || wk<0 || wk>=static_cast<int>(input_image->cols) || wr>=static_cast<int>(input_image->rows))	// je�li okno wystaje poza obraz to wpisywane s� w to miejsce zera
					out[l] = 0;
				else
					out[l] = getPoint(input_image,wr,wk);	// kopiowanie danych z obrazu do osobnej tablicy reprezentuj�cej okno
		 		hist[out[l]]++;	// obliczam histogram
				l++;				
			}
}

/** 
 * Zwraca median� z wektora reprezentowanego przez jego histogram
 * \param[in] hist		tablica z histogramem 
 * \param[in] tabsize		rozmiar tablicy
 * \return warto�� mediany
 * \remarks Na podstawie http://mathematics.blurtit.com/q6132160.html
*/
unsigned short getMedianHist( const unsigned int *hist, unsigned int tabsize )
{
	unsigned int cum_hist[GRAYSCALE]; // histogram kumulacyjny
	unsigned short cum_index[GRAYSCALE];	// indeksy z histo kt�re wchodz� w sklad kumulacyjnego (niezerowe warto�ci histogramu)
	unsigned short licznik;
	unsigned short a=0;
	unsigned short M;
	unsigned int prev_cum;

	while(a<GRAYSCALE-1 && 0==hist[a])	// je�li pierwszy element histo jest 0, to szukamy pierwszego niezerowego, a sprawdzam w zakresie 0-65535 (uwaga na przepe�nienie)
		a++;
	if(a==GRAYSCALE-1)	// to znaczy �e w histo s� same 0, ko�czymy, zwracaj�c median�
		return 0;
	
	// je�li normalny histogram to wycinamy warto�ci niezerowe
	cum_hist[0] = hist[a];	// pierwsza warto�� niezerowa
	cum_index[0] = a;	// oraz jej indeks
	licznik = 1;	// od nast�pniej warto�ci bo zerowa ju� ustawiona powy�ej
	// obliczanie kumulacyjnego ale tylko z warto�ci niezerowych
	for(unsigned int b=a+1;b<GRAYSCALE;b++)	// uwaga na przepe�nienie - b nie mo�e by� szort
		if (hist[b]>0)
		{
			cum_hist[licznik] = cum_hist[licznik-1] + hist[b]; // niezerowa warto��
			cum_index[licznik] = b; // oraz jej index w histo (jasno�� piksela)
			licznik++;
		}
	--licznik;	// indeks ostatniego elementu w cum_hist i cum_indeks
	M = cum_hist[licznik]/2;
	// do kt�rego przedzia�u trafia M?
	for(a=0;a<=licznik;a++)
		if(M<cum_hist[a])
			break;
	if(0==a)	// czyli nale�y do pierwszego przedzia�u
		prev_cum = 0;
	else
		prev_cum = cum_hist[a-1];

	// mediana
	return cum_index[a] + (M-prev_cum)/hist[ cum_index[a] ];
}

/** 
 * Filtruje obraz median�
 * Na podstawie Huang, A Fast Two-Dimensional Median Filtering Algorithm.
 * \param[in] image		obraz wej�ciowy
 * \param[out] tabout	wska�nik na tablice wyj�ciow� z przefiltrowanym obrazem o rozmiarze obrazu wej�ciowego
 * \param[in] mask		rozmiar maski, maska nieparzysta i kwadratowa
 * \remarks Na rogach obrazu pojawiaj� si� zera. Pozatym mo�na procedur� jeszcze przyspieszy� modyfikuj�c pierwsz� median� (mo�e na containerze b�dzie szybsza (getMedian)
 * oraz modyfikuj�c pobieranie warto�ci okna poprzez kopiowanie ca�ych rzed�w na raz (s� liniowo w pami�ci).
*/
void FastMedian_Huang(	OBRAZ *image,
						unsigned short *tabout, 
						unsigned short mask)
{
	unsigned int *hist=NULL;				// histogram obszaru filtrowanego
	unsigned short *window = NULL;			// dane z okna maski
	unsigned short mdm;						// warto�� mediany w oknie
 	unsigned int lmdm;					// liczba element�w obrazu o warto�ciach mniejszych od mdm
 	unsigned short *left_column = NULL;		// lewa kolumna okna
 	unsigned short *right_column = NULL;	// prawa kolumna okna
 	unsigned int r,k;						// indeksy bierz�cej pozycji okna na obrazie (�rodka)
 	unsigned short mask_center = (mask+1)/2;// �rodek maski (indeks)
 	unsigned short bok_maski = (mask-1)/2;	// rozmiar boku maski ca�a maska to 2*bok + 1 
 	unsigned int l;							// licznik
	unsigned short picval;					// pomocnicza warto�� piksela obrazu
	unsigned int th = (mask*mask)/2;			// parametr pomocniczy
 
 	hist = new unsigned int[GRAYSCALE];		// zak�adam g��bi� 16 bit
 	left_column = new unsigned short[mask];	// lewa kolumna poprzedniej pozycji maski (maska jest zawsze kwadratowa)
 	right_column = new unsigned short[mask];// prawa kolumna bierzacej maski
 	window = new unsigned short[static_cast<unsigned int>(mask)*mask];
 	/* 
 	 * Przegl�danie obrazu po rz�dach a procedura szybkiej filtracji po 
 	 * kolumnach. Dla kazdego nowego rz�du powtarza si� wszystko od pocz�tku.
 	 */
 	for (r = 0;r<image->rows;r++)	// g��wna p�tla po rz�dach obrazu
 	{
 		// -------------------- inicjalizacja parametr�w dla ka�dego rz�du --------------------------
		k = 0;
		CopyWindow(image,mask,r,k,window,hist);	// kopiowanie okna skrajnego lewego dla danego rz�du
 // 		mdm = getMedian(window,static_cast<unsigned int>(mask)*mask);	// znajduj� median� z okna
		mdm = getMedianHist(hist,GRAYSCALE);
		tabout[r*image->cols+k] = mdm;	// ustawiam wyj�cie przy za�o�eniu �e tabout jaest taka sama jak tabin
		// obliczanie lmdm
		for(l=0,lmdm=0;l<static_cast<unsigned int>(mask)*mask;l++)
			if(window[l]<mdm)
				lmdm++;
		for (k = 0+1;k<image->cols;k++)	// g��wna p�tla po kolumnach obrazu, dla pierwszej pozycji k=mask_center obliczane jest osobno
 		{

 			 // modyfikacja histogramu - Na podstawie Huang, A Fast Two-Dimensional Median Filtering Algorithm 

			CopyOneColumn(image,mask,static_cast<int>(r)-bok_maski,static_cast<int>(k)-bok_maski-1,left_column); //pobieranie lewej kolumny poprzedniego (k-1) okna (podaj� [r,k] pocz�tku kolumny	
			CopyOneColumn(image,mask,static_cast<int>(r)-bok_maski,static_cast<int>(k)+bok_maski,right_column);	// prawa kolumna bierz�cego k okna
			// liczenie mediany
			for(l=0;l<mask;l++)	// po wszystkich warto�ciach kolumny
			{
				picval = left_column[l];
				_ASSERT(hist[picval]>0);	// je�li =0 to po odj�ciu przepe�nienie
				hist[ picval ]--;	// kasowanie lewej kolumny z histogtramu
				if (picval<mdm)
					lmdm--;
				picval = right_column[l];	
				_ASSERT(hist[picval]<65535);	// je�li =65535 to po odj�ciu przepe�nienie
				hist[picval]++;		// dodawanie prawej kolummny
				if (picval<mdm)
					lmdm++;
			}	// koniec p�tli po kolumnach okna
			if (lmdm>th)
				while (lmdm>th)	// zmiana wzgl�dem orygina�u !!! FUCK !!
				{
					_ASSERT(mdm>0);
					mdm--;
					_ASSERT(lmdm>=hist[mdm]);
					lmdm-=hist[mdm];
				} 
			else
				while(lmdm+hist[mdm]<=th) {
					lmdm+=hist[mdm];
					_ASSERT(mdm<65535);
					mdm++;
				}		

			tabout[r*image->cols+k] = mdm;	// ustawiam wyj�cie przy za�o�eniu �e tabout jaest taka sama jak tabin
		} // koniec p�tli po kolumnach obrazu
  
 	} // koniec p�tli po rz�dach
 
 	SAFE_DELETE(hist);
 	SAFE_DELETE(left_column);
 	SAFE_DELETE(right_column);
 	SAFE_DELETE(window);
}

/** 
 * kopiuje jedn� kolumn� zaczynaj�c od pozycji poz
 * Na podstawie Huang, A Fast Two-Dimensional Median Filtering Algorithm 
 * \param[in] input_image		obraz wej�ciowy
 * \param[in] mask		rozmiar maski, maska nieparzysta i kwadratowa
 * \param[in] r			rz�d startowy
 * \param[in] k			kolumna startowa	
 * \param[out] out		tablica o rozmiarze mask z kolumn�	
 * \remarks Procedura dopuszcza ujemne kolumny i rz�dy, co odpowiada sytuacji gdy kolumna nie miesci si�
 * na obrazie, czyli dla berzeg�w
*/
void CopyOneColumn( OBRAZ *input_image, unsigned short mask, int r, int k, unsigned short *out )
{
	unsigned short a;
	for (a=0;a<mask;a++)
	{
		if(r<0 || r>=static_cast<int>(input_image->rows) || k<0 || k>=static_cast<int>(input_image->cols))	// je�li rz�d lub kolumna poza obrazem t owpisywane s� 0
			out[a] = 0;
		else
			out[a] = getPoint(input_image,r,k);
		r++;
	}
}

/** 
 * Filtruje obraz median� - funkcja exportowalna dl DLL. Zak�ada �e obrz jest podawany wierszami w tablicy 1D
 * \param[in] input_image		obraz wej�ciowy
 * \param[out] output_image	wska�nik na tablice wyj�ciow� z przefiltrowanym obrazem o rozmiarze obrazu wej�ciowego
 * \param[in] nrows		liczba wierszy
 * \param[in] ncols liczba kolumn
 * \remarks Funckja dokonuje transformacji parametr�w wej�ciowych na format z poprzedniego projektu
 * \warning Zak�ada mask� o rozmiarze 31x31
*/
extern "C" __declspec(dllexport) void med_filt(const UINT16* input_image, UINT16* output_image, UINT16 nrows, UINT16 ncols)
{
	OBRAZ obraz;	// lokalna kopia obrazu wej�ciowego (p�ytka)
	unsigned short mask = 31;	// rozmiar maski
	obraz.tab = input_image;
	obraz.rows = nrows;
	obraz.cols = ncols;
	obraz.tabsize = nrows*ncols;
	FastMedian_Huang(&obraz,output_image,mask);
}

/** 
 * Filtruje obraz median� - funkcja exportowalna dl DLL. Zak�ada �e obrz jest podawany wierszami w tablicy 1D. Dodatkowo pozwala na ustawienie maski
 * \param[in] input_image		obraz wej�ciowy
 * \param[out] output_image	wska�nik na tablice wyj�ciow� z przefiltrowanym obrazem o rozmiarze obrazu wej�ciowego
 * \param[in] nrows		liczba wierszy
 * \param[in] ncols liczba kolumn
 * \param[in] mask Rozmiar maski
 * \remarks Funckja dokonuje transformacji parametr�w wej�ciowych na format z poprzedniego projektu
*/
extern "C" __declspec(dllexport) void med_filt_mask(const UINT16* input_image, UINT16* output_image, UINT16 nrows, UINT16 ncols, UINT16 mask)
{
	OBRAZ obraz;	// lokalna kopia obrazu wej�ciowego (p�ytka)
	obraz.tab = input_image;
	obraz.rows = nrows;
	obraz.cols = ncols;
	obraz.tabsize = nrows*ncols;
	FastMedian_Huang(&obraz,output_image,mask);
}