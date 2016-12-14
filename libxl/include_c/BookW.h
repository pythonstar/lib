#ifndef LIBXL_BOOKW_H
#define LIBXL_BOOKW_H

#include <stddef.h>
#include "setup.h"
#include "handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    XLAPI     BookHandle XLAPIENTRY xlCreateBookCW();
    XLAPI     BookHandle XLAPIENTRY xlCreateXMLBookCW();

    XLAPI            int XLAPIENTRY xlBookLoadW(BookHandle handle, const wchar_t* filename);
    XLAPI            int XLAPIENTRY xlBookSaveW(BookHandle handle, const wchar_t* filename);

    XLAPI            int XLAPIENTRY xlBookLoadRawW(BookHandle handle, const char* data, unsigned size);
    XLAPI            int XLAPIENTRY xlBookSaveRawW(BookHandle handle, const char** data, unsigned* size);

    XLAPI    SheetHandle XLAPIENTRY xlBookAddSheetW(BookHandle handle, const wchar_t* name, SheetHandle initSheet);
    XLAPI    SheetHandle XLAPIENTRY xlBookInsertSheetW(BookHandle handle, int index, const wchar_t* name, SheetHandle initSheet);
    XLAPI    SheetHandle XLAPIENTRY xlBookGetSheetW(BookHandle handle, int index);
    XLAPI            int XLAPIENTRY xlBookSheetTypeW(BookHandle handle, int index);
    XLAPI            int XLAPIENTRY xlBookDelSheetW(BookHandle handle, int index);
    XLAPI            int XLAPIENTRY xlBookSheetCountW(BookHandle handle);

    XLAPI   FormatHandle XLAPIENTRY xlBookAddFormatW(BookHandle handle, FormatHandle initFormat);
    XLAPI     FontHandle XLAPIENTRY xlBookAddFontW(BookHandle handle, FontHandle initFont);
    XLAPI            int XLAPIENTRY xlBookAddCustomNumFormatW(BookHandle handle, const wchar_t* customNumFormat);
    XLAPI const wchar_t* XLAPIENTRY xlBookCustomNumFormatW(BookHandle handle, int fmt);

    XLAPI   FormatHandle XLAPIENTRY xlBookFormatW(BookHandle handle, int index);
    XLAPI            int XLAPIENTRY xlBookFormatSizeW(BookHandle handle);

    XLAPI     FontHandle XLAPIENTRY xlBookFontW(BookHandle handle, int index);
    XLAPI            int XLAPIENTRY xlBookFontSizeW(BookHandle handle);

    XLAPI         double XLAPIENTRY xlBookDatePackW(BookHandle handle, int year, int month, int day, int hour, int min, int sec, int msec);
    XLAPI            int XLAPIENTRY xlBookDateUnpackW(BookHandle handle, double value, int* year, int* month, int* day, int* hour, int* min, int* sec, int* msec);

    XLAPI            int XLAPIENTRY xlBookColorPackW(BookHandle handle, int red, int green, int blue);
    XLAPI           void XLAPIENTRY xlBookColorUnpackW(BookHandle handle, int color, int* red, int* green, int* blue);

    XLAPI            int XLAPIENTRY xlBookActiveSheetW(BookHandle handle);
    XLAPI           void XLAPIENTRY xlBookSetActiveSheetW(BookHandle handle, int index);

    XLAPI            int XLAPIENTRY xlBookPictureSizeW(BookHandle handle);
    XLAPI            int XLAPIENTRY xlBookGetPictureW(BookHandle handle, int index, const char** data, unsigned* size);

    XLAPI            int XLAPIENTRY xlBookAddPictureW(BookHandle handle, const wchar_t* filename);
    XLAPI            int XLAPIENTRY xlBookAddPicture2W(BookHandle handle, const char* data, unsigned size);

    XLAPI const wchar_t* XLAPIENTRY xlBookDefaultFontW(BookHandle handle, int* fontSize);
    XLAPI           void XLAPIENTRY xlBookSetDefaultFontW(BookHandle handle, const wchar_t* fontName, int fontSize);

    XLAPI            int XLAPIENTRY xlBookRefR1C1W(BookHandle handle);
    XLAPI           void XLAPIENTRY xlBookSetRefR1C1W(BookHandle handle, int refR1C1);

    XLAPI           void XLAPIENTRY xlBookSetKeyW(BookHandle handle, const wchar_t* name, const wchar_t* key);

    XLAPI            int XLAPIENTRY xlBookRgbModeW(BookHandle handle);
    XLAPI           void XLAPIENTRY xlBookSetRgbModeW(BookHandle handle, int rgbMode);

    XLAPI            int XLAPIENTRY xlBookVersionW(BookHandle handle);
    XLAPI            int XLAPIENTRY xlBookBiffVersionW(BookHandle handle);

    XLAPI            int XLAPIENTRY xlBookSetLocaleW(BookHandle handle, const char* locale);
    XLAPI    const char* XLAPIENTRY xlBookErrorMessageW(BookHandle handle);

    XLAPI           void XLAPIENTRY xlBookReleaseW(BookHandle handle);

#ifdef __cplusplus
}
#endif

#endif
