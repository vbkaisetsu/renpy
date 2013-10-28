#ifndef TTGSUBTable_H
#define TTGSUBTable_H

#include <stdint.h>
#include <ft2build.h>
#include <freetype/ftotval.h>

/* common */
typedef struct
{
    uint32_t Version;
    uint16_t ScriptList;
    uint16_t FeatureList;
    uint16_t LookupList;
} tt_header;

typedef struct
{
    uint16_t LookupOrder;
    uint16_t ReqFeatureIndex;
    uint16_t FeatureCount;
    uint16_t *FeatureIndex;
} TLangSys;

typedef struct
{
    uint32_t LangSysTag;
    TLangSys LangSys;
} TLangSysRecord;

typedef struct
{
    uint16_t DefaultLangSys;
    uint16_t LangSysCount;
    TLangSysRecord *LangSysRecord;
} TScript;

typedef struct
{
    uint32_t ScriptTag;
    TScript Script;
} TScriptRecord;

typedef struct
{
    uint16_t ScriptCount;
    TScriptRecord *ScriptRecord;
} TScriptList;

typedef struct
{
    uint16_t FeatureParams;
    int LookupCount;
    uint16_t *LookupListIndex;
} TFeature;

typedef struct
{
    uint32_t FeatureTag;
    TFeature Feature;
} TFeatureRecord;

typedef struct
{
    int FeatureCount;
    TFeatureRecord *FeatureRecord;
} TFeatureList;

typedef struct
{
    uint16_t Start;
    uint16_t End;
    uint16_t StartCoverageIndex;
} TRangeRecord;

typedef struct
{
    uint16_t CoverageFormat;
    uint16_t GlyphCount;
    uint16_t *GlyphArray;
    uint16_t RangeCount;
    TRangeRecord *RangeRecord;
} TCoverageFormat;

/* GSUB */
typedef struct
{
    uint16_t SubstFormat;
    TCoverageFormat Coverage;
    int16_t DeltaGlyphID;
    uint16_t GlyphCount;
    uint16_t *Substitute;
} TGSUBSingleSubstFormat;

typedef struct
{
    uint16_t LookupType;
    uint16_t LookupFlag;
    uint16_t SubTableCount;
    TGSUBSingleSubstFormat *SubTable;
} TGSUBLookup;

typedef struct
{
    int LookupCount;
    TGSUBLookup *Lookup;    
} TGSUBLookupList;

typedef struct
{
    int loaded;
    tt_header header;
    TScriptList ScriptList;
    TFeatureList FeatureList;
    TGSUBLookupList LookupList;
} TTGSUBTable;

/* GPOS */
typedef struct
{
    int16_t XPlacement;
    int16_t YPlacement;
    int16_t XAdvance;
    int16_t YAdvance;
    int16_t XPlaDevice;
    int16_t YPlaDevice;
    int16_t XAdvDevice;
    int16_t YAdvDevice;
} TGPOSValueRecord;

typedef struct
{
    uint16_t PosFormat;
    TCoverageFormat Coverage;
    uint16_t ValueFormat;
    uint16_t ValueCount;
    TGPOSValueRecord *Value;
} TGPOSSinglePosFormat;

typedef struct
{
    uint16_t LookupType;
    uint16_t LookupFlag;
    uint16_t SubTableCount;
    TGPOSSinglePosFormat *SubTable;
} TGPOSLookup;

typedef struct
{
    int LookupCount;
    TGPOSLookup *Lookup;    
} TGPOSLookupList;

typedef struct
{
    int loaded;
    tt_header header;
    TScriptList ScriptList;
    TFeatureList FeatureList;
    TGPOSLookupList LookupList;
} TTGPOSTable;

/* common */
uint8_t GetUInt8(FT_Bytes *p);
int16_t GetInt16(FT_Bytes *p);
uint16_t GetUInt16(FT_Bytes *p);
int32_t GetInt32(FT_Bytes *p);
uint32_t GetUInt32(FT_Bytes *p);

void LoadOTTable(TTGSUBTable *gsubtable, TTGPOSTable *gpostable, FT_Face face);

