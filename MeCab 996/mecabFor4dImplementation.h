/*
 * This file is part of Wakanda software, licensed by 4D under
 *  (i) the GNU General Public License version 3 (GNU GPL v3), or
 *  (ii) the Affero General Public License version 3 (AGPL v3) or
 *  (iii) a commercial license.
 * This file remains the exclusive property of 4D and/or its licensors
 * and is protected by national and international legislations.
 * In any event, Licensee's compliance with the terms and conditions
 * of the applicable license constitutes a prerequisite to any use of this file.
 * Except as otherwise expressly stated in the applicable license,
 * such license does not include any other license or rights on this file,
 * 4D's and/or its licensors' trademarks and/or other proprietary rights.
 * Consequently, no title, copyright or other proprietary rights
 * other than those specified in the applicable license is granted.
 */
#ifndef ____mecabFor4dImplementation__
#define ____mecabFor4dImplementation__

#ifndef _WIN32
#include <Cocoa/Cocoa.h>
#define MECAB_DLL_EXPORT  
#else
#include <Windows.h>
//removed code to use find(), we don't process duplicates in this piece of code
//#include <algorithm>	//to use find()
#ifndef uint8_t
typedef unsigned char uint8_t;
//moved to Mecab4DInterface.h
//#define MECAB_DLL_EXPORT __declspec(dllexport) 
#endif
#endif

#include "Mecab4DInterface.h"
#include "mecab.h"

#include <string>
#include <map>
#include <iostream>
#include <sstream> 

class VMecabModel : public IMecabModel
{
private: 	
	
	std::string		fMecabSignature;
	
public:
	
	//used internally to process keywords (ignore, concatenate...)
	typedef enum keywordActionType
	{
		
		KEYWORD_NO_ACTION	= 0,
		KEYWORD_ADD			= 1,
		KEYWORD_REPLACE		= 2	
		
	}keywordActionType;
	
	keywordActionType keywordActionTypeForPosIdPair(unsigned short previousPosId, unsigned short currentPosId);
	
	void			Release();	
	
	// 4D will create one lattice per database context
	LatticeRef		CreateLattice();
	void			ReleaseLattice( LatticeRef inLattice);
    
	// returns a vector of pairs (offset in utf8 text, length of word)
	bool			GetWordBoundaries( LatticeRef inLattice, const uint8_t *inTextUTF8, size_t inTextLength, size_t **outWords, size_t *outCount);
	
	void			ReleaseWordBoundaries( size_t *inWords);
	
	void			Init();
	
    MeCab::Model*   fMeCabModel;
    MeCab::Tagger*  fMeCabTagger;
	
	const char *	GetSignature(){return this->fMecabSignature.c_str();};
	const char *	GetVersion(){return this->fMeCabModel->version();};
	
};

#endif /* defined(____mecabFor4dImplementation__) */
