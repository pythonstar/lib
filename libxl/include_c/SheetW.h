#ifndef LIBXL_SHEETW_H
#define LIBXL_SHEETW_H

#include <stddef.h>
#include "setup.h"
#include "handle.h"
#include "enum.h"

#ifdef __cplusplus
extern "C"
{
#endif

    XLAPI            int XLAPIENTRY xlSheetCellTypeW(SheetHandle handle, int row, int col);
    XLAPI            int XLAPIENTRY xlSheetIsFormulaW(SheetHandle handle, int row, int col);

    XLAPI   FormatHandle XLAPIENTRY xlSheetCellFormatW(SheetHandle handle, int row, int col);
    XLAPI           void XLAPIENTRY xlSheetSetCellFormatW(SheetHandle handle, int row, int col, FormatHandle format);

    XLAPI const wchar_t* XLAPIENTRY xlSheetReadStrW(SheetHandle handle, int row, int col, FormatHandle* format);
    XLAPI            int XLAPIENTRY xlSheetWriteStrW(SheetHandle handle, int row, int col, const wchar_t* value, FormatHandle format);

    XLAPI         double XLAPIENTRY xlSheetReadNumW(SheetHandle handle, int row, int col, FormatHandle* format);
    XLAPI            int XLAPIENTRY xlSheetWriteNumW(SheetHandle handle, int row, int col, double value, FormatHandle format);

    XLAPI            int XLAPIENTRY xlSheetReadBoolW(SheetHandle handle, int row, int col, FormatHandle* format);
    XLAPI            int XLAPIENTRY xlSheetWriteBoolW(SheetHandle handle, int row, int col, int value, FormatHandle format);

    XLAPI            int XLAPIENTRY xlSheetReadBlankW(SheetHandle handle, int row, int col, FormatHandle* format);
    XLAPI            int XLAPIENTRY xlSheetWriteBlankW(SheetHandle handle, int row, int col, FormatHandle format);

    XLAPI const wchar_t* XLAPIENTRY xlSheetReadFormulaW(SheetHandle handle, int row, int col, FormatHandle* format);
    XLAPI            int XLAPIENTRY xlSheetWriteFormulaW(SheetHandle handle, int row, int col, const wchar_t* value, FormatHandle format);

    XLAPI const wchar_t* XLAPIENTRY xlSheetReadCommentW(SheetHandle handle, int row, int col);
    XLAPI           void XLAPIENTRY xlSheetWriteCommentW(SheetHandle handle, int row, int col, const wchar_t* value, const wchar_t* author, int width, int height);

    XLAPI            int XLAPIENTRY xlSheetIsDateW(SheetHandle handle, int row, int col);
    XLAPI            int XLAPIENTRY xlSheetReadErrorW(SheetHandle handle, int row, int col);

    XLAPI         double XLAPIENTRY xlSheetColWidthW(SheetHandle handle, int col);
    XLAPI         double XLAPIENTRY xlSheetRowHeightW(SheetHandle handle, int row);

    XLAPI            int XLAPIENTRY xlSheetSetColW(SheetHandle handle, int colFirst, int colLast, double width, FormatHandle format, int hidden);
    XLAPI            int XLAPIENTRY xlSheetSetRowW(SheetHandle handle, int row, double height, FormatHandle format, int hidden);

    XLAPI            int XLAPIENTRY xlSheetRowHiddenW(SheetHandle handle, int row);
    XLAPI            int XLAPIENTRY xlSheetSetRowHiddenW(SheetHandle handle, int row, int hidden);

    XLAPI            int XLAPIENTRY xlSheetColHiddenW(SheetHandle handle, int col);
    XLAPI            int XLAPIENTRY xlSheetSetColHiddenW(SheetHandle handle, int col, int hidden);

    XLAPI            int XLAPIENTRY xlSheetGetMergeW(SheetHandle handle, int row, int col, int* rowFirst, int* rowLast, int* colFirst, int* colLast);
    XLAPI            int XLAPIENTRY xlSheetSetMergeW(SheetHandle handle, int rowFirst, int rowLast, int colFirst, int colLast);
    XLAPI            int XLAPIENTRY xlSheetDelMergeW(SheetHandle handle, int row, int col);

    XLAPI            int XLAPIENTRY xlSheetPictureSizeW(SheetHandle handle);
    XLAPI            int XLAPIENTRY xlSheetGetPictureW(SheetHandle handle, int index, int* rowTop, int* colLeft, int* rowBottom, int* colRight,
                                                                                      int* width, int* height, int* offset_x, int* offset_y);

    XLAPI           void XLAPIENTRY xlSheetSetPictureW(SheetHandle handle, int row, int col, int pictureId, double scale, int offset_x, int offset_y);
    XLAPI           void XLAPIENTRY xlSheetSetPicture2W(SheetHandle handle, int row, int col, int pictureId, int width, int height, int offset_x, int offset_y);

    XLAPI            int XLAPIENTRY xlSheetGetHorPageBreakW(SheetHandle handle, int index);
    XLAPI            int XLAPIENTRY xlSheetGetHorPageBreakSizeW(SheetHandle handle);

    XLAPI            int XLAPIENTRY xlSheetGetVerPageBreakW(SheetHandle handle, int index);
    XLAPI            int XLAPIENTRY xlSheetGetVerPageBreakSizeW(SheetHandle handle);

    XLAPI            int XLAPIENTRY xlSheetSetHorPageBreakW(SheetHandle handle, int row, int pageBreak);
    XLAPI            int XLAPIENTRY xlSheetSetVerPageBreakW(SheetHandle handle, int col, int pageBreak);

    XLAPI           void XLAPIENTRY xlSheetSplitW(SheetHandle handle, int row, int col);

    XLAPI            int XLAPIENTRY xlSheetGroupRowsW(SheetHandle handle, int rowFirst, int rowLast, int collapsed);
    XLAPI            int XLAPIENTRY xlSheetGroupColsW(SheetHandle handle, int colFirst, int colLast, int collapsed);

    XLAPI            int XLAPIENTRY xlSheetGroupSummaryBelowW(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetGroupSummaryBelowW(SheetHandle handle, int below);

    XLAPI            int XLAPIENTRY xlSheetGroupSummaryRightW(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetGroupSummaryRightW(SheetHandle handle, int right);

    XLAPI           void XLAPIENTRY xlSheetClearW(SheetHandle handle, int rowFirst, int rowLast, int colFirst, int colLast);

    XLAPI            int XLAPIENTRY xlSheetInsertColW(SheetHandle handle, int colFirst, int colLast);
    XLAPI            int XLAPIENTRY xlSheetInsertRowW(SheetHandle handle, int rowFirst, int rowLast);
    XLAPI            int XLAPIENTRY xlSheetRemoveColW(SheetHandle handle, int colFirst, int colLast);
    XLAPI            int XLAPIENTRY xlSheetRemoveRowW(SheetHandle handle, int rowFirst, int rowLast);

    XLAPI            int XLAPIENTRY xlSheetCopyCellW(SheetHandle handle, int rowSrc, int colSrc, int rowDst, int colDst);

    XLAPI            int XLAPIENTRY xlSheetFirstRowW(SheetHandle handle);
    XLAPI            int XLAPIENTRY xlSheetLastRowW(SheetHandle handle);
    XLAPI            int XLAPIENTRY xlSheetFirstColW(SheetHandle handle);
    XLAPI            int XLAPIENTRY xlSheetLastColW(SheetHandle handle);

    XLAPI            int XLAPIENTRY xlSheetDisplayGridlinesW(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetDisplayGridlinesW(SheetHandle handle, int show);

    XLAPI            int XLAPIENTRY xlSheetPrintGridlinesW(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetPrintGridlinesW(SheetHandle handle, int print);

    XLAPI            int XLAPIENTRY xlSheetZoomW(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetZoomW(SheetHandle handle, int zoom);

    XLAPI            int XLAPIENTRY xlSheetPrintZoomW(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetPrintZoomW(SheetHandle handle, int zoom);

    XLAPI            int XLAPIENTRY xlSheetGetPrintFitW(SheetHandle handle, int* wPages, int* hPages);
    XLAPI           void XLAPIENTRY xlSheetSetPrintFitW(SheetHandle handle, int wPages, int hPages);

    XLAPI            int XLAPIENTRY xlSheetLandscapeW(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetLandscapeW(SheetHandle handle, int landscape);

    XLAPI            int XLAPIENTRY xlSheetPaperW(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetPaperW(SheetHandle handle, int paper);

    XLAPI const wchar_t* XLAPIENTRY xlSheetHeaderW(SheetHandle handle);
    XLAPI            int XLAPIENTRY xlSheetSetHeaderW(SheetHandle handle, const wchar_t* header, double margin);
    XLAPI         double XLAPIENTRY xlSheetHeaderMarginW(SheetHandle handle);

    XLAPI const wchar_t* XLAPIENTRY xlSheetFooterW(SheetHandle handle);
    XLAPI            int XLAPIENTRY xlSheetSetFooterW(SheetHandle handle, const wchar_t* footer, double margin);
    XLAPI         double XLAPIENTRY xlSheetFooterMarginW(SheetHandle handle);

    XLAPI            int XLAPIENTRY xlSheetHCenterW(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetHCenterW(SheetHandle handle, int hCenter);

    XLAPI            int XLAPIENTRY xlSheetVCenterW(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetVCenterW(SheetHandle handle, int vCenter);

    XLAPI         double XLAPIENTRY xlSheetMarginLeftW(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetMarginLeftW(SheetHandle handle, double margin);

    XLAPI         double XLAPIENTRY xlSheetMarginRightW(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetMarginRightW(SheetHandle handle, double margin);

    XLAPI         double XLAPIENTRY xlSheetMarginTopW(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetMarginTopW(SheetHandle handle, double margin);

    XLAPI         double XLAPIENTRY xlSheetMarginBottomW(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetMarginBottomW(SheetHandle handle, double margin);

    XLAPI            int XLAPIENTRY xlSheetPrintRowColW(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetPrintRowColW(SheetHandle handle, int print);

    XLAPI           void XLAPIENTRY xlSheetSetPrintRepeatRowsW(SheetHandle handle, int rowFirst, int rowLast);
    XLAPI           void XLAPIENTRY xlSheetSetPrintRepeatColsW(SheetHandle handle, int colFirst, int colLast);
    XLAPI           void XLAPIENTRY xlSheetSetPrintAreaW(SheetHandle handle, int rowFirst, int rowLast, int colFirst, int colLast);

    XLAPI           void XLAPIENTRY xlSheetClearPrintRepeatsW(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetClearPrintAreaW(SheetHandle handle);

    XLAPI            int XLAPIENTRY xlSheetGetNamedRangeW(SheetHandle handle, const wchar_t* name, int* rowFirst, int* rowLast, int* colFirst, int* colLast);
    XLAPI            int XLAPIENTRY xlSheetSetNamedRangeW(SheetHandle handle, const wchar_t* name, int rowFirst, int rowLast, int colFirst, int colLast);
    XLAPI            int XLAPIENTRY xlSheetDelNamedRangeW(SheetHandle handle, const wchar_t* name);

    XLAPI            int XLAPIENTRY xlSheetNamedRangeSizeW(SheetHandle handle);
    XLAPI const wchar_t* XLAPIENTRY xlSheetNamedRangeW(SheetHandle handle, int index, int* rowFirst, int* rowLast, int* colFirst, int* colLast);

    XLAPI const wchar_t* XLAPIENTRY xlSheetNameW(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetNameW(SheetHandle handle, const wchar_t* name);

    XLAPI            int XLAPIENTRY xlSheetProtectW(SheetHandle handle);
    XLAPI           void XLAPIENTRY xlSheetSetProtectW(SheetHandle handle, int protect);

    XLAPI            int XLAPIENTRY xlSheetHiddenW(SheetHandle handle);
    XLAPI            int XLAPIENTRY xlSheetSetHiddenW(SheetHandle handle, int hidden);

    XLAPI           void XLAPIENTRY xlSheetGetTopLeftViewW(SheetHandle handle, int* row, int* col);
    XLAPI           void XLAPIENTRY xlSheetSetTopLeftViewW(SheetHandle handle, int row, int col);

    XLAPI           void XLAPIENTRY xlSheetAddrToRowColW(SheetHandle handle, const wchar_t* addr, int* row, int* col, int* rowRelative, int* colRelative);
    XLAPI const wchar_t* XLAPIENTRY xlSheetRowColToAddrW(SheetHandle handle, int row, int col, int rowRelative, int colRelative);

#ifdef __cplusplus
}
#endif

#endif
