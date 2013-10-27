/*
Copyright 2012-2013 Koichi Akabe <vbkaisetsu@gmail.com>

The original C++ source is distributed under the PD-like license.
For more details, see:
     http://higambana.ashigaru.jp/

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdlib.h>
#include "ttottable.h"

uint8_t GetUInt8(FT_Bytes *p)
{
    uint8_t ret = (*p)[0];
    *p += 1;
    return ret;
}

int16_t GetInt16(FT_Bytes *p)
{
    uint16_t ret = (*p)[0] << 8 | (*p)[1];
    *p += 2;
    return *(int16_t*)&ret;
}

uint16_t GetUInt16(FT_Bytes *p)
{
    uint16_t ret = (*p)[0] << 8 | (*p)[1];
    *p += 2;
    return ret;
}

int32_t GetInt32(FT_Bytes *p)
{
    uint32_t ret = (*p)[0] << 24 | (*p)[1] << 16 | (*p)[2] << 8 | (*p)[3];
    *p += 4;
    return *(int32_t*)&ret;
}

uint32_t GetUInt32(FT_Bytes *p)
{
    uint32_t ret = (*p)[0] << 24 | (*p)[1] << 16 | (*p)[2] << 8 | (*p)[3];
    *p += 4;
    return ret;
}

void ParseScriptList(FT_Bytes raw, TScriptList *rec)
{
    int i;
    FT_Bytes sp = raw;
    rec->ScriptCount = GetUInt16(&sp);
    if(rec->ScriptCount <= 0)
    {
        rec->ScriptRecord = NULL;
        return;    
    }
    rec->ScriptRecord = calloc(rec->ScriptCount, sizeof(TScriptRecord));
    for(i = 0; i < rec->ScriptCount; i++)
    {
        rec->ScriptRecord[i].ScriptTag = GetUInt32(&sp);
        uint16_t offset = GetUInt16(&sp);
        ParseScript(&raw[offset], &rec->ScriptRecord[i].Script);
    }
}

void ParseScript(FT_Bytes raw, TScript *rec)
{
    int i;
    FT_Bytes sp = raw;
    rec->DefaultLangSys = GetUInt16(&sp);
    rec->LangSysCount = GetUInt16(&sp);
    if(rec->LangSysCount <= 0)
    {
        rec->LangSysRecord = NULL;
        return;
    }
    rec->LangSysRecord = calloc(rec->LangSysCount, sizeof(TLangSysRecord));
    for(i = 0; i < rec->LangSysCount; i++)
    {
        rec->LangSysRecord[i].LangSysTag = GetUInt32(&sp);
        uint16_t offset = GetUInt16(&sp);
        ParseLangSys(&raw[offset], &rec->LangSysRecord[i].LangSys);
    }
}

void ParseLangSys(FT_Bytes raw, TLangSys *rec)
{
    FT_Bytes sp = raw;
    rec->LookupOrder = GetUInt16(&sp);
    rec->ReqFeatureIndex = GetUInt16(&sp);
    rec->FeatureCount = GetUInt16(&sp);
    if(rec->FeatureCount <= 0)
        return;
    rec->FeatureIndex = calloc(rec->FeatureCount, sizeof(uint16_t));
}

void ParseFeatureList(FT_Bytes raw, TFeatureList *rec)
{
    int i;
    FT_Bytes sp = raw;
    rec->FeatureCount = GetUInt16(&sp);
    if(rec->FeatureCount <= 0)
    {
        rec->FeatureRecord = NULL;
        return;
    }
    rec->FeatureRecord = calloc(rec->FeatureCount, sizeof(TFeatureRecord));
    for(i = 0; i < rec->FeatureCount; i++)
    {
        rec->FeatureRecord[i].FeatureTag = GetUInt32(&sp);
        uint16_t offset = GetUInt16(&sp);
        ParseFeature(&raw[offset], &rec->FeatureRecord[i].Feature);
    }
}

void ParseFeature(FT_Bytes raw, TFeature *rec)
{
    int i;
    FT_Bytes sp = raw;
    rec->FeatureParams = GetUInt16(&sp);
    rec->LookupCount = GetUInt16(&sp);
    if(rec->LookupCount <= 0)
    {
        return;
    }
    rec->LookupListIndex = calloc(rec->LookupCount, sizeof(uint16_t));
    for(i = 0;i < rec->LookupCount; i++)
    {
        rec->LookupListIndex[i] = GetUInt16(&sp);
    }
}

void ParseCoverage(FT_Bytes raw, TCoverageFormat *rec)
{
    FT_Bytes sp = raw;
    uint16_t Format = GetUInt16(&sp);
    switch(Format)
    {
    case 1:
        rec->CoverageFormat = 1;
        ParseCoverageFormat1(raw, rec);
        break;
    case 2:
        rec->CoverageFormat = 2;
        ParseCoverageFormat2(raw, rec);
        break;
    default:
        rec->CoverageFormat = 0;
    }
}

void ParseCoverageFormat1(FT_Bytes raw, TCoverageFormat *rec)
{
    int i;
    FT_Bytes sp = raw;
    GetUInt16(&sp);
    rec->GlyphCount = GetUInt16(&sp);
    if(rec->GlyphCount <= 0)
    {
        rec->GlyphArray = NULL;
        return;
    }
    rec->GlyphArray = calloc(rec->GlyphCount, sizeof(uint16_t));
    for(i = 0; i < rec->GlyphCount; i++)
    {
        rec->GlyphArray[i] = GetUInt16(&sp);
    }
}

void ParseCoverageFormat2(FT_Bytes raw, TCoverageFormat *rec)
{
    int i;
    FT_Bytes sp = raw;
    GetUInt16(&sp);
    rec->RangeCount = GetUInt16(&sp);
    if(rec->RangeCount <= 0)
    {
        rec->RangeRecord = NULL;
        return;
    }
    rec->RangeRecord = calloc(rec->RangeCount, sizeof(TRangeRecord));
    for(i = 0; i < rec->RangeCount; i++)
    {
        rec->RangeRecord[i].Start = GetUInt16(&sp);
        rec->RangeRecord[i].End = GetUInt16(&sp);
        rec->RangeRecord[i].StartCoverageIndex = GetUInt16(&sp);
    }
}

int GetCoverageIndex(TCoverageFormat *Coverage, uint32_t g)
{
    int i;
    switch(Coverage->CoverageFormat)
    {
    case 1:
        for(i = 0; i < Coverage->GlyphCount; i++)
        {
            if((uint32_t)Coverage->GlyphArray[i] == g)
            {
                return i;
            }
        }
        return -1;
    case 2:
        for(i = 0; i < Coverage->RangeCount; i++)
        {
            uint32_t s = Coverage->RangeRecord[i].Start;
            uint32_t e = Coverage->RangeRecord[i].End;
            uint32_t si = Coverage->RangeRecord[i].StartCoverageIndex;
            if(si + s <= g && g <= si + e)
            {
                return si + g - s;
            }
        }
        return -1;
    }
    return -1;
}

/* LoadOTTable */
void LoadOTTable(TTGSUBTable *gsubtable, TTGPOSTable *gpostable, FT_Face face)
{
    FT_Bytes base = NULL;
    FT_Bytes gdef = NULL;
    FT_Bytes gpos = NULL;
    FT_Bytes gsub = NULL;
    FT_Bytes jstf = NULL;
    FT_OpenType_Validate(face, FT_VALIDATE_GSUB | FT_VALIDATE_GPOS, &base, &gdef, &gpos, &gsub, &jstf);

    if(gsub != NULL)
    {
        gsubtable->loaded = !LoadGSUBTable(gsubtable, gsub);
        FT_OpenType_Free(face, gsub);
    }
    else
    {
        gsubtable->loaded = 0;
    }

    if(gpos != NULL)
    {
        gpostable->loaded = !LoadGPOSTable(gpostable, gpos);
        FT_OpenType_Free(face, gpos);
    }
    else
    {
        gpostable->loaded = 0;
    }
}
