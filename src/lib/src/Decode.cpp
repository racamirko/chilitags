/*******************************************************************************
*   Copyright 2013 EPFL                                                        *
*                                                                              *
*   This file is part of chilitags.                                            *
*                                                                              *
*   Chilitags is free software: you can redistribute it and/or modify          *
*   it under the terms of the Lesser GNU General Public License as             *
*   published by the Free Software Foundation, either version 3 of the         *
*   License, or (at your option) any later version.                            *
*                                                                              *
*   Chilitags is distributed in the hope that it will be useful,               *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*   GNU Lesser General Public License for more details.                        *
*                                                                              *
*   You should have received a copy of the GNU Lesser General Public License   *
*   along with Chilitags.  If not, see <http://www.gnu.org/licenses/>.         *
*******************************************************************************/

#include "Decode.hpp"

namespace {
const int scDataSize = 6;
}

chilitags::Decode::Decode(
        const unsigned char *const *pMatrix,
        Codec &pCodec) :
	mMatrix(pMatrix),
	mMatrix90(new unsigned char[scDataSize*scDataSize]),
	mMatrix180(new unsigned char[scDataSize*scDataSize]),
	mMatrix270(new unsigned char[scDataSize*scDataSize]),
	mCodec(pCodec),
	mDecodedTag(-1),
	mOrientation(-1)
{
}

chilitags::Decode::~Decode()
{
	delete[] mMatrix270;
	delete[] mMatrix180;
	delete[] mMatrix90;
}


void chilitags::Decode::run()
{
	const unsigned char *tMatrix = *mMatrix;
	for (int i = 0; i < scDataSize; ++i)
	{
		for (int j = 0; j < scDataSize; ++j)
		{
			unsigned char tBit = tMatrix[i*scDataSize + j];
			mMatrix90 [(scDataSize-1-j)*scDataSize +               i ] = tBit;
			mMatrix180[(scDataSize-1-i)*scDataSize + (scDataSize-1-j)] = tBit;
			mMatrix270[              j *scDataSize + (scDataSize-1-i)] = tBit;
		}
	}

	tag_info_t *tResult = 0;
	if (mCodec.decode(tMatrix, &tResult)) mOrientation = 0;
	else if (mCodec.decode(mMatrix90, &tResult)) mOrientation = 1;
	else if (mCodec.decode(mMatrix180, &tResult)) mOrientation = 2;
	else if (mCodec.decode(mMatrix270, &tResult)) mOrientation = 3;

	mDecodedTag = tResult ? tResult->id : -1;
}