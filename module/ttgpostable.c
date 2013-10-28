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

int LoadGPOSTable(TTGPOSTable *table, FT_Bytes gpos)
{
    table->header.Version = gpos[0] << 24 | gpos[1] << 16 | gpos[2] << 8 | gpos[3];
    if(table->header.Version != 0x00010000)
    {
        return -1;
    }
    table->header.ScriptList  = gpos[4] << 8 | gpos[5];
    table->header.FeatureList = gpos[6] << 8 | gpos[7];
    table->header.LookupList  = gpos[8] << 8 | gpos[9];
    return GPOSParse(table, &gpos[table->header.ScriptList], &gpos[table->header.FeatureList], &gpos[table->header.LookupList]);
}

int GPOSParse(TTGPOSTable *table, FT_Bytes scriptlist, FT_Bytes featurelist, FT_Bytes lookuplist)
{
    ParseScriptList(scriptlist, &table->ScriptList);
    ParseFeatureList(featurelist, &table->FeatureList);
    GPOSParseLookupList(lookuplist, &table->LookupList);
    return 0;
}

void GPOSParseLookupList(FT_Bytes raw, TGPOSLookupList *rec)
{
    int i;
    FT_Bytes sp = raw;
    rec->LookupCount = GetUInt16(&sp);
    if(rec->LookupCount <= 0)
    {
        rec->Lookup = NULL;
        return;
    }
    rec->Lookup = calloc(rec->LookupCount, sizeof(TGPOSLookup));
    for(i = 0; i < rec->LookupCount; i++)
    {
        uint16_t offset = GetUInt16(&sp);
        GPOSParseLookup(&raw[offset], &rec->Lookup[i]);
    }
}

void GPOSParseLookup(FT_Bytes raw, TGPOSLookup *rec)
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
    rec->SubTable = calloc(rec->SubTableCount, sizeof(TGPOSSinglePosFormat));

    if(rec->LookupType != 1)
        return;

    for(i = 0; i < rec->SubTableCount; i++)
    {
        uint16_t offset = GetUInt16(&sp);
        GPOSParseSinglePos(&raw[offset], &rec->SubTable[i]);
    }
}

void GPOSParseSinglePos(FT_Bytes raw, TGPOSSinglePosFormat *rec)
{
    int i, j;
    FT_Bytes sp = raw;
    uint16_t Format = GetUInt16(&sp);
    switch(Format)
    {
    case 1:
        rec->PosFormat = 1;
        break;
    case 2:
        rec->PosFormat = 2;
        break;
    default:
        rec->PosFormat = 0;
        return;
    }
    uint16_t offset = GetUInt16(&sp);
    uint16_t vf_tmp;
    ParseCoverage(&raw[offset], &rec->Coverage);
    rec->ValueFormat = GetUInt16(&sp);
    if(rec->PosFormat == 2)
    {
        rec->ValueCount = GetUInt16(&sp);
    }
    else
    {
        rec->ValueCount = 1;
    }
    rec->Value = calloc(rec->ValueCount, sizeof(TGPOSValueRecord));
    for(i = 0; i < rec->ValueCount; i++)
    {
        vf_tmp = rec->ValueFormat;
        rec->Value[i].XPlacement = vf_tmp & 0x0001 ? GetInt16(&sp) : 0;
        rec->Value[i].YPlacement = (vf_tmp >>= 1) & 0x0001 ? GetInt16(&sp) : 0;
        rec->Value[i].XAdvance = (vf_tmp >>= 1) & 0x0001 ? GetInt16(&sp) : 0;
        rec->Value[i].YAdvance = (vf_tmp >>= 1) & 0x0001 ? GetInt16(&sp) : 0;
        /* XxDevice is not supported */
        rec->Value[i].XPlaDevice = (vf_tmp >>= 1) & 0x0001 ? GetInt16(&sp) : 0;
        rec->Value[i].YPlaDevice = (vf_tmp >>= 1) & 0x0001 ? GetInt16(&sp) : 0;
        rec->Value[i].XAdvDevice = (vf_tmp >>= 1) & 0x0001 ? GetInt16(&sp) : 0;
        rec->Value[i].YAdvDevice = (vf_tmp >>= 1) & 0x0001 ? GetInt16(&sp) : 0;
        for(j = 0; j < 8; j++)
        {
            if((vf_tmp >>= 1) & 0x0001)
            {
                GetInt16(&sp);
            }
        }
    }
}

int GetHHalfPosInfo(TTGPOSTable *table, uint32_t glyphnum,
                   int16_t *XPlacement, int16_t *YPlacement, int16_t *XAdvance, int16_t *YAdvance)
{
    uint32_t tag[] = {
        (uint8_t)'h' << 24 |
        (uint8_t)'a' << 16 |
        (uint8_t)'l' <<  8 |
        (uint8_t)'t',
        0,
    };
    return GPOSSinglePosInfo(table, tag, glyphnum, XPlacement, YPlacement, XAdvance, YAdvance);
}

