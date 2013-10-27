from freetype cimport *

cdef extern from "stdint.h":
    ctypedef signed short int16_t
    ctypedef unsigned short uint16_t
    ctypedef signed long int32_t
    ctypedef unsigned long uint32_t

cdef extern from "ttottable.h":

    # common
    ctypedef struct tt_header:
        uint32_t Version
        uint16_t ScriptList
        uint16_t FeatureList
        uint16_t LookupList

    ctypedef struct TLangSys:
        uint16_t LookupOrder
        uint16_t ReqFeatureIndex
        uint16_t FeatureCount
        uint16_t *FeatureIndex

    ctypedef struct TLangSysRecord:
        uint32_t LangSysTag
        TLangSys LangSys

    ctypedef struct TScript:
        uint16_t DefaultLangSys
        uint16_t LangSysCount
        TLangSysRecord *LangSysRecord

    ctypedef struct TScriptRecord:
        uint32_t ScriptTag
        TScript Script

    ctypedef struct TScriptList:
        uint16_t ScriptCount
        TScriptRecord *ScriptRecord

    ctypedef struct TFeature:
        uint16_t FeatureParams
        int LookupCount
        uint16_t *LookupListIndex

    ctypedef struct TFeatureRecord:
        uint32_t FeatureTag
        TFeature Feature

    ctypedef struct TFeatureList:
        int FeatureCount
        TFeatureRecord *FeatureRecord

    ctypedef struct TRangeRecord:
        uint16_t Start
        uint16_t End
        uint16_t StartCoverageIndex

    ctypedef struct TCoverageFormat:
        uint16_t CoverageFormat
        uint16_t GlyphCount
        uint16_t *GlyphArray
        uint16_t RangeCount
        TRangeRecord *RangeRecord

    # GSUB
    ctypedef struct TGSUBSingleSubstFormat:
        uint16_t SubstFormat
        TCoverageFormat Coverage
        int16_t DeltaGlyphID
        uint16_t GlyphCount
        uint16_t *Substitute

    ctypedef struct TGSUBLookup:
        uint16_t LookupType
        uint16_t LookupFlag
        uint16_t SubTableCount
        TGSUBSingleSubstFormat *SubTable

    ctypedef struct TGSUBLookupList:
        int LookupCount
        TGSUBLookup *Lookup

    ctypedef struct TTGSUBTable:
        int loaded
        tt_header header
        TScriptList ScriptList
        TFeatureList FeatureList
        TGSUBLookupList LookupList
    
    # GPOS
    ctypedef struct TGPOSValueRecord:
        int16_t XPlacement
        int16_t YPlacement
        int16_t XAdvance
        int16_t YAdvance
        int16_t XPlaDevice
        int16_t YPlaDevice
        int16_t XAdvDevice
        int16_t YAdvDevice
    
    ctypedef struct TGPOSSinglePosFormat:
        uint16_t PosFormat
        TCoverageFormat Coverage
        uint16_t ValueFormat
        uint16_t ValueCount
        TGPOSValueRecord *Value
        
    ctypedef struct TGPOSLookup:
        uint16_t LookupType
        uint16_t LookupFlag
        uint16_t SubTableCount
        TGPOSSinglePosFormat *SubTable
        
    ctypedef struct TGPOSLookupList:
        int LookupCount
        TGPOSLookup *Lookup
        
    ctypedef struct TTGPOSTable:
        int loaded
        tt_header header
        TScriptList ScriptList
        TFeatureList FeatureList
        TGPOSLookupList LookupList

    void LoadOTTable(TTGSUBTable *gsubtable, TTGPOSTable *gpostable, FT_Face face)

    int GetVerticalGlyph(TTGSUBTable *table, uint32_t glyphnum, uint32_t *vglyphnum)
    void init_gsubtable(TTGSUBTable *table)
    void free_gsubtable(TTGSUBTable *table)

    int GetHalfPosInfo(TTGPOSTable *table, uint32_t glyphnum, int16_t *XPlacement, int16_t *YPlacement, int16_t *XAdvance, int16_t *YAdvance)
    int GetPropPosInfo(TTGPOSTable *table, uint32_t glyphnum, int16_t *XPlacement, int16_t *YPlacement, int16_t *XAdvance, int16_t *YAdvance)
    void init_gpostable(TTGPOSTable *table)
    void free_gpostable(TTGPOSTable *table)
