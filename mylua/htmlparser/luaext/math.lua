function math.clamp(v,min,max)
	return math.min(math.max(v,min), max)
end

math.infinity = -math.log(0)
