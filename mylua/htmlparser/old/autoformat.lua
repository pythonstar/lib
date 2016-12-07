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
tokenizer

the idea is nice
and works great for formal languages
not so much for html
--]]

local buffer = ''
for line in io.lines(filename) do
	buffer = buffer .. line
end

--hack table to use the same :new naming scheme
tablemeta = { __index = table }
function table:new()
	local t = {}
	setmetatable(t, tablemeta)
	return t
end

function class()
	local classobj = {}
	classobj.metaobject = { __index = classobj }
	function classobj:new(...)
		local obj = {}
		setmetatable(obj, self.metaobject)
		if obj.ctor then obj:ctor(...) end
		return obj
	end
	return classobj
end

Lex = class()	-- nameless classes.  amazing.  good luck keeping track of them without always directly setting ...
Lex.STATE = { TEXT = {}, TAG = {} }
function Lex:ctor(str)
	self.state = self.STATE.TEXT
	self.str = str
	self.index = 1	-- current location.  nil means we're done (find-style)
	self:getNextChar()
	self:getToken()
	-- load the first char / first token into the system.
end
-- grab the next char (for token building)
function Lex:getNextChar()	-- get the next char
	local lastchar = self.nextchar	-- return the prev char before the one we get
	if not self.index then
		self.nextchar = nil
	else
		-- grab the next token
		self.nextchar = self.str:sub(self.index, self.index)
		-- increment and test for eof
		self.index = self.index + 1
		if self.index > #self.str then self.index = nil end
	end
	return lastchar
