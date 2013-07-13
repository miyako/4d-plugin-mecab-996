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
#ifndef __Mecab4DInterface__
#define __Mecab4DInterface__

#include <vector>

// this preprocessor definition was added to the vcproj
#ifdef MECAB_DLL
#define MECAB_DLL_EXPORT __declspec(dllexport) 
#else
#define MECAB_DLL_EXPORT 
#endif

namespace MeCab
{
    class Lattice;
};

class IMecabModel
{
public:
	
	typedef MeCab::Lattice*	LatticeRef;
	
	virtual	void			Release() = 0;
	
	// 4D will create one lattice per database context
	virtual	LatticeRef		CreateLattice() = 0;
	virtual	void			ReleaseLattice( LatticeRef inLattice) = 0;
	
	// returns a vector of pairs (offset in utf8 text, length of word)
	virtual bool			GetWordBoundaries( LatticeRef inLattice, const uint8_t *inTextUTF8, size_t inTextLength, size_t **outWords, size_t *outCount) = 0;
	
	// on windows we need to release the object in the same dll that created it
	virtual void			ReleaseWordBoundaries( size_t *inWords) = 0;
	
};

typedef IMecabModel* (*CreateMecabModelProc)();

extern "C"
{
	// this is the only exported symbol.
	// 4D will create only one mecab model at launch time
	
	MECAB_DLL_EXPORT IMecabModel*	CreateMecabModel();
	
	// altarnative interface; load user dictionary from path if it exists.
	MECAB_DLL_EXPORT IMecabModel*	CreateMecabModelWithUserDictionary(const uint8_t *inPathUTF8, size_t inPathLength);	
};

#endif /* defined(__Mecab4DInterface__) */
