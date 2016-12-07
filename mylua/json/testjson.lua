
json = require ("dkjson")

local jsonTest = json.encode( { 1, 2, 'fred', {first='mars',second='venus',third='earth'} } )

print(jsonTest)

local result = json.decode(jsonTest)

table.foreach(result,print)
