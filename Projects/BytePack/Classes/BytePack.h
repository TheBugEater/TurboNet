#pragma once
#ifndef _BYTE_DATA_H_
#define _BYTE_DATA_H_

#include <map>
#include <vector>
#include <string>

#if defined(_USRDLL)
#define TURBO_DLL     __declspec(dllexport)
#else         /* use a DLL library */
#define TURBO_DLL     __declspec(dllimport)
#endif

using namespace std;

class TURBO_DLL BytePack
{
public:
	BytePack(void);
	~BytePack(void);

	//String Values
	void					AddValue(string key,const char *value);
	void					GetValue(string key,string &value);

	//Integer Values
	void					AddValue(string key,int value);
	void					GetValue(string key,int &value);

	//Float Values
	void					AddValue(string key,float value);
	void					GetValue(string key,float &value);

	//Boolean Values
	void					AddValue(string key,bool value);
	void					GetValue(string key,bool &value);

	void					RemoveValue(string key);

	void					InitWithDataString(const char *data);

	const	char*			GetDataString();

	void					Clean();

private:
	string					m_string;
	vector<string>			Parse(string str,string delimeter);

	map<string,string>		m_valuesString;
	map<string,int>			m_valuesInt;
	map<string,float>		m_valuesFloat;
	map<string,bool>		m_valuesBool;
};

#endif