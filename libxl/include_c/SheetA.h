#ifndef LIBXL_SHEETA_H
#define LIBXL_SHEETA_H

#include "setup.h"
#include "handle.h"
#include "enum.h"

#ifdef __cplusplus
extern "C"
{
#endif

    XLAPI            int XLAPIENTRY xlSheetCellTypeA(SheetHandle handle, int row, int col);
    XLAPI            int XLAPIENTRY xlSheetIsFormulaA(SheetHandle handle, int row, int col);

    XLAPI   FormatHandle XLAPIENTRY xlSheetCellFormatA(SheetHandle handle, int row, int col);
    XLAPI           void XLAPIENTRY xlSheetSetCellFormatA(SheetHandle handle, int row, int col, FormatHandle format);

    XLAPI    const char* XLAPIENTRY xlSheetReadStrA(SheetHandle handle, int row, int col, FormatHandle* format);
    XLAPI            int XLAPIENTRY xlSheetWriteStrA(SheetHandle handle, int row, int col, const char* value, FormatHandle format);

    XLAPI         double XLAPIENTRY xlSheetReadNumA(SheetHandle handle, int row, int col, FormatHandle* format);
    XLAPI            int XLAPIENTRY xlSheetWriteNumA(SheetHandle handle, int row, int col, double value, FormatHandle format);

    XLAPI            int XLAPIENTRY xlSheetReadBoolA(SheetHandle handle, int row, int col, FormatHandle* format);
    XLAPI            int XLAPIENTRY xlSheetWriteBoolA(SheetHandle handle, int row, int col, int value, FormatHandle format);

    XLAPI            int XLAPIENTRY xlSheetReadBlankA(SheetHandle handle, int row, int col, FormatHandle* format);
    XLAPI            int XLAPIENTRY xlSheetWriteBlankA(SheetHandle handle, int row, int col, FormatHandle format);

    XLAPI    const char* XLAPIENTRY xlSheetReadFormulaA(SheetHandle handle, int row, int col, FormatHandle* format);
    XLAPI            int XLAPIENTRY xlSheetWriteFormulaA(SheetHandle handle, int row, int col, const char* value, FormatHandle format);

    XLAPI    const char* XLAPIENTRY xlSheetReadCommentA(SheetHandle handle, int row, int col);
    XLAPI           void XLAPIENTRY xlSheetWriteCommentA(SheetHandle handle, int row, int col, const char* value, const char* author, int width, int height);

    XLAPI            int XLAPIENTRY xlSheetIsDateA(SheetHandle handle, int row, int col);
    XLAPI            int XLAPIENTRY xlSheetReadErrorA(SheetHandle handle, int row, int col);

    XLAPI         double XLAPIENTRY xlSheetColWidthA(SheetHandle handle, int col);
    XLAPI         double XLAPIENTRY xlSheetRowHeightA(SheetHandle handle, int row);

    XLAPI            int XLAPIENTRY xlSheetSetColA(SheetHandle handle, int colFirst, int colLast, double width, FormatHandle format, int hidden);
    XLAPI            int XLAPIENTRY xlSheetSetRowA(SheetHandle handle, int row, double height, FormatHandle format, int hidden);

    XLAPI            int XLAPIENTRY xlSheetRowHiddenA(SheetHandle handle, int row);
    XLAPI            int XLAPIENTRY xlSheetSetRowHiddenA(SheetHandle handle, int row, int hidden);

    XLAPI            int XLAPIENTRY xlSheetColHiddenA(SheetHandle handle, int col);
    XLAPI            int XLAPIENTRY xlSheetSetColHiddenA(SheetHandle handle, int col, int hidden);

    XLAPI            int XLAPIENTRY xlSheetGetMergeA(SheetHandle handle, int row, int col, int* rowFirst, int* rowLast, int* colFirst, int* colLast);
    XLAPI            int XLAPIENTRY xlSheetSetMergeA(SheetHandle handle, int rowFirst, int rowLast, int colFirst, int colLast);
    XLAPI            int XLAPIENTRY xlSheetDelMergeA(SheetHandle handle, int row, int col);

    XLAPI            int XLAPIENTRY xlSheetPictureSizeA(SheetHandle handle);
    XLAPI            int XLAPIENTRY xlSheetGetPictureA(SheetHandle handle, int index, int* rowTop, int* colLeft, int* rowBottom, int* colRight,
                                                                                      int* width, int* height, int* offset_x, int* offset_y);

    XLAPI           void XLAPIENTRY xlSheetSetPictureA(SheetHandle handle, int row, int col, int pictureId, double scale, int offset_x, int offset_y);
    XLAPI           void XLAPIENTRY xlSheetSetPicture2A(SheetHandle handle, int row, int col, int pictureId, int width, int height, int offset_x, int offset_y);

    XLAPI            int XLAPIENTRY xlSheetGetHorPageBreakA(SheetHandle handle, int index);
    XLAPI            int XLAPIENTRY xlSheetGetHorPageBreakSizeA(SheetHandle handle);

    XLAPI            int XLAPIENTRY xlSheetGetVerPageBreakA(SheetHandle handle, int index);
    XLAPI            int XLAPIENTRY xlSheetGetVerPageBreakSizeA(SheetHandle handle);

    XLAPI            int XLAPIENTRY xlSheetSetHorPageBreakA(SheetHandle handle, int row, int pageBreak);
    XLAPI            int XLAPIENTRY xlSheetSetVerPageBreakA(SheetHandle handle, int col, int pageBreak);

    XLAPI           void XLAPIENTRY xlSheetSplitA(SheetHandle handle, int row, int col);

    XLAPI            int XLAPIENTRY xlSheetGroupRowsA(SheetHandle handle, int rowFirst, int rowLast, int collapsed);
    XLAPI            int XLAPIENTRY xlSheetGroupColsA(SheetHandle handle, int colFirst, int colLast, int collapsed);

    XLAPI            int XLAPIENTRY xlSheetGroupSummaryBelowA(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetGroupSummaryBelowA(SheetHandle handle, int below);

    XLAPI            int XLAPIENTRY xlSheetGroupSummaryRightA(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetGroupSummaryRightA(SheetHandle handle, int right);

    XLAPI           void XLAPIENTRY xlSheetClearA(SheetHandle handle, int rowFirst, int rowLast, int colFirst, int colLast);

    XLAPI            int XLAPIENTRY xlSheetInsertColA(SheetHandle handle, int colFirst, int colLast);
    XLAPI            int XLAPIENTRY xlSheetInsertRowA(SheetHandle handle, int rowFirst, int rowLast);
    XLAPI            int XLAPIENTRY xlSheetRemoveColA(SheetHandle handle, int colFirst, int colLast);
    XLAPI            int XLAPIENTRY xlSheetRemoveRowA(SheetHandle handle, int rowFirst, int rowLast);

    XLAPI            int XLAPIENTRY xlSheetCopyCellA(SheetHandle handle, int rowSrc, int colSrc, int rowDst, int colDst);

    XLAPI            int XLAPIENTRY xlSheetFirstRowA(SheetHandle handle);
    XLAPI            int XLAPIENTRY xlSheetLastRowA(SheetHandle handle);
    XLAPI            int XLAPIENTRY xlSheetFirstColA(SheetHandle handle);
    XLAPI            int XLAPIENTRY xlSheetLastColA(SheetHandle handle);

    XLAPI            int XLAPIENTRY xlSheetDisplayGridlinesA(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetDisplayGridlinesA(SheetHandle handle, int show);

    XLAPI            int XLAPIENTRY xlSheetPrintGridlinesA(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetPrintGridlinesA(SheetHandle handle, int print);

    XLAPI            int XLAPIENTRY xlSheetZoomA(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetZoomA(SheetHandle handle, int zoom);

    XLAPI            int XLAPIENTRY xlSheetPrintZoomA(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetPrintZoomA(SheetHandle handle, int zoom);

    XLAPI            int XLAPIENTRY xlSheetGetPrintFitA(SheetHandle handle, int* wPages, int* hPages);
    XLAPI           void XLAPIENTRY xlSheetSetPrintFitA(SheetHandle handle, int wPages, int hPages);

    XLAPI            int XLAPIENTRY xlSheetLandscapeA(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetLandscapeA(SheetHandle handle, int landscape);

    XLAPI            int XLAPIENTRY xlSheetPaperA(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetPaperA(SheetHandle handle, int paper);

    XLAPI    const char* XLAPIENTRY xlSheetHeaderA(SheetHandle handle);
    XLAPI            int XLAPIENTRY xlSheetSetHeaderA(SheetHandle handle, const char* header, double margin);
    XLAPI         double XLAPIENTRY xlSheetHeaderMarginA(SheetHandle handle);

    XLAPI    const char* XLAPIENTRY xlSheetFooterA(SheetHandle handle);
    XLAPI            int XLAPIENTRY xlSheetSetFooterA(SheetHandle handle, const char* footer, double margin);
    XLAPI         double XLAPIENTRY xlSheetFooterMarginA(SheetHandle handle);

    XLAPI            int XLAPIENTRY xlSheetHCenterA(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetHCenterA(SheetHandle handle, int hCenter);

    XLAPI            int XLAPIENTRY xlSheetVCenterA(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetVCenterA(SheetHandle handle, int vCenter);

    XLAPI         double XLAPIENTRY xlSheetMarginLeftA(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetMarginLeftA(SheetHandle handle, double margin);

    XLAPI         double XLAPIENTRY xlSheetMarginRightA(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetMarginRightA(SheetHandle handle, double margin);

    XLAPI         double XLAPIENTRY xlSheetMarginTopA(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetMarginTopA(SheetHandle handle, double margin);

    XLAPI         double XLAPIENTRY xlSheetMarginBottomA(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetMarginBottomA(SheetHandle handle, double margin);

    XLAPI            int XLAPIENTRY xlSheetPrintRowColA(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetPrintRowColA(SheetHandle handle, int print);

    XLAPI           void XLAPIENTRY xlSheetSetPrintRepeatRowsA(SheetHandle handle, int rowFirst, int rowLast);
    XLAPI           void XLAPIENTRY xlSheetSetPrintRepeatColsA(SheetHandle handle, int colFirst, int colLast);
    XLAPI           void XLAPIENTRY xlSheetSetPrintAreaA(SheetHandle handle, int rowFirst, int rowLast, int colFirst, int colLast);

    XLAPI           void XLAPIENTRY xlSheetClearPrintRepeatsA(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetClearPrintAreaA(SheetHandle handle);

    XLAPI            int XLAPIENTRY xlSheetGetNamedRangeA(SheetHandle handle, const char* name, int* rowFirst, int* rowLast, int* colFirst, int* colLast);
    XLAPI            int XLAPIENTRY xlSheetSetNamedRangeA(SheetHandle handle, const char* name, int rowFirst, int rowLast, int colFirst, int colLast);
    XLAPI            int XLAPIENTRY xlSheetDelNamedRangeA(SheetHandle handle, const char* name);

    XLAPI            int XLAPIENTRY xlSheetNamedRangeSizeA(SheetHandle handle);
    XLAPI    const char* XLAPIENTRY xlSheetNamedRangeA(SheetHandle handle, int index, int* rowFirst, int* rowLast, int* colFirst, int* colLast);

    XLAPI    const char* XLAPIENTRY xlSheetNameA(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetNameA(SheetHandle handle, const char* name);

    XLAPI            int XLAPIENTRY xlSheetProtectA(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetProtectA(SheetHandle handle, int protect);

    XLAPI            int XLAPIENTRY xlSheetHiddenA(SheetHandle handle);
    XLAPI            int XLAPIENTRY xlSheetSetHiddenA(SheetHandle handle, int hidden);

    XLAPI           void XLAPIENTRY xlSheetGetTopLeftViewA(SheetHandle handle, int* row, int* col);
    XLAPI           void XLAPIENTRY xlSheetSetTopLeftViewA(SheetHandle handle, int row, int col);

    XLAPI           void XLAPIENTRY xlSheetAddrToRowColA(SheetHandle handle, const char* addr, int* row, int* col, int* rowRelative, int* colRelative);
    XLAPI    const char* XLAPIENTRY xlSheetRowColToAddrA(SheetHandle handle, int row, int col, int rowRelative, int colRelative);

#ifdef __cplusplus
}
#endif

#endif
