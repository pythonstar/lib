-- TODO -move to htmlparser and make instances of the tree nodes

function findnode(list, callback)
	local res = {}
	assert(type(list) == 'table')
	for _,v in ipairs(list) do
		if callback(v) then
			table.insert(res, v)
		end
	end
	return res
end

function findtags(list, tagname, attrs)
	return findnode(list, function(n)
		if type(n) ~= 'table' then return false end
		if n.tag ~= tagname then return false end
		if attrs then
			for k,v in pairs(attrs) do
				local found = false
				if n.attrs then
					for _,kv in ipairs(n.attrs) do
						if kv.name == k then
							if kv.value ~= v then return false end
							found = true
						end
					end
				end
				if not found then return false end
			end
		end
		return true
	end)
end

function findtag(...)
	return (findtags(...))[1]
end

function findchild(node, ...)
	return findtag(node.child, ...)
end

function findchilds(node, ...)
	return findtags(node.child, ...)
end

function findattr(node, name)
	if node.attrs then
		for _,kv in ipairs(node.attrs) do
			if kv.name == name then return kv.value end
		end
	end
end