void ParseScriptList(FT_Bytes raw, TScriptList *rec);
void ParseScript(FT_Bytes raw, TScript *rec);
void ParseLangSys(FT_Bytes raw, TLangSys *rec);

void ParseFeatureList(FT_Bytes raw, TFeatureList *rec);
void ParseFeature(FT_Bytes raw, TFeature *rec);

void ParseCoverage(FT_Bytes raw, TCoverageFormat *rec);
void ParseCoverageFormat1(FT_Bytes raw, TCoverageFormat *rec);
void ParseCoverageFormat2(FT_Bytes raw, TCoverageFormat *rec);

int GetCoverageIndex(TCoverageFormat *Coverage, uint32_t g);

/* GSUB */
int LoadGSUBTable(TTGSUBTable *table, FT_Bytes gsub);

int GSUBParse(TTGSUBTable *table, FT_Bytes scriptlist, FT_Bytes featurelist, FT_Bytes lookuplist);

void GSUBParseLookupList(FT_Bytes raw, TGSUBLookupList *rec);
void GSUBParseLookup(FT_Bytes raw, TGSUBLookup *rec);

void GSUBParseSingleSubst(FT_Bytes raw, TGSUBSingleSubstFormat *rec);
void GSUBParseSingleSubstFormat1(FT_Bytes raw, TGSUBSingleSubstFormat *rec);
void GSUBParseSingleSubstFormat2(FT_Bytes raw, TGSUBSingleSubstFormat *rec);

int GetVerticalGlyph(TTGSUBTable *table, uint32_t glyphnum, uint32_t *vglyphnum);
int GetVerticalGlyphSub(TTGSUBTable *table, uint32_t glyphnum, uint32_t *vglyphnum, TFeature *Feature);
int GetVerticalGlyphSub2(uint32_t glyphnum, uint32_t *vglyphnum, TGSUBLookup *Lookup);

void init_gsubtable(TTGSUBTable *table);
void free_gsubtable(TTGSUBTable *table);

/* GPOS */
int LoadGPOSTable(TTGPOSTable *table, FT_Bytes gpos);

int GPOSParse(TTGPOSTable *table, FT_Bytes scriptlist, FT_Bytes featurelist, FT_Bytes lookuplist);

void GPOSParseLookupList(FT_Bytes raw, TGPOSLookupList *rec);
void GPOSParseLookup(FT_Bytes raw, TGPOSLookup *rec);

void GPOSParseSinglePos(FT_Bytes raw, TGPOSSinglePosFormat *rec);

int GetHHalfPosInfo(TTGPOSTable *table, uint32_t glyphnum,
                    int16_t *XPlacement, int16_t *YPlacement, int16_t *XAdvance, int16_t *YAdvance);
int GetVHalfPosInfo(TTGPOSTable *table, uint32_t glyphnum,
                    int16_t *XPlacement, int16_t *YPlacement, int16_t *XAdvance, int16_t *YAdvance);
int GetHPropPosInfo(TTGPOSTable *table, uint32_t glyphnum,
                    int16_t *XPlacement, int16_t *YPlacement, int16_t *XAdvance, int16_t *YAdvance);
int GetVPropPosInfo(TTGPOSTable *table, uint32_t glyphnum,
                    int16_t *XPlacement, int16_t *YPlacement, int16_t *XAdvance, int16_t *YAdvance);

int GPOSSinglePosInfo(TTGPOSTable *table, uint32_t tag[], uint32_t glyphnum,
                      int16_t *XPlacement, int16_t *YPlacement, int16_t *XAdvance, int16_t *YAdvance);
int GPOSSinglePosInfoSub(TTGPOSTable *table, uint32_t glyphnum,
                         int16_t *XPlacement, int16_t *YPlacement, int16_t *XAdvance, int16_t *YAdvance, TFeature *Feature);
int GPOSSinglePosInfoSub2(uint32_t glyphnum, int16_t *XPlacement, int16_t *YPlacement,
                                             int16_t *XAdvance, int16_t *YAdvance, TGPOSLookup *Lookup);
void init_gpostable(TTGPOSTable *table);
void free_gpostable(TTGPOSTable *table);
#endif