end
-- grab the next token
function Lex:getToken()
	local lasttoken = self.token
	local lasttokentype = self.tokenType

	-- skip any white spaces
	if self.nextchar then
		while self.nextchar:find('%s') do self:getNextChar() end
	end
	-- now append our token
	self.token = self:getNextChar()
	self.tokenType = nil			-- clear == arbitrary
	if self.token then

		-- if we're inside of a <> then parse one way
		-- if we're outside then parse another (amass things until we get to a <)
		if self.state == self.STATE.TEXT then
			if self.token == '<' then
				-- in comment detecting
				-- a side effect is now all <! <!- and <!-- tokens are distinct
				if self.nextchar == '!' then
					self.token = self.token .. self:getNextChar()
					if self.nextchar == '-' then
						self.token = self.token .. self:getNextChar()
						if self.nextchar == '-' then
							self.token = self.token .. self:getNextChar()
							self.tokenType = 'comment'
							-- just read the comment in here
							while self.token:sub(#self.token-2) ~= '-->' do
								self.token = self.token .. self:getNextChar()
							end
						end
					end
				end
				if self.tokenType ~= 'comment' then -- if it wasn't a comment then it better've been a tag ...
					self.state = self.STATE.TAG
				end
			else
				self.tokenType = 'text'
				while self.nextchar and self.nextchar ~= '<' do
					self.token = self.token .. self:getNextChar()
				end
			end
		elseif self.state == self.STATE.TAG then
			-- while we should add it do so.
			-- whitespace (should we even consider these?) or should we skip them?  skip.
			-- alpha char <-> read until we run out of
			if self.token:find('%a') then
				self.tokenType = 'name'
				while self.nextchar:find('[%a%d-_]') do
					self.token = self.token .. self:getNextChar()
				end
			-- numeric chars ... [-] ###.####
			elseif self.token == '-' or self.token == '.' or self.token:find('%d') then
				self.tokenType = 'number'
				if self.token == '-' or self.token:find('%d') then
					while self.nextchar:find('%d') do
						self.token = self.token .. self:getNextChar()
					end
				end
				if self.nextchar  == '.' then
					self.token = self.token .. self:getNextChar()
					while self.nextchar:find('%d') do
						self.token = self.token .. self:getNextChar()
					end
				end
			-- strings
			elseif self.token == '\'' or self.token == '"' then
				self.tokenType = 'string'
				local quotetype = self.token
				while self.nextchar ~= quotetype do
					self.token = self.token .. self:getNextChar()
					if self.token == '\\' then	-- make sure its not escaped ...
						self.token = self.token .. self:getNextChar()
					end
				end
				self.token = self.token .. self:getNextChar()
			elseif self.token == '>' then
				self.state = self.STATE.TEXT
			end
		end
	end
	return lasttoken, lasttokentype
end

function Lex:error(msg)
	msg = msg .. '\n'
	msg = msg .. 'current token is '..self.token..'\n'
	msg = msg .. 'current type is '..self.tokenType..'\n'
	msg = msg .. 'current buffer location: '..self.index..'\n'
	local index = self.index or #self.str
	local min = index - 30
	if min < 1 then min = 1 end
	local max = index + 30
	if max > #self.str then max = #self.str end
	msg = msg .. 'surrounding content:\n'..self.str:sub(min,max)..'\n'
	error(msg)
end

function Lex:maybe(token)
	if self.token == token then
		self:getToken()
		return true
	end
end

function Lex:maybeType(...)
	for i,t in ipairs{...} do
		if self.tokenType == t then
			self:getToken()
			return true
		end
	end
end

function Lex:expect(token)
	local nexttoken = self:getToken()
	if nexttoken ~= token then lex:error('expected '..token) end
	return nexttoken
end

function Lex:expectType(...)
	local expectedTypes = ''
	for i,t in ipairs{...} do
		if self.tokenType == t then
			return self:getToken()	-- make sure we give back then info if we find it
		end
		expectedTypes = expectedTypes .. ' ' .. t
	end
	lex:error("didn't find a token from the list we were given:\n"..expectedTypes)
end


lex = Lex:new(buffer)

Node = class()
function Node:ctor(args)
	if args then
		for k,v in pairs(args) do
			self[k] = v
		end
	end
end
function Node:print(tabs, sameline)
	if not tabs then tabs = '' end
	local endline = '\n'
	if sameline then endline = '' tabs = '' end

	if self.type == 'comment' or self.type == 'text' then
		io.write(self.text..endline)
	elseif self.type == 'tag' then
		io.write(tabs..'<')
		if self.doctype then io.write('!') end
		io.write(self.name)
		if self.args then
			for i,v in ipairs(self.args) do
				local value = ''
				if v.value then value = '='..v.value end
				io.write(' '..v.key..value)
			end
		end
		local close = '>'
		if self.standalone then close = '/' .. close .. endline end
		io.write(close)
		if self.children then
			if #self.children == 0 then
			elseif #self.children == 1 and (not self.children[1].children or #self.children[1].children == 0) then
				self.children[1]:print('',true)
			else
				print()	--newline
				for i,v in ipairs(self.children) do
					v:print(tabs..'\t')
				end
				-- only print the closer if we have children
				io.write(tabs)
			end
			io.write('</'..self.name..'>'..endline)
		end
	elseif self.type == 'document' then
		for i,v in ipairs(self.children) do
			v:print()
		end
	else
		assert(false,'forgot to make a Node:print for type '..tostring(self.type))
	end
end
function Node:detailedPrint()
	if self.type then print('type: '..self.type) end
	if self.name then print('name: '..self.name) end
	if self.args then
		print('args:')
		for i,v in ipairs(self.args) do
			local value = ''
			if v.value then value = '='..v.value end
			print('\t' .. v.key .. value)
		end
	end
	if self.children then
		print('children:')
		for i,v in ipairs(self.children) do
			print('begin child -- ')
			v:detailedPrint()
			print('-- end child')
		end
	end
end

function readkey()
	if lex.tokenType == 'string' then return lex:getToken() end
	if lex.tokenType == 'name' then
		local key = lex:getToken()	-- returns the token, used for the key
		if lex.token == ':' then
			key = key .. lex:getToken()	--namespace dealio
			key = key .. lex:expectType('name')
		end
		return key
	end
end

function node()
	if not lex.token then return end
	local n = Node:new()
	-- see if we got some text first of all ...
	if lex.tokenType == 'text' or lex.tokenType == 'comment' then
		n.type = lex.tokenType
		n.text = lex.token
		lex:getToken()
	else
		-- pick out a < and keep going til you get a >
		if lex:maybe('<!') then	--doctype
			n.doctype = true
			n.name = lex:expect('DOCTYPE')
		elseif lex:maybe('<') then
			if lex:maybe('/') then
				n.closer = true
			end
			n.name = lex:expectType('name')
		else
			lex:error('expected a < or a <!')
		end
		n.type = 'tag'
		if not n.closer then	-- no args on a closer
			n.args = table:new()
			local key = readkey()
			while key do
				-- store args as {{key=.., value=..}} rather than {..=..} so we preserve order
				local arg = { key = key }
				if lex:maybe('=') then
					arg.value = lex:expectType('string','name','number')
				end
				n.args:insert(arg)
				key = readkey()
			end

			if lex:maybe('/') then
				n.standalone = true	-- no children
			end
		end
		lex:expect('>')
	end
	return n
end

--[[
next algorithm for building a tree from quirksmode:
1) run forward to the first closing tag
2) run backwards to the first matching opening tag
3) build a subtree from that.
--]]
function treeize(n,first,last)
	if not first then first = 1 end
	if not last then last = #n.children end
