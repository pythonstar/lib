--[[
	Copyright (c) 2009 Christopher E. Moore ( christopher.e.moore@gmail.com / http://christopheremoore.net )

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
--]]

--[[
format of htmlparser.parse(s):

returns an indexed array of nodes representing the root nodes of a html tree

these nodes can be of either type objects:

	string:
		strings represent text leaf nodes within the tree

	table:
		tables are composed of the following properties:

			type: the type of the node.  could be one of the following:
				'tag'
				'cdata'
				'comment'

			str: if type is 'cdata', 'comment', or 'string' then this will hold the comment/cdata contents

			tag: if type is 'tag' then this holds the tag name.
			NOTE: tags with name 'script' or 'style' will only hold one child: a single string set to the script content

			attrs: if type is 'tag' then this holds an indexed array of objects with properties 'name' and 'value' defined
				if a html node has not attributes then this will be nil
			
			child: if type is 'tag' then this holds an indexed array of the child nodes of this node
				if a html node has not child then this will be nil

--]]

local readtoken = coroutine.wrap(function()
	for i=1,#page do
		coroutine.yield(page:sub(i,i))
	end
end)

local lasttoken = ''
local thistoken = ''
local thiscol = 1
local thisrow = 1
local function nexttoken()
	lasttoken = thistoken
	thistoken = readtoken()
	if thistoken == '\n' then
		if lasttoken ~= '\r' then
			thisrow = thisrow + 1
			thiscol = 0
		end
	elseif thistoken == '\r' then
		thisrow = thisrow + 1
		thiscol = 0
	else
		thiscol = thiscol + 1
	end
end

local function parseerror(msg)
	error(thisrow..':'..thiscol..':'..(msg or ''))
end

local function parseassert(test, msg)
	if not test then
		parseerror(msg)
	end
end

local function parseassertequals(a,b,msg)
	if a ~= b then
		parseerror((msg or '') .. ': '..tostring(a)..' ~= '..tostring(b))
	end
end

local function canbe(pattern)
	if not thistoken then return false end
	if string.match(thistoken, pattern) then
		nexttoken()
		return true
	end
end

local function mustbe(...)
	for _,pattern in ipairs{...} do
		parseassert(string.match(thistoken, pattern))
		nexttoken()
	end
end

local function spaces()
	while canbe('%s') do end
end

local namepattern = '[%w-]'
local function name()
	mustbe(namepattern)
	local n = lasttoken
	while canbe(namepattern) do
		n = n .. lasttoken
	end
	return n
end

local function enclosing(capstart, capfinish, nodetype)
	local s = ''
	local capture = capstart .. '(.*)' .. capfinish
	while thistoken do
		s = s .. thistoken
		nexttoken()
		local str = s:match(capture)
		if str then
			return {
				type=nodetype;
				str=str;
			}
		end
	end
	parseerror("found a non-terminating enclosing deal "..nodetype)
end

-- already got <![
local function cdata()
	return enclosing('CDATA%[', '%]%]>', 'cdata')
end

-- already got <!
local function comment()
	local str = ''
	if canbe('%[') then
		return cdata()
	end
	return enclosing('%-%-', '%-%->', 'comment')
end

local function short(s)
	if #s < 10 then return s end
	return s:sub(1,10):gsub('[\r\n]','.')..'...('..#s..')'
end

-- already got </
local function tagend()
	spaces()
	local n = name()
	spaces()
	mustbe('>')
	--print('reading closing tag '..short(n))
	return {
		type='closing';
		tag=n;
	}
end

-- this could be uesd with 'enclosing' maybe?
local function quotedstring()
	local quotetoken = thistoken
	mustbe("['\"]")
	--return enclosing('', quotetoken, 'string').str
	local s = ''
	while true do
		if canbe(quotetoken) then
			return s
		end
		s = s .. thistoken
		nexttoken()
	end
end

-- already got <
local function tagstart()
	-- closing tag...
	if canbe('/') then
		return tagend()
	end
	-- comment
	if canbe('!') then
		return comment()
	end
	
	local t = {
		type='tag';
		child=true;	-- true means we should parse children (and replace with a table) 
	}

	spaces()
	t.tag = name()

	while true do
		spaces()
		if canbe('/') then
			mustbe('>')
			t.child = nil	-- turn off our 'parse children' flag
			--print('reading standalone tag '..short(t.tag))
			return t
		end
		if canbe('>') then
			--print('reading tag '..short(t.tag))
			return t
		end
		local attr = {
			name = name();
		}
		if not attr.name then
			break
		end
		--print('  reading attr name '..attr.name)
		spaces()
		mustbe('=')
		spaces()
		attr.value = quotedstring()
		--print('  reading attr value '..attr.value)
		if not t.attrs then t.attrs = {} end
		table.insert(t.attrs, attr)
	end
	
	parseerror("shouldn't get this far")
end

-- tagname must be escaped
local function tagofsinglestring(tagname)
--[[
--[=[
this is going incredibly slloooowwww
that means it might be better to do a state-based compare
rather than a pattern-match-based one
--]=]
	local finish = '</%s*'..tagname..'%s*>'
	return enclosing('', finish, 'script')
--]]
-- [[ state based
	local s = ''
	local foundname = ''
	local closingindex
	local states = {
		-- states are arrays of pattern/nextstate
		openbrace = {};
		slash = {};
		openspace = {};
		tag = {};
		closespace = {};
	}
	states.openbrace.edges = {
		{reg='<', new=states.slash};
	}
	states.slash.enter = function()
		closingindex = #s-1
	end
	states.slash.edges = {
		{reg='/', new=states.openspace};
		{reg='<', new=states.slash};
		{new=states.openbrace};
	}
	states.openspace.edges = {
		{reg='[%w%-]', new=function() 
			foundname = thistoken
			return states.tag
		end};
		{reg='<', new=states.slash};
		{reg='[^%s]', new=states.openbrace};
	}
	states.tag.edges = {
		{reg='[%w%-]', new=function()
			foundname = foundname .. thistoken
			return states.tag
		end};
		{reg='<', new=states.slash};
		{reg='>', new=function()
			if foundname == tagname then
				return nil, s:sub(1,closingindex)
			end
			return states.openbrace
		end};
		{reg='%s', new=states.closespace};
		{new=states.openbrace};
	}
	states.closespace.edges = {
		{reg='>', new=function()
			if foundname == tagname then
				return nil, s:sub(1,closingindex)
			end
			return states.openbrace
		end};
		{reg='<', new=states.slash};
		{reg='[^%s]', new=states.openbrace};
	}
	local state = states.openbrace
	while thistoken do
		s = s .. thistoken
		for _,v in ipairs(state.edges) do
			if not v.reg or thistoken:match(v.reg) then
				local newstate = state
				if type(v.new) == 'table' then
					newstate = v.new
				elseif type(v.new) == 'function' then
					-- first argument is the new state
					-- second argument, if it exists, is the return node
					local nn
					newstate, nn = v.new()
					if nn then
						nexttoken()
						return nn
					end
				end
				if newstate ~= state then
					if state.exit then state.exit() end
					state = newstate
					if state.enter then state.enter() end
				end
				break
			end
		end
		nexttoken()
	end
--]]
end

local tagarray

local function tag()
	local t = tagstart()
	if t.tag and (t.tag == 'script' or t.tag == 'style') then
		local tagcontent = tagofsinglestring(t.tag)
		if #tagcontent > 0 then
			t.child = {tagcontent}
		else 
			t.child = nil
		end
		return t
	end
	if not t.child then return t end
	local array, closer = tagarray(t)
	t.child = array	-- run until you find a closing tag of type t.tag
	if #t.child == 0 then t.child = nil end
	return t, closer
end

-- assumes we already have a [^<]
local function leafstring()
	local s = ''
	while canbe('[^<]') do
		s = s .. lasttoken
	end
	return s
end

local nodestack = {}

function tagarray(parent)
	local parenttag
	if parent then parenttag = parent.tag end
	if parent then table.insert(nodestack, parent) end
	--print('... entering child set of type '..tostring(parenttag))
	local array = {}
	while thistoken do
		spaces()
		if canbe('<') then
			local ch, closer = tag()
			if closer then print('closing off multiple elements down to '..closer) end
			assert(ch.type ~= 'closing' or not closer, "we shouldn't have a closing tag and a closer returned")
			if ch.type == 'closing' or closer then
				closer = closer or ch.tag
-- [[ the new way
--[=[
closing tags in html are clever...
trace back up the stack
once we find a match, close all up to it
if we find no matches then throw it as an erroneous closing tag
--]=]
				local closingindex
				for i=#nodestack,1,-1 do
					if nodestack[i].tag == closer then
						-- then close off all tags down to that one
						-- and longjump into it in the stack (i.e. the flatten-stack operation)
						-- this will be tricky...
						closingindex = i
						break
					end
				end

				--print('closing off down to '..closer..' and found index to be '..tostring(closingindex)..' of '..#nodestack)
				--io.write('nodestack:')
				--for _,v in ipairs(nodestack) do io.write('  '..v.tag) end
				--io.write('\n')
				
				if closingindex then
					-- if we did find the closing tag in the list then
					-- if it was on top of the stack then return our children
					if closingindex == #nodestack then
						if parent then table.remove(nodestack) end
						return array
					end
					-- otherwise return what we're closing off at the end
					-- to be dealt with by the parent
					if parent then table.remove(nodestack) end
					return array, ch.tag
				end
				-- if we didn't find the closing tag in our list then don't return the array
				-- because it didn't close anything off
--]]
--[[ the old way
				parseassertequals(ch.tag, parenttag)
				print('... exiting child set of type '..tostring(parenttag))
				if parent then table.remove(nodestack) end
				return array
--]]
			else
				table.insert(array, ch)
			end
		else
			table.insert(array, leafstring())
		end
	end
	return array
	--parseerror("shouldn't have gotten here")
end

local function base()
	local ar = tagarray()
	return ar
end

-- if I recall there's a better way to do this
local htmlparser = {}
function htmlparser.parse(_page)
	-- set up our local data
	page = _page
	-- populate 'thistoken'
	nexttoken()
	-- and parse
	return base()
end

-- pretty printer while I'm here
function htmlparser.prettyprint(tree, tab)
	tab = tab or 0
	local tabstr = string.rep('\t',tab)
	for i,n in ipairs(tree) do
		if type(n) == 'string' then
			io.write(tabstr..n..'\n')
		elseif type(n) == 'table' then
			if n.type == 'tag' then
				io.write(tabstr..'<' .. n.tag)
				if n.attrs then
					for _,a in ipairs(n.attrs) do
						io.write(' '..a.name..'="'..a.value..'"')
					end
				end
				if (not n.child or #n.child == 0) and n.tag ~= 'script' and n.tag ~= 'style' then
					io.write('/>\n')
				else
					io.write('>\n')
					if n.child then
						htmlparser.prettyprint(n.child, tab+1)
					end
					io.write(tabstr..'</'..n.tag..'>\n')
				end
			elseif n.type == 'cdata' then
				io.write(tabstr..'<![CDATA[' .. n.str .. ']]>\n')
			elseif n.type == 'comment' then
				io.write(tabstr..'<!--' .. n.str .. '-->\n')
			else
				print("found child index "..i.." unknown node type: "..tostring(n.type))
			end
		else
			print("found child index "..i.." unknown lua type: "..tostring(type(n)))
		end
	end
end

local function debugprintnode(k, n, tab)
	print(string.rep('\t',tab),k,'=>',n)
	if type(n) == 'table' then
		htmlparser.debugprint(n, tab+1)
	end
end

function htmlparser.debugprint(tree, tab)
	tab = tab or 0
	for k,n in pairs(tree) do
		-- technically this'll miss any numbers that are set beyond the largest contiguous 1-based index...
		if k ~= 'child' then
			debugprintnode(k,n,tab)
		end
	end
	if tree.child then
		debugprintnode('child', tree.child, tab)
	end
end

return htmlparser
