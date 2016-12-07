
------------------------------------------------------
--通过百度博客网址获取百度博客账号
function GetBaiduBlogAccount(url)
	local pos1=nil
	local pos2=nil
	local name=nil

	--找百度博客的域名
	pos2,pos1 = string.find(url,'hi.baidu.com/')
	if pos1==nil then
		return nil
	end 

	--从上次查找的结束位置开始找/
	pos1 = pos1+1
	pos2,_ = string.find(url,'/',pos1)
	if pos2==nil then
		pos2 = string.len(url)
	else 
		pos2 = pos2-1
	end 

	return string.sub(url,pos1,pos2)
end
------------------------------------------------------


------------------------------------------------------
--通过百度博客首页源码获取百度博客昵称
function GetBaiduBlogName(s)
  local i, f, t = string.find(s, "<title>(.+)_百度空间")
  return t or ""
end
------------------------------------------------------

------------------------------------------------------
--通过百度文章页面获取文章分类
function GetBaiduBlogArticalCategaries(s)
	local result={}
	local i=1
	local start=nil
	local over=nil
	local pos1=nil
	local pos2=nil
	local url=nil
	local name=nil

	start,start = string.find(s,[[id="m_artclg"]])
	if start==nil then 
		return result
	end

	over,_ = string.find(s,'<table',start)
	if over==nil then 
		over=string.len(s)
	end

	while true do
		pos2,pos1 = string.find(s,[[href="]],start)
		if pos1==nil or pos1>over then 
			break
		end
		pos2,_ = string.find(s,[["]],pos1+1)
		url = 'http://hi.baidu.com' .. string.sub(s,pos1+1,pos2-1)
		--print(url)

		start = pos2;
		pos2,pos1 = string.find(s,'>',start)
		if pos1==nil or pos1>over then 
			break
		end
		pos2,_ = string.find(s,[[</a>]],pos1)
		name = string.sub(s,pos1+1,pos2-1)
		--print(name)

		--保存到表中
		result[i] = {}	--为表添加一个元素
		result[i].url = url
		result[i].name = name
		i = i+1

		start = pos2;
		
	end	--end while

	return result
end
------------------------------------------------------

--测试
local s1 = 'http://hi.baidu.com/xeboy/blog'
local s2 = 'http://hi.baidu.com/xeboy'
local s3 = 'http://hi.baidu.com'

name = GetBaiduBlogAccount(s3)
print(name)
name = GetBaiduBlogAccount(s1)
print(name)
name = GetBaiduBlogAccount(s2)
print(name)

--博客首页地址
url = 'http://hi.baidu.com/' .. name

require 'socket'
local http = require 'socket.http'
--[[
html = assert(http.request(url))
name = GetBaiduBlogName(html)
print(name)
]]

--博客文章页面
url = 'http://hi.baidu.com/' .. name .. '/blog'
--html = assert(http.request(url))

html=[[<div id="m_artclg" class="modbox">

<div class="item"><a href="/xeboy/blog/category/%C4%AC%C8%CF%B7%D6%C0%E0" title="查看该分类中所有文章">默认分类</a>(50)</div>
<div class="line">&nbsp;</div>

<div class="item"><a href="/xeboy/blog/category/%BD%E9%C9%DC" title="查看该分类中所有文章">介绍</a>(34)</div>
<div class="line">&nbsp;</div>

<div class="item"><a href="/xeboy/blog/category/%BD%CC%B3%CC" title="查看该分类中所有文章">教程</a>(23)</div>
<div class="line">&nbsp;</div>

<div class="item"><a href="/xeboy/blog/category/%C8%E7%BA%CE%BF%AA%B7%A2%D2%BB%B8%F6%B0%D9%B6%C8%BF%D5%BC%E4" title="查看该分类中所有文章">如何开发一个百度空间</a>(3)</div>
<div class="line">&nbsp;</div>

<div class="item"><a href="/xeboy/blog/category/Wpf" title="查看该分类中所有文章">Wpf</a>(1)</div>
<div class="line">&nbsp;</div>

<div class="item"><a href="/xeboy/blog/category/%CA%FD%BE%DD%BD%E1%B9%B9%26%2338%3B%CB%E3%B7%A8" title="查看该分类中所有文章">数据结构&#38;算法</a>(2)</div>
<div class="line">&nbsp;</div>

<div class="item"><a href="/xeboy/blog/category/python%26%2338%3Bdjango" title="查看该分类中所有文章">python&#38;django</a>(12)</div>
<div class="line">&nbsp;</div>

<div class="item"><a href="/xeboy/blog/category/Bosoft" title="查看该分类中所有文章">Bosoft</a>(0)</div>
<div class="line">&nbsp;</div>

<div class="item"><a href="/xeboy/blog/category/Nitrogen" title="查看该分类中所有文章">Nitrogen</a>(11)</div>
<div class="line">&nbsp;</div>

<div class="item"><a href="/xeboy/blog/category/Corona" title="查看该分类中所有文章">Corona</a>(4)</div>
<div class="line">&nbsp;</div>

</div>
<table width="100%" border="0" cellspacing="0" cellpadding="0" height="8">
<tr><td class="modbl" width="7">&nbsp;</td>
<td class="modbc">&nbsp;</td>
<td class="modbr" width="7">&nbsp;</td>
</tr></table>
</div>]]

artclg=GetBaiduBlogArticalCategaries(html)
for i,v in ipairs(artclg) do 
	print(v.url,v.name)
end



