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
gah, i couldn't stand it ...
this is html auto format script for lua
i'm sure there are better/other ones out there.
but 10 mins of searching couldn't find me any (free) options
and i'm not downloading eclipse again .... no siree
--]]

assert(filename, 'you need to provide a filename (using -e "filename=\'...\'")')

tableMeta = { __index = table }
function table:new()
	local t = {}
	setmetatable(t, tableMeta)
	return t
end

-- make a buffer from our file
buffer = ''
for line in io.lines(filename) do
	buffer = buffer .. line .. '\n'
end

--[[
elements in a html tree:
text (anything else)
comment <!-- ... -->
stand-alone < ... />
open/closer < ... > ... </ ... >
and a non-xml-std stand-alone < ... >

 now for pattern matching ...
--]]

Node = {}
NodeMeta = { __index = Node }

function Node:new(t)
	if not t then t = {} end
	t.objs = table:new()
	setmetatable(t, NodeMeta)
	return t
end

function Node:print(space)
	if not space then space = '' end
	if self.wrap then io.write(space..'<') end
	-- if its a leaf then print its text
	if #self.objs == 0 then
		io.write(space..self.str)
	else
		for k,v in ipairs(self.objs) do
			v:print(space..'\t')
		end
	end
	if self.wrap then io.write('>') end
	io.write('\n')
end

TextNode={}
function TextNode:new(str)
	return Node:new{class='text', str=str}
end

TagNode={}
function TagNode:new(str)
	-- see if it is a comment ...
	-- if it is then don't go looking for a tag name
	if str:sub(1,4) == '<!--' and str:sub(#str-2) == '-->' then
		return Node:new{class='comment', str=str}
	end

	local n = Node:new{class='tag', str=str}

	-- pick out the name
	-- the more I use regexp's themore i think i should just parse it char at a time (SAX-style)
	local first,last
	first,last = str:find('%a[%w_]*')
	if not first then
		-- no name found - switch over to a tag node
		assert(false, 'found a tag with no name: '..str)
		n.class = 'text'
	else
		-- tag found - read it in as a string
		n.name = str:sub(first,last)
		-- if there's a / before us then we're a closing tag
		
		-- if there's a / after us then we're a stand-alone (unless it is in some quotes ... or double quotes ... )
		
	end
	return n
end

-- return a tree object of our string as html
function eat(str)
	local node = Node:new{class='page',str=str}
	-- first find our first <***>
	-- insert any text before it into our current branch
	-- while our <> doesn't have an ending / (while we're not stand-alone)
	-- 	and while our <***> doesn't have a matching </***>
	-- keep chugging 

	local cur = 1
	local done = false
	repeat
		local first,last
		first,last = str:find('<[^>]*>', cur)	-- look for balanced <>'s
		if not first then 	-- no <>'s found?  we can just put the whole thing in
			if cur <= #str then node.objs:insert(TextNode:new(str:sub(cur))) end	-- text node
			done = true
		else
			-- up to our node is safe
			if cur <= first-1 then node.objs:insert(TextNode:new(str:sub(cur, first-1))) end

--[[
now we pick the tag name out of the <>
that means take the <...>
then look for the first alpha+_
and follow it through all alphanumeric+_ characters to the end
that will be our name
now we scan from the back to the front (not crossing our path)
looking for another <> that holds the same name (via the same method)
if we find it then - for all nodes between there -
	first merge the two <>'s with the same name into one node
	second put all between nodes inside of it
--]]

			-- skip the outer <>'s and pass it through
			-- actually we don't have to eat this.  it is safe.
			node.objs:insert(TagNode:new(str:sub(first,last)))
			cur = last+1
		end
	until done

	-- now we have our list of nodes

	return node
end

tree = eat(buffer)

-- now recurse through the tree
tree:print()

