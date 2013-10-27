/*
Copyright 2012-2013, Koichi Akabe <vbkaisetsu@gmail.com>

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

int LoadGSUBTable(TTGSUBTable *table, FT_Bytes gsub)
{
    table->header.Version = gsub[0] << 24 | gsub[1] << 16 | gsub[2] << 8 | gsub[3];
    if(table->header.Version != 0x00010000)
    {
        return -1;
    }
    table->header.ScriptList  = gsub[4] << 8 | gsub[5];
    table->header.FeatureList = gsub[6] << 8 | gsub[7];
    table->header.LookupList  = gsub[8] << 8 | gsub[9];
    return GSUBParse(table, &gsub[table->header.ScriptList], &gsub[table->header.FeatureList], &gsub[table->header.LookupList]);
}

int GSUBParse(TTGSUBTable *table, FT_Bytes scriptlist, FT_Bytes featurelist, FT_Bytes lookuplist)
{
    ParseScriptList(scriptlist, &table->ScriptList);
    ParseFeatureList(featurelist, &table->FeatureList);
    GSUBParseLookupList(lookuplist, &table->LookupList);
    return 0;
}

void GSUBParseLookupList(FT_Bytes raw, TGSUBLookupList *rec)
{
    int i;
    FT_Bytes sp = raw;
    rec->LookupCount = GetUInt16(&sp);
    if(rec->LookupCount <= 0)
    {
        rec->Lookup = NULL;
        return;
    }
    rec->Lookup = calloc(rec->LookupCount, sizeof(TGSUBLookup));
    for(i = 0; i < rec->LookupCount; i++)
    {
        uint16_t offset = GetUInt16(&sp);
        GSUBParseLookup(&raw[offset], &rec->Lookup[i]);
    }
}

void GSUBParseLookup(FT_Bytes raw, TGSUBLookup *rec)
{
    int i;
    FT_Bytes sp = raw;
    rec->LookupType = GetUInt16(&sp);
    rec->LookupFlag = GetUInt16(&sp);
    rec->SubTableCount = GetUInt16(&sp);
    if(rec->SubTableCount <= 0)
    {
        rec->SubTable = NULL;
        return;
    }
    rec->SubTable = calloc(rec->SubTableCount, sizeof(TGSUBSingleSubstFormat));
    if(rec->LookupType != 1)
        return;
    for(i = 0; i < rec->SubTableCount; i++)
    {
        uint16_t offset = GetUInt16(&sp);
        GSUBParseSingleSubst(&raw[offset], &rec->SubTable[i]);
    }
}

void GSUBParseSingleSubst(FT_Bytes raw, TGSUBSingleSubstFormat *rec)
{
    FT_Bytes sp = raw;
    uint16_t Format = GetUInt16(&sp);
    switch(Format)
    {
    case 1:
        rec->SubstFormat = 1;
        GSUBParseSingleSubstFormat1(raw, rec);
        break;
    case 2:
        rec->SubstFormat = 2;
        GSUBParseSingleSubstFormat2(raw, rec);
        break;
    default:
        rec->SubstFormat = 0;
    }
}

void GSUBParseSingleSubstFormat1(FT_Bytes raw, TGSUBSingleSubstFormat *rec)
{
    FT_Bytes sp = raw;
    GetUInt16(&sp);
    uint16_t offset = GetUInt16(&sp);
    ParseCoverage(&raw[offset], &rec->Coverage);
    rec->DeltaGlyphID = GetInt16(&sp);
}

void GSUBParseSingleSubstFormat2(FT_Bytes raw, TGSUBSingleSubstFormat *rec)
{
    int i;
    FT_Bytes sp = raw;
    GetUInt16(&sp);
    uint16_t offset = GetUInt16(&sp);
    ParseCoverage(&raw[offset], &rec->Coverage);
    rec->GlyphCount = GetUInt16(&sp);
    if(rec->GlyphCount <= 0)
    {
        rec->Substitute = NULL;
        return;
    }
    rec->Substitute = calloc(rec->GlyphCount, sizeof(uint16_t));
    for(i = 0; i < rec->GlyphCount; i++)
    {
        rec->Substitute[i] = GetUInt16(&sp);
    }
}

int GetVerticalGlyph(TTGSUBTable *table, uint32_t glyphnum, uint32_t *vglyphnum)
{
    int i, j;
    uint32_t tag[] = {
        (uint8_t)'v' << 24 |
        (uint8_t)'r' << 16 |
        (uint8_t)'t' <<  8 |
        (uint8_t)'2',

        (uint8_t)'v' << 24 |
        (uint8_t)'e' << 16 |
        (uint8_t)'r' <<  8 |
        (uint8_t)'t',
    };
    if(!table->loaded)
    {
        return -1;
    }
    for(i = 0; i < 2; i++)
    {
        for(j = 0; j < table->FeatureList.FeatureCount; j++)
        {
            if(table->FeatureList.FeatureRecord[j].FeatureTag == tag[i])
            {
                if(GetVerticalGlyphSub(table, glyphnum, vglyphnum, &table->FeatureList.FeatureRecord[j].Feature) == 0)
                {
                    return 0;
                }
            }
        }
    }
    return -1;
}

int GetVerticalGlyphSub(TTGSUBTable *table, uint32_t glyphnum, uint32_t *vglyphnum, TFeature *Feature)
{
    int i, index;
    for(i = 0; i < Feature->LookupCount; i++)
    {
        index = Feature->LookupListIndex[i];
        if(index < 0 || table->LookupList.LookupCount < index)
        {
            continue;
        }
        if(table->LookupList.Lookup[index].LookupType == 1)
        {
            if(GetVerticalGlyphSub2(glyphnum, vglyphnum, &table->LookupList.Lookup[index]) == 0)
            {
                return 0;
            }
        }
    }
    return -1;
}

int GetVerticalGlyphSub2(uint32_t glyphnum, uint32_t *vglyphnum, TGSUBLookup *Lookup)
{
    int i, index;
    TGSUBSingleSubstFormat *tbl;
    for(i = 0; i < Lookup->SubTableCount; i++)
    {
        switch(Lookup->SubTable[i].SubstFormat)
        {
        case 1:
            tbl = &Lookup->SubTable[i];
            if(GetCoverageIndex(&tbl->Coverage, glyphnum) >= 0)
            {
                *vglyphnum = glyphnum + tbl->DeltaGlyphID;
                return 0;
            }
            break;
        case 2:
            tbl = &Lookup->SubTable[i];
            index = GetCoverageIndex(&tbl->Coverage, glyphnum);
            if(0 <= index && index < tbl->GlyphCount)
            {
                *vglyphnum = tbl->Substitute[index];
                return 0;
            }
            break;
        }
    }
    return -1;
}

void init_gsubtable(TTGSUBTable *table)
{
    table->loaded = 0;
    table->ScriptList.ScriptCount = 0;
    table->ScriptList.ScriptRecord = NULL;
    table->FeatureList.FeatureCount = 0;
    table->FeatureList.FeatureRecord = NULL;
    table->LookupList.LookupCount = 0;
    table->LookupList.Lookup = NULL;
}

void free_gsubtable(TTGSUBTable *table)
{
    if(table->loaded == 0)
    {
        return;
    }
    int i, j;
    int scr_cnt = table->ScriptList.ScriptCount;
    TScriptRecord *scr_rcd = table->ScriptList.ScriptRecord;
    for(i = 0; i < scr_cnt; i++)
    {
        int ls_cnt = scr_rcd[i].Script.LangSysCount;
        TLangSysRecord *ls_rcd = scr_rcd[i].Script.LangSysRecord;
        for(j = 0; j < ls_cnt; j++)
        {
            free(ls_rcd[j].LangSys.FeatureIndex);
        }
        free(ls_rcd);
    }
    free(scr_rcd);
    int ftr_cnt = table->FeatureList.FeatureCount;
    TFeatureRecord *ftr_rcd = table->FeatureList.FeatureRecord;
    for(i = 0; i < ftr_cnt; i++)
    {
        free(ftr_rcd[i].Feature.LookupListIndex);
    }
    free(ftr_rcd);
    int lup_cnt = table->LookupList.LookupCount;
    TGSUBLookup *lup = table->LookupList.Lookup;
    for(i = 0; i < lup_cnt; i++)
    {
        int ls_cnt = lup[i].SubTableCount;
        TGSUBSingleSubstFormat *subt = lup[i].SubTable;
        for(j = 0; j < ls_cnt; j++)
        {
            if(subt[j].Coverage.CoverageFormat == 1) {
                free(subt[j].Coverage.GlyphArray);
            } else if(subt[j].Coverage.CoverageFormat == 2) {
            	free(subt[j].Coverage.RangeRecord);
            }
            if(subt[j].SubstFormat == 2)
                free(subt[j].Substitute);
        }
        free(subt);
    }
    free(lup);
}

