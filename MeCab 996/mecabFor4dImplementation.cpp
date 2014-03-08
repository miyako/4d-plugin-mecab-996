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
#include "mecabFor4dImplementation.h"

void VMecabModel::Release(){
	
	if(fMeCabModel){
		
		if(fMeCabTagger){
			
			delete fMeCabTagger;
			
		}
		
		delete fMeCabModel;
	}
	
	delete this;
	
}

void VMecabModel::Init(){

	if(fMeCabTagger){
		
		const MeCab::DictionaryInfo *d = fMeCabTagger->dictionary_info();
		
		std::map< std::string , unsigned short > map;	// sorted list of dics with their versions	
		for (; d; d = d->next){
			
			std::string filename( d->filename);
#ifdef _WIN32			
			size_t found = filename.find_last_of("/\\");
#else
			size_t found = filename.find_last_of("/");			
#endif			
			if(found != std::string::npos)
				filename = filename.substr(found + 1); 
			
			map.insert(std::map< std::string , unsigned short >::value_type( filename, d->version));
		}
		std::ostringstream s;
		for(std::map< std::string , unsigned short >::iterator i = map.begin() ; i != map.end() ; ++i)
			s << i->first << ":" <<  i->second << ";";
		
		fMecabSignature = s.str();
		
	}
	
}	

BOOL GetMeCabResourcePath(std::string &rcfile){
	
	//the file mecabrc should be placed at /Resources/mecab/mecabrc
	//it can be an empty file, but required to initialize mecab
	//in fact the file must be empty, in order not to load unexpected dictionaries
	//mecab accepts path in utf-8 on both platforms	
	//this function returns the mecabrc path in utf-8
	
	rcfile = "--output-format-type=none\n--rcfile=";
	
#ifdef _WIN32
	
	//wchar_t	mainExePath[_MAX_PATH] = {0};
	wchar_t	fDrive[_MAX_DRIVE], fDir[_MAX_DIR], fName[_MAX_FNAME], fExt[_MAX_EXT];
	wchar_t	libmecabPath[ _MAX_PATH ] = {0};
	
	HMODULE libmecab = GetModuleHandleW(L"MeCab 996.4dx");
	
	if(libmecab){
		if(GetModuleFileNameW(libmecab, libmecabPath, _MAX_PATH)){
			_wsplitpath_s(libmecabPath, fDrive, fDir, fName, fExt);
			std::wstring mecabrcPath = std::wstring(fDrive) + std::wstring(fDir) + L"mecabrc";
			
			//the second paramter should be 0, WC_ERR_INVALID_CHARS will fail on XP	
			int len = WideCharToMultiByte(CP_UTF8, 0, mecabrcPath.c_str(), -1, NULL, 0, NULL, NULL);
			
			if(len){
				std::vector<uint8_t> buf(len);
				if(WideCharToMultiByte(CP_UTF8, 0, mecabrcPath.c_str(), mecabrcPath.size(), (LPSTR)&buf[0], len, NULL, NULL)){
					rcfile += (const char *)&buf[0];
					return TRUE;
				}
			}
		}
	}
	
	/*
	if(GetModuleFileNameW(NULL, mainExePath, _MAX_PATH)){
		
		_wsplitpath_s(mainExePath, fDrive, fDir, fName, fExt);
		std::wstring mecabrcPath = std::wstring(fDrive) + std::wstring(fDir) + L"Resources\\mecab\\mecabrc";
		
		//the second paramter should be 0, WC_ERR_INVALID_CHARS will fail on XP
		int len = WideCharToMultiByte(CP_UTF8, 0, mecabrcPath.c_str(), mecabrcPath.size(), NULL, 0, NULL, NULL);
		
		if(len){
			std::vector<uint8_t> buf(len + 1);
			if(WideCharToMultiByte(CP_UTF8, 0, mecabrcPath.c_str(), mecabrcPath.size(), (LPSTR)&buf[0], len, NULL, NULL)){
				rcfile += (const char *)&buf[0];
				return TRUE;
			}
		}
	}
	*/
	
#else	
	
	NSBundle *libmecab = [NSBundle bundleWithIdentifier:@"com.4D.4DPlugin.miyako.MeCab.996"];
	
	if(libmecab){
		NSString *mecabrcPath = [[[libmecab executablePath]stringByDeletingLastPathComponent]stringByAppendingPathComponent:@"mecabrc"];
		rcfile += (const char *)[mecabrcPath UTF8String];
		return TRUE;
	}
	
	/*
	NSBundle *mainBundle = [NSBundle mainBundle];
	 
	if(mainBundle){
		
		NSString *mecabrcPath = [[[mainBundle resourcePath]stringByAppendingPathComponent:@"mecab"]stringByAppendingPathComponent:@"mecabrc"];
		rcfile += (const char *)[mecabrcPath UTF8String];
		return TRUE;
	}
	*/
	
#endif
	
	return FALSE;
}

