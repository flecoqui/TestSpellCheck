// TestSpellCheck.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


inline void HR(HRESULT const result)
{
	if (S_OK != result)
		exit(1);
}
bool ParseCommandLine(int argc, wchar_t* argv[], std::wstring& lang, std::wstring& text) 
{
	bool success = false;
	while (--argc != 0)
	{

		std::wstring option = *++argv;
		if (option[0] == '-')
		{
			if (option == L"--lang")
			{
				if (--argc != 0)
					lang = *++argv;
			}
			else if (option == L"--text")
			{
				if (--argc != 0)
					text = *++argv;
			}
		}
	}
	if((lang.length()>0)&&
	   (text.length()>0))
		success = true;
	return success;
}

int _tmain(int argc, wchar_t* argv[])
{
	std::wstring lang = L"en-US";
	std::wstring text = L"";
	_setmode(_fileno(stdout), _O_U16TEXT);

	bool result = ParseCommandLine(argc, argv, lang, text);
	if (result == false)
	{
		std::wstringstream oss;
		oss.str(std::wstring());
		oss << "Native Spell Check syntax error" << std::endl;
		oss << "Syntax:" << std::endl;
		oss << "TestspellCheck.exe --lang \"Text Language\" --text \"Text to be checked \" " << std::endl;
		oss << "For instance: " << std::endl;
		oss << "TestspellCheck.exe --lang \"fr-FR\" --text \"Après le coup de télephone, j'ai bu un cafe et une crème brûlee inouïe à Nimes\" " << std::endl;
		std::wcout << oss.str().c_str();
		return 0;
	}

	HRESULT hr = CoInitializeEx(nullptr, // reserved
		COINIT_MULTITHREADED);
	if (SUCCEEDED(hr))
	{
		CComPtr<ISpellCheckerFactory> factory;
		hr = factory.CoCreateInstance(__uuidof(SpellCheckerFactory));
		if (SUCCEEDED(hr))
		{
			BOOL supported;			
			hr = factory->IsSupported(lang.c_str(), &supported);
			if (SUCCEEDED(hr))
			{
				if (supported)
				{
					CComPtr<ISpellChecker> checker;
					hr = factory->CreateSpellChecker(lang.c_str(), &checker);
					if (SUCCEEDED(hr))
					{
						std::wstringstream oss;
						std::wstring newtext = L"";
						oss << "Native Spell Check in language \"" << lang.c_str() << "\" text: \"" << text.c_str() << "\"" << std::endl;
						std::wcout << oss.str().c_str();
						CComPtr<IEnumSpellingError> errors;
						hr = checker->Check(text.c_str(),
							&errors);
						if (SUCCEEDED(hr))
						{
							ULONG LastIndex = 0;
							for (;;)
							{
								CComPtr<ISpellingError> error;

								if (S_OK != errors->Next(&error))
								{
									std::wstring temp(text.c_str() + LastIndex,
										text.c_str() + text.length() );

									newtext += temp;
									break;
								}

								// Respond to each error here!
								ULONG startIndex;
								hr = error->get_StartIndex(&startIndex);

								ULONG length;
								hr = error->get_Length(&length);

								std::wstring temp(text.c_str() + LastIndex,
									text.c_str() + startIndex );
								LastIndex = startIndex + length;

								newtext += temp;

								std::wstring word(text.c_str() + startIndex,
									text.c_str() + startIndex + length);
								oss.str(std::wstring());
								oss << "\tSpelling mistake in the word: " << word.c_str() << std::endl;
								std::wcout << oss.str().c_str();
								CORRECTIVE_ACTION action;
								hr = error->get_CorrectiveAction(&action);
								if (action == CORRECTIVE_ACTION_GET_SUGGESTIONS)
								{

									oss.str(std::wstring());
									oss << "\t\tSuggestions: " << std::endl;
									std::wcout << oss.str().c_str();
									CComPtr<IEnumString> suggestions;

									hr = checker->Suggest(word.c_str(),
										&suggestions);
									bool suggestionAdded = false;
									for (;;)
									{
										wchar_t * suggestion;

										if (S_OK != suggestions->Next(1, &suggestion, nullptr))
										{
											break;
										}

										// Add the suggestion to a list for presentation
										oss.str(std::wstring());
										oss << suggestion  << std::endl;
										std::wcout << oss.str().c_str();
										if (suggestionAdded == false)
										{
											suggestionAdded = true;
											newtext += suggestion;
										}
										CoTaskMemFree(suggestion);
									}


								}
								else if (action == CORRECTIVE_ACTION_REPLACE) {
									wchar_t * replacement;
									hr = error->get_Replacement(&replacement);

									oss.str(std::wstring());
									oss << "\t\tReplacement with word: " << replacement << std::endl;
									std::wcout << oss.str().c_str();
									newtext += replacement;
									CoTaskMemFree(replacement);

								}
								else if (action == CORRECTIVE_ACTION_DELETE)
								{
									oss.str(std::wstring());
									oss << "\t\tDelete word: " << word << std::endl;
									std::wcout << oss.str().c_str();

								}
							}

						}
						oss.str(std::wstring());
						oss << "Input text:  " << text.c_str() << std::endl;
						if(newtext != text)
							oss << "Output text: " << newtext.c_str()  << std::endl;
						else 
							oss << "No error " << std::endl;
						std::wcout << oss.str().c_str();
					}
				}
				else
				{
					std::wstringstream oss;
					oss.str(std::wstring());
					oss << "Native Spell Check in " << lang.c_str() << " language not supported" << std::endl;
					std::wcout << oss.str().c_str();
				}
			}
		}

	}
	CoUninitialize();
    return 0;
}

 