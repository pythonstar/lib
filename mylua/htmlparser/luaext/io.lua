function io.fileexists(fn)
	local f = {io.open(fn, 'r')}
	if not f[1] then return unpack(f) end
	f = f[1]
	f:close()
	return true
end

function io.readfile(fn)
	local f = {io.open(fn, 'rb')}
	if not f[1] then return unpack(f) end
	f = f[1]
	local d = f:read('*a')
	f:close()
	return d
end

function io.writefile(fn, d)
	local f = {io.open(fn, 'wb')}
	if not f[1] then return unpack(f) end
	f = f[1]
	f:write(d)
	f:close()
	return true
end

function io.readproc(cmd)
	local f = {io.popen(cmd)}
	if not f[1] then return unpack(f) end
	f = f[1]
	local d = f:read('*a')
	f:close()
	return d
end

