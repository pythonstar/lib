table.__index = table

function table.new(...)
	local t = setmetatable({}, table)
	for _,o in ipairs{...} do
		for k,v in pairs(o) do
			t[k] = v
		end
	end
	return t
end

setmetatable(table, {
	__call = function(t, ...)
		return table.new(...)
	end
})

-- something to consider:
-- mapvalue() returns a new table
-- but append() modifies the current table
-- for consistency shouldn't append() create a new one as well?
function table:append(...)
	for _,u in ipairs{...} do
		for _,v in ipairs(u) do
			table.insert(self, v)
		end
	end
	return self
end

function table:removeKeys(...)
	for _,v in ipairs{...} do
		self[v] = nil
	end
end

-- something to consider:
-- rather than map() for function(k,v) and mapvalue() for function(v)
-- I saw someone simply reverse k and v to produce a single
-- map(function(v,k)) which could operate as either
function table:map(cb)
	local t = table.new()
	for k,v in pairs(self) do
		local nk,nv = cb(k,v)
		t[nk] = nv
	end
	return t
end

function table:mapvalue(cb)
	local t = table.new()
	for k,v in pairs(self) do
		t[k] = cb(v)
	end
	return t
end

function table:keys()
	local t = table()
	for k,_ in pairs(self) do
		t:insert(k)
	end
	return t
end

-- should we have separate finds for pairs and ipairs?
function table:find(value, eq)
	if eq then
		for k,v in pairs(self) do
			if eq(v, value) then return k end
		end
	else
		for k,v in pairs(self) do
			if v == value then return k end
		end
	end
end

-- should insertUnique only operate on the pairs() ?
-- 	especially when insert() itself is an ipairs() operation
function table:insertUnique(value)
	if not table.find(self, value) then table.insert(self, value) end
end

function table:removeObject(...)
	local removed
	local len = #self
	local k = table.find(self, ...)
	while k ~= nil do
		if type(k) == 'number' and tonumber(k) <= len then
			-- print('removing by index')
			table.remove(self, k)
			removed = true
		else
			-- print('removing by key')
			self[k] = nil
			removed = true
		end
		k = table.find(self, ...)
	end
	return removed
end

-- I need to think of a better name for this...
function table:kvmerge()
	local t = table.new()
	for k,v in pairs(self) do
		table.insert(t, {k,v})
	end
	return t
end

-- TODO - math instead of table?
function table:sup()
	local best
	for _,v in pairs(self) do
		if best == nil or v > best then best = v end
	end
	return best
end

-- TODO - math instead of table?
function table:inf()
	local best
	for _,v in pairs(self) do
		if best == nil or v < best then best = v end
	end
	return best
end

