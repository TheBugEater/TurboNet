#include <iostream>
#include "NetworkData.h"
#include <conio.h>

using namespace std;

int main()
{
	NetworkData *pData = new NetworkData;

	pData->AddValue("HI",5);
	pData->AddValue("HS",15);
	pData->AddValue("HG",55);

	pData->AddValue("asd",5.023f);
	pData->AddValue("gsa",151235.023f);
	pData->AddValue("g",55243151245.023f);

	pData->AddValue("bfs",true);
	pData->AddValue("te",false);
	pData->AddValue("HGbhs",true);

	pData->GetDataString();


	getch();
}