	local page
	require 'socket'
	local http = require 'socket.http'

	local mainpage = 'http://hi.baidu.com/xeboy/home'
	do
		local filename = 'cachedpage.html'
		local file = io.open(filename, 'r')
		if file then
			page = assert(file:read('*a'))
			file:close()
		else
			page = assert(http.request(mainpage))
			file = assert(io.open(filename, 'w'))
			file:write(page)
			file:close()
		end
	end