int GetVHalfPosInfo(TTGPOSTable *table, uint32_t glyphnum,
                   int16_t *XPlacement, int16_t *YPlacement, int16_t *XAdvance, int16_t *YAdvance)
{
    uint32_t tag[] = {
        (uint8_t)'v' << 24 |
        (uint8_t)'h' << 16 |
        (uint8_t)'a' <<  8 |
        (uint8_t)'l',
        0,
    };
    return GPOSSinglePosInfo(table, tag, glyphnum, XPlacement, YPlacement, XAdvance, YAdvance);
}

int GetHPropPosInfo(TTGPOSTable *table, uint32_t glyphnum,
                   int16_t *XPlacement, int16_t *YPlacement, int16_t *XAdvance, int16_t *YAdvance)
{
    uint32_t tag[] = {
        (uint8_t)'p' << 24 |
        (uint8_t)'a' << 16 |
        (uint8_t)'l' <<  8 |
        (uint8_t)'t',
        0,
    };
    return GPOSSinglePosInfo(table, tag, glyphnum, XPlacement, YPlacement, XAdvance, YAdvance);
}

int GetVPropPosInfo(TTGPOSTable *table, uint32_t glyphnum,
                   int16_t *XPlacement, int16_t *YPlacement, int16_t *XAdvance, int16_t *YAdvance)
{
    uint32_t tag[] = {
        (uint8_t)'v' << 24 |
        (uint8_t)'p' << 16 |
        (uint8_t)'a' <<  8 |
        (uint8_t)'l',
        0,
    };
    return GPOSSinglePosInfo(table, tag, glyphnum, XPlacement, YPlacement, XAdvance, YAdvance);
}

int GPOSSinglePosInfo(TTGPOSTable *table, uint32_t tag[], uint32_t glyphnum,
                      int16_t *XPlacement, int16_t *YPlacement, int16_t *XAdvance, int16_t *YAdvance)
{
    int i, j;
    if(!table->loaded)
    {
        printf("not loaded\n");
        return -1;
    }
    for(i = 0; tag[i] != 0; i++)
    {
        for(j = 0; j < table->FeatureList.FeatureCount; j++)
        {
            if(table->FeatureList.FeatureRecord[j].FeatureTag == tag[i])
            {
                if(GPOSSinglePosInfoSub(table, glyphnum, XPlacement, YPlacement, XAdvance, YAdvance, &table->FeatureList.FeatureRecord[j].Feature) == 0)
                {
                    return 0;
                }
            }
        }
    }
    return -1;
}

int GPOSSinglePosInfoSub(TTGPOSTable *table, uint32_t glyphnum,
                         int16_t *XPlacement, int16_t *YPlacement, int16_t *XAdvance, int16_t *YAdvance, TFeature *Feature)
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
            if(GPOSSinglePosInfoSub2(glyphnum, XPlacement, YPlacement, XAdvance, YAdvance, &table->LookupList.Lookup[index]) == 0)
            {
                return 0;
            }
        }
    }
    return -1;
}

int GPOSSinglePosInfoSub2(uint32_t glyphnum, int16_t *XPlacement, int16_t *YPlacement,
                                             int16_t *XAdvance, int16_t *YAdvance, TGPOSLookup *Lookup)
{
    int i, index;
    TGPOSSinglePosFormat *tbl;
    for(i = 0; i < Lookup->SubTableCount; i++)
    {
        switch(Lookup->SubTable[i].PosFormat)
        {
        case 1:
            tbl = &Lookup->SubTable[i];
            if(GetCoverageIndex(&tbl->Coverage, glyphnum) >= 0)
            {
                TGPOSValueRecord vals = tbl->Value[0];
                *XPlacement = vals.XPlacement;
                *YPlacement = vals.YPlacement;
                *XAdvance = vals.XAdvance;
                *YAdvance = vals.YAdvance;
                return 0;
            }
            break;
        case 2:
            tbl = &Lookup->SubTable[i];
            index = GetCoverageIndex(&tbl->Coverage, glyphnum);
            if(0 <= index && index < tbl->ValueCount)
            {
                TGPOSValueRecord vals = tbl->Value[index];
                *XPlacement = vals.XPlacement;
                *YPlacement = vals.YPlacement;
                *XAdvance = vals.XAdvance;
                *YAdvance = vals.YAdvance;
                return 0;
            }
            break;
        }
    }
    return -1;
}

void init_gpostable(TTGPOSTable *table)
{
    table->loaded = 0;
    table->ScriptList.ScriptCount = 0;
    table->ScriptList.ScriptRecord = NULL;
    table->FeatureList.FeatureCount = 0;
    table->FeatureList.FeatureRecord = NULL;
    table->LookupList.LookupCount = 0;
    table->LookupList.Lookup = NULL;
}

void free_gpostable(TTGPOSTable *table)
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
    TGPOSLookup *lup = table->LookupList.Lookup;
    for(i = 0; i < lup_cnt; i++)
    {
        int ls_cnt = lup[i].SubTableCount;
        TGPOSSinglePosFormat *subt = lup[i].SubTable;
        for(j = 0; j < ls_cnt; j++)
        {
            if(subt[j].Coverage.CoverageFormat == 1) {
                free(subt[j].Coverage.GlyphArray);
            } else if(subt[j].Coverage.CoverageFormat == 2) {
            	free(subt[j].Coverage.RangeRecord);
            }
            free(subt[j].Value);
        }
        free(subt);
    }
    free(lup);
}
