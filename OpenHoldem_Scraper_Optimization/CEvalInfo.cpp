#include "stdafx.h"

#include "CEvalInfo.h"
#include "CSymbols.h"

CEvalInfoFunctionArray::~CEvalInfoFunctionArray()
{
	for (int i=0;i<GetSize();i++)
		delete GetAt(i);

	RemoveAll();
}

void CEvalInfoFunctionArray::DumpFunctionArray(int indent)
{
	indent++;
	for (int i=0;i<GetSize();i++)
		GetAt(i)->DumpFunction(indent);
}

void CEvalInfoSymbol::DumpSymbol(int indent)
{
	CString message = "", format = "";

	if (indent > 0) 
	{
		format.Format("%%%ds%%s=%%.2f", indent*3);
		message.Format(format, "", m_Symbol, m_Value);
	} 
	else
		message.Format("%s=%.2f", m_Symbol, m_Value);

	p_symbols->symboltrace_collection_add(message);
}

void CEvalInfoSymbolArray::DumpSymbolArray(int indent)
{
	indent++;
	for (int i=0;i<GetSize();i++)
		GetAt(i)->DumpSymbol(indent);
}

void CEvalInfoFunction::DumpFunction(int indent)
{
	CString message = "", format = "", space = "";

	if (indent > 0) 
	{
		format.Format("%%%ds", indent*3);
		space.Format(format, "");
	}

	if (m_Cached)
		message.Format("%s%s=%.2f [Cached]", space, m_FunctionName, m_Result);
	else
		message.Format("%s%s=%.2f [Line: %d, Col: %d]", space, m_FunctionName, m_Result, m_Line, m_Column);
	
	p_symbols->symboltrace_collection_add(message);

	m_CalledFunctions.DumpFunctionArray(indent+1);
	m_SymbolsUsed.DumpSymbolArray(indent+1);
}

CEvalInfoSymbolArray::~CEvalInfoSymbolArray()
{
	for (int i=0;i<GetSize();i++)
		delete GetAt(i);

	RemoveAll();
}

CEvalInfoFunction *CEvalInfoFunctionArray::FindFunction(const char *name) 
{
	for (int i=0;i<GetSize();i++) 
	{
		if (!GetAt(i)->m_FunctionName.Compare(name))
			return GetAt(i);
	}

	return NULL;
}