--[[
now p is one flat collection of all nodes
now we piece them together ...
look for tags that are not standalone
then search backwards (across our current range) for a closer with matching tag
once you find them subdivide and conquor, then continue on after the closer
--]]
	local i = first
	while i <= last do
		local v = n.children[i]
		if v.type == 'tag' and v.closer then
			assert(not v.standalone)	--clsoers aren't standalones.  </asdf> ~= <asdf/>
			assert(not v.args)	-- i better not have added args to closers...
			local j = i-1
			while j >= first do
				local u = n.children[j]
				-- here u.children is the flag for whether or not it's already been paired with an opener
				if u.type == 'tag' and u.name == v.name and not u.standalone and not u.closer and not u.children then
					-- we found a match!
					-- move everything from i+1 to j-1 into v's children (should be empty before now)
					u.children = table:new()
					for k=j+1,i-1 do
						u.children:insert(n.children[k])
					end

					-- now we can remove these stray elements ... do so back to front for indexing consistenty
					local removed = 0
					for k=i,j+1,-1 do	-- remove index 'j' as well cuz we dont need the closer anymore
						n.children:remove(k)
						removed = removed + 1
					end

					-- then remove u (because it's part of v)
					-- and continue processing after j
					--
					-- don't forget to offset j by the subset removed when we continue
					-- and 'last' too

					i = j	-- plus one to pass the last entry, but that'll happen in a few lines
					last = #n.children	--last - removed
					break
				end
				j = j - 1
			end
		end
		i = i + 1
	end
end

function trim(n)
	if n.args then
		local i = #n.args
		while i > 0 do
			local a = n.args[i]
			if n.type == 'tag' and n.name == 'a' and a.key == 'target' then
				if a.value == '_self' or a.value == '"_self"' or a.value == "'_self'" then
					n.args:remove(i)	-- target="_self" is redundant
				end
			end
			i = i - 1
		end
	end
	if n.children then
		-- traverse children backwards so we can remove them if we want to
		local i = #n.children
		while i > 0 do
			local ch = n.children[i]
			trim(ch)
			if ch.type == 'comment' then
				n.children:remove(i)
			elseif ch.type == 'tag' then
				if (ch.name == 'span' --[[or ch.name == 'div'--]]) and (not ch.children or #ch.children == 0) then
					n.children:remove(i)
				end
			end
			i = i - 1
		end
	end
end

--[[
page:
expect a tag up front
it might be a doctype (<!DOCTYPE ... >)
otherwise it better be a <html> tag
--]]
function doc()
	local d = Node:new{type='document', children=table:new()}
	while true do
		local n = node()	-- doctype?
		if not n then break end
		d.children:insert(n)
	end
	treeize(d)
	trim(d)

	return d
end

local p = doc(buffer)
p:print()
