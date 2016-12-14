
--http://www.wellho.net/resources/ex.php4?item=u110/emmas


--[[
This example looks for all the email addresses in a web server
log file.  You might not expect there to be any email addresses
in such a file, but in practise, web crawlers and other automata
often include an email address in their user agent string as a
courtesy to the web server admin (this is not universal - many
provide a more useful URL instead to explain why they're crawling
you.  ]]

flow = io.open("../data/ac_20090727","r")
emails = {}
while true do
        line = flow:read()
        if not line then break end

        -- look for all email addresses in the log file line
        -- if there are none, the loop will be skipped
        for name,domain in string.gmatch(line,"([-%a%d%._]+)@([-%a%d.]+)") do

                -- Really no need to split and rejoin the email address
                -- it was a part of the demonstration.
                fulladdy = name .. "@" .. domain

                -- Count up the number of times each address occured

                if emails[fulladdy] then
                        emails[fulladdy] = emails[fulladdy] + 1
                else
                        emails[fulladdy] = 1
                end
        end
end

-- Turn a non-indexed table (a hash) into an indexed one for sorting

mypairs = {}
for name,value in pairs(emails) do
    table.insert(mypairs,{name=name, value=value})
    end

table.sort(mypairs,function(a,b) return a.value < b.value end)

for k,v in pairs(mypairs) do
        io.write (v.value .. " -- " .. v.name .. "\n")
        end
