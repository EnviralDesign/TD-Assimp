/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#pragma once

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <iostream>
#include <string>
#include <vector>
#include <array>

#include "SOP_CPlusPlusBase.h"
#include "DataAndTypes.h"

// To get more help about these functions, look at SOP_CPlusPlusBase.h
class TdAssimp : public SOP_CPlusPlusBase
{
public:

	TdAssimp(const OP_NodeInfo* info);

	virtual ~TdAssimp();

	virtual void	getGeneralInfo(SOP_GeneralInfo*, const OP_Inputs*, void* reserved1) override;

	virtual void	execute(SOP_Output*, const OP_Inputs*, void* reserved) override;


	virtual void executeVBO(SOP_VBOOutput* output, const OP_Inputs* inputs,
							void* reserved) override;


	virtual void getErrorString(OP_String* error, void* reserved1) override;
	
	virtual int32_t getNumInfoCHOPChans(void* reserved) override;

	virtual void getInfoCHOPChan(int index, OP_InfoCHOPChan* chan, void* reserved) override;

	virtual bool getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved) override;

	virtual void getInfoDATEntries(int32_t index, int32_t nEntries,
									OP_InfoDATEntries* entries,
									void* reserved) override;

	virtual void setupParameters(OP_ParameterManager* manager, void* reserved) override;
	virtual void pulsePressed(const char* name, void* reserved) override;

private:

	// example functions for generating a geometry, change them with any
	// fucntions and algorithm:

	void cubeGeometry(SOP_Output* output, float scale = 1.0f);

	void lineGeometry(SOP_Output* output);

	void triangleGeometry(SOP_Output* output);

	void cubeGeometryVBO(SOP_VBOOutput* output, float scale = 1.0f);

	void lineGeometryVBO(SOP_VBOOutput* output);

	void triangleGeometryVBO(SOP_VBOOutput* output);

	void particleGeometryVBO(SOP_VBOOutput* output);

	//// holder for generic data.
	//Position pos;
	//Vector vec;
	//Color col;
	//TexCoord tex;
	//std::string	myError;
	//std::array<double, 4> vertexTint;
	//float normal[3];
	//float tangent[3];
	//float bitangent[3];
	//int vtxOffset;
	//int triOffset;

	// We don't need to store this pointer, but we do for the example.
	// The OP_NodeInfo class store information about the node that's using
	// this instance of the class (like its name).
	const OP_NodeInfo*		myNodeInfo;

	// In this example this value will be incremented each time the execute()
	// function is called, then passes back to the SOP
	int32_t					myExecuteCount;


	double					myOffset;
	std::string             myChopChanName;
	float                   myChopChanVal;
	std::string             myChop;

	std::string             myDat;

	int						myNumVBOTexLayers;
};