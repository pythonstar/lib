// mylualib.cpp : 定义 DLL 应用程序的入口点。
//

#include "stdafx.h"
#include "mylib.h"

#include <luabind/luabind.hpp>
#include <luabind/function.hpp>
#include <luabind/class.hpp>
using namespace luabind;

#include <StarLib/File/File.h>
#include <StarLib/Common/zipHelperEx.h>

#ifdef _DEBUG
#pragma comment(lib,"luaD.lib")
#pragma comment(lib,"luabindD.lib")
#pragma comment(lib,"User32.lib")
#else
#pragma comment(lib,"lua.lib")
#pragma comment(lib,"luabind.lib")
#endif


#ifdef _MANAGED
#pragma managed(push, off)
#endif


//////////////////////////////////////////////////////////////////////////



/*
require "star"
require "libxl"

xls = star.XLS()
if xls:create() then
	xls:addtitle('1')
	xls:addtitle('2')
	xls:addtitle('3')
	xls:writestr(2,0,'123')
	xls:writestr(2,1,'123')
	xls:writestr(3,2,'123')
	xls:save('d:/2.xls')
else
	print(xls:error())
end

//////////////////////////////////////////////////////////////////////////
require "star"

dir = star.getluapath()
ini = star.INI(dir..'config.ini')
ini:setint('main','x',10)
print(ini:getint('main','x',0))
ini:setstring('main','h','11'..1001)
print(ini:getstring('main','h',''))

//////////////////////////////////////////////////////////////////////////
require "star"
sql = star.SQL()
sql:open('test.db')
sql:executesql('')
sql:close()
*/
extern "C" __declspec(dllexport) int luaopen_star(lua_State *L) { 
	CoInitialize(NULL);

	luabind::open(L);

	luaL_register(L, "star", starlib);	

#ifdef MYLUALIB_EXPORTS
	module(L,"star")
	[
		class_<CZipHelperEx>("zip")
			.def(constructor<>())
			.def("create", &CZipHelperEx::Create)
			.def("push", &CZipHelperEx::AddFile)
			.def("close", &CZipHelperEx::Close)
			.def("open", &CZipHelperEx::Open)
			.def("pull", &CZipHelperEx::UnzipOneItem),
		def("zipit", &CZipHelperEx::PackFile),
		def("unzip", &CZipHelperEx::UnpackFile),
		def("zipadd", &CZipHelperEx::Add),
		def("zipdelete", &CZipHelperEx::Delete)
	];
#endif

	//module(L,"star")
	//[
	//	class_<CXlsOperator>("XLS")
	//		.def(constructor<>())
	//		.def("create", &CXlsOperator::Create)
	//		.def("setcol", &CXlsOperator::SetCol)
	//		.def("addtitle", &CXlsOperator::AddTitle)
	//		.def("writestr", &CXlsOperator::WriteStr)
	//		.def("getrowscount", &CXlsOperator::GetRowsCount)
	//		.def("getcolscount", &CXlsOperator::GetColsCount)
	//		.def("save", &CXlsOperator::Save)
	//		.def("error", &CXlsOperator::GetLastError),
	//	class_<Star::File::CIni>("INI")
	//		.def(constructor<>())
	//		.def(constructor<LPCTSTR>())
	//		.def("setfile", &Star::File::CIni::SetIniFile)
	//		.def("getstring", &Star::File::CIni::GetIniString)
	//		.def("setstring", &Star::File::CIni::SetIniString)
	//		.def("getint", &Star::File::CIni::GetIniInt)
	//		.def("setint", &Star::File::CIni::SetIniInt),
	//	//def("a",a),
	//	//def("b",b)
	//];

	return 1; 
}


#ifdef _MANAGED
#pragma managed(pop)
#endif