BOOL GetMeCabSystemDictionaryFolderPath(std::string &dicdir){
	
	//using juman since 0.996, not naist
	//https://sites.google.com/site/masayua/m/majanalyzer
	//mecab handles posix delimiters on both platforms
	
	//dicdir = "\n--dicdir=$(rcpath)/../resources/dic/jumandic-7.0";

	return TRUE;	
}

IMecabModel* CreateMecabModelWithUserDictionary(const uint8_t *inPathUTF8, size_t inPathLength){
	
	VMecabModel *model = new VMecabModel;
	
	//construct the command-line-style argument to initialise mecab model
	//we have modified file param.cpp (192) to use 0x0A as delimeter instead of isspace() 
	//mecab accepts path in utf-8 on both platforms	
	//http://mecab.googlecode.com/svn/trunk/mecab/doc/mecab.html
	
	std::string rcfile;
	GetMeCabResourcePath(rcfile);
	
	std::string dicdir;
	GetMeCabSystemDictionaryFolderPath(dicdir);	
	
	std::string usrdic = "\n--userdic=";
	usrdic += std::string((const char *)inPathUTF8, inPathLength);	
	
	model->fMeCabModel = MeCab::createModel((rcfile + dicdir + usrdic).c_str());
	
	if(model->fMeCabModel){
		
		model->fMeCabTagger = model->fMeCabModel->createTagger();
		
		model->Init();
		
	}	
	
	return model;	
}

IMecabModel* CreateMecabModel(){
	
	VMecabModel *model = new VMecabModel;
	
	std::string rcfile;
	GetMeCabResourcePath(rcfile);
	
	//std::string dicdir;
	//GetMeCabSystemDictionaryFolderPath(dicdir);	
	
	//model->fMeCabModel = MeCab::createModel((rcfile + dicdir).c_str());
	model->fMeCabModel = MeCab::createModel((rcfile ).c_str());
	
	if(model->fMeCabModel){
		
		model->fMeCabTagger = model->fMeCabModel->createTagger();
		
		model->Init();
		
	}
	
	return model;
}

// 4D will create one lattice per database context
IMecabModel::LatticeRef VMecabModel::CreateLattice(){
	
	LatticeRef lattice = NULL;
	
	if(this->fMeCabModel){
		
		//http://mecab.googlecode.com/svn/trunk/mecab/doc/doxygen/classMeCab_1_1Lattice.html
		lattice = this->fMeCabModel->createLattice();
		
	}
	
	return lattice;	
	
}

void VMecabModel::ReleaseLattice( LatticeRef inLattice){
	
	if(inLattice){
		
		delete inLattice;
		
	}
	
}

void VMecabModel::ReleaseWordBoundaries( size_t *inWords){
	
	if(inWords != NULL){
		free( inWords);	
	}
	
}

// returns a vector of pairs (offset in utf8 text, length of word)
bool VMecabModel::GetWordBoundaries( LatticeRef inLattice, const uint8_t *inTextUTF8, size_t inTextLength, size_t **outWords, size_t *outCount){
	
	bool ok = false;
	std::vector<std::pair<size_t,size_t> > boundaries;
	if(inTextLength){
		
		if(this->fMeCabModel){
			
			if(this->fMeCabTagger){
				
				if(inLattice){
					
					inLattice->set_sentence((const char *)inTextUTF8, inTextLength);
					
					if(this->fMeCabTagger->parse(inLattice)){
						
						size_t newPos = 0, oldLen = 0;
						size_t newLen = 0, oldPos = 0;
						
						unsigned short oldPosId = 0;
						
						//bos = beginning of string, eos = end of string
						const MeCab::Node* node = inLattice->bos_node();
						
						while(node){
							
							BOOL shouldAdd = FALSE;
							
							switch(node->stat){
								case MECAB_BOS_NODE:
								case MECAB_EOS_NODE:
									break;
								default:	
									
									newPos = (const uint8_t *)node->surface - inTextUTF8;
									newLen = node->length;
									
									VMecabModel::keywordActionType keywordActionType = KEYWORD_ADD;
									
									if((oldPos + oldLen) != newPos)
									{
										oldPosId = 0;
									}
									
									keywordActionType = this->keywordActionTypeForPosIdPair(oldPosId, node->posid);
									
									switch (keywordActionType) {
											
										case KEYWORD_NO_ACTION:
											break;
											
										case KEYWORD_ADD:
											shouldAdd = TRUE;
											break;
											
										case KEYWORD_REPLACE:
											
											newPos = oldPos;
											newLen += oldLen;
											
											boundaries.pop_back();
											
											shouldAdd = TRUE;
											break;												
									}
									
							}
							
							if(shouldAdd){
								
								boundaries.push_back(std::pair<size_t, size_t>(newPos, newLen));
								
							}
							
							oldPosId = node->posid;
							oldPos = newPos;
							oldLen = newLen;
							
							node = node->next;	
							
						}
						
						inLattice->clear();	
						
					}
					
				}
				
			}
			
		}
		
	}
	
	if (boundaries.empty())
	{
		*outWords = NULL;
		*outCount = 0;
	}
	else
	{
		*outWords = (size_t *) malloc( sizeof( size_t) * boundaries.size() * 2);
		*outCount = boundaries.size();
		if (*outWords != NULL)
		{
			size_t *p = *outWords;
			for( std::vector<std::pair<size_t,size_t> >::iterator i = boundaries.begin() ; i != boundaries.end() ; ++i)
			{
				*p++ = i->first;
				*p++ = i->second;
			}
			ok = true;
		}
	}
	return ok;
}

//this is where we implement custom rules to create useful keywords 
VMecabModel::keywordActionType VMecabModel::keywordActionTypeForPosIdPair(unsigned short previousPosId, unsigned short currentPosId){
	
	
	//particles to not count as keywords
	//rules based on pos-id of juman dictionary
	
	if(
	   (currentPosId ==  2)	//shijishi,	fukushi-keitai
	   || (currentPosId ==  3)	//shijishi,	meishi-keitai
	   || (currentPosId ==  4)	//shijishi,	rentaishi-keitai
	   || (currentPosId ==  5)	//jyoshi,	kaku-jyoshi
	   || (currentPosId ==  6)	//jyoshi,	shu-jyoshi
	   || (currentPosId ==  7)	//jyoshi,	setsuzoku-jyoshi
	   || (currentPosId ==  8)	//jyoshi,	fuku-jyoshi
	   || (currentPosId ==  9)	//jyodoshi  
	   || (currentPosId == 23)	//tokushu,	kakko-hiraki
	   || (currentPosId == 24)	//tokushu,	kakko-owari
	   || (currentPosId == 25)	//tokushu,	kigou
	   || (currentPosId == 26)	//tokushu,	ku-ten
	   || (currentPosId == 27)	//tokushu,	ku-haku
	   || (currentPosId == 28)	//tokushu,	tou-ten  
	   || (currentPosId == 29)	//hanteishi	   
	   || (currentPosId == 12)	//settouji,	na-keiyoushi settouji	 
	   )	
	{
		return KEYWORD_NO_ACTION;
	}	
	
	//suffix particles
	
	if(
	   (currentPosId ==  15)	//setsubiji,	keiyoushi-sei-jyutsugo
	   || (currentPosId ==  16)	//setsubiji,	keiyoushi-sei-meishi
	   || (currentPosId ==  17)	//setsubiji,	doushi-sei
	   //	   || (currentPosId ==  18)	//setsubiji,	meishi-sei-jyutsugo
	   	   || (currentPosId ==  19)	//setsubiji,	meishi-sei-tokushu
	   //	   || (currentPosId ==  20)	//setsubiji,	meishi-sei-meishi-jyosuu
	   //	   || (currentPosId ==  21)	//setsubiji,	meishi-sei   
	   )	
	{
		if(
		   (previousPosId ==  2)	//shijishi,	fukushi-keitai
		   || (previousPosId ==  3)	//shijishi,	meishi-keitai
		   || (previousPosId ==  4)	//shijishi,	rentaishi-keitai
		   || (previousPosId ==  5)	//jyoshi,	kaku-jyoshi
		   || (previousPosId ==  6)	//jyoshi,	shu-jyoshi
		   || (previousPosId ==  7)	//jyoshi,	setsuzoku-jyoshi
		   || (previousPosId ==  8)	//jyoshi,	fuku-jyoshi
		   || (previousPosId ==  9)	//jyodoshi  
		   || (previousPosId == 23)	//tokushu,	kakko-hiraki
		   || (previousPosId == 24)	//tokushu,	kakko-owari
		   || (previousPosId == 25)	//tokushu,	kigou
		   || (previousPosId == 26)	//tokushu,	ku-ten
		   || (previousPosId == 27)	//tokushu,	ku-haku
		   || (previousPosId == 28)	//tokushu,	tou-ten  
		   || (previousPosId == 29)	//hanteishi	   
		   ){
			return KEYWORD_ADD;
		}else{
			return KEYWORD_REPLACE;
		}
		
	}
	
	//prefix particles
	
	if(
	   (currentPosId ==  31)	//meishi,	sa-hen meishi
	   || (currentPosId ==  32)	//meishi,	keishiki meishi
	   || (currentPosId ==  33)	//meishi,	koyuu meishi
	   || (currentPosId ==  34)	//meishi,	jisou meishi
	   || (currentPosId ==  39)	//meishi,	futsuu meishi
	   || (currentPosId ==  40)	//meishi,	fukushiteki meishi
	   )	
	{
		if(
		   (previousPosId ==  14)		//settouji,	meishi-settouji
		   || (previousPosId ==  12)	//settouji,	na-keiyoushi settouji	
		   ){
			return KEYWORD_REPLACE;
		}else{
			return KEYWORD_ADD;
		}
		
	}
	
	return KEYWORD_ADD;
	
}
