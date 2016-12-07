
require 'star'

--star.msgbox(star.encodeurlutf8('ÄãºÃworld'))
strHeaders = [[
Connection: keep-alive
Accept: application/json, text/javascript, */*; q=0.01
Origin: http://www.ximalaya.com
X-Requested-With: XMLHttpRequest
User-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/43.0.2357.134 Safari/537.36
Content-Type: application/x-www-form-urlencoded
Referer: http://www.ximalaya.com/
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.8,en;q=0.6
Cookie:xmHotwordsFS=%7B%22data%22%3A%5B%22%E7%9B%97%E5%A2%93%E7%AC%94%E8%AE%B0%22%2C%22%E7%BD%97%E8%BE%91%E6%80%9D%E7%BB%B4%22%2C%22%E8%8A%B1%E5%8D%83%E9%AA%A8%22%2C%22%E9%AC%BC%E5%90%B9%E7%81%AF%22%2C%22%E6%AE%B5%E5%AD%90%E6%9D%A5%E4%BA%86%22%5D%2C%22timestamp%22%3A1437384310066%7D; searchHistory=%255B%252299%25E5%25A4%259C%25E8%2588%25AA%25E7%258F%25AD%2522%252C%2522%25E6%2598%259F%25E6%2598%259F%25E5%2591%258A%25E8%25AF%2589%25E6%2588%2591%2522%252C%2522%25E5%258E%259F%25E5%258E%259F%25E9%25AB%2598%2522%255D; bdshare_firstime=1437726535139; 1&remember_me=y; 1&_token=27640597&4babd1e804ab5047a30951e000679a00073a; Hm_lvt_4a7d8ec50cfd6af753c4f8aee3425070=1437702532,1437973247,1438046403,1438136836; Hm_lpvt_4a7d8ec50cfd6af753c4f8aee3425070=1438158830; _ga=GA1.2.1777510722.1432177150; _gat=1; msgwarn=%7B%22category%22%3A%22%22%2C%22newMessage%22%3A0%2C%22newNotice%22%3A0%2C%22newComment%22%3A0%2C%22newQuan%22%3A0%2C%22newFollower%22%3A0%2C%22newLikes%22%3A0%7D; 1_l_flag=27640597&4babd1e804ab5047a30951e000679a00073a_2015-07-29 16:40:55
]]
star.sendhttpdata('www.ximalaya.com', 'msgcenter/create_letter', strHeaders, 'to_nickname=%E9%A3%9E%E7%87%95_ev&content=%E4%BD%A0%E5%90%AC%E8%BF%87%E5%A4%9A%E5%B0%91')

--[[
zip = star.zip()
zip:create('e:\\1.zip')
zip:push('e:\\1.txt',nil)
zip:push('e:\\2.jpg','res\\2.jpg')
zip:close()

zip:open('e:\\1.zip')
zip:pull('res/2.jpg','e:\\1')
zip:close()

--]]

--star.test()
--s = star.zipfile([[C:\Users\hzzhuxingxing\Desktop\ximalaya.zip]], [[C:\Users\hzzhuxingxing\Desktop\ximalaya.lua]],nil)
--s = star.zipadd([[C:\Users\hzzhuxingxing\Desktop\ximalaya.zip]],'ximalaya2.lua', [[C:\Users\hzzhuxingxing\Desktop\ximalaya.lua]])
--s = star.zipdelete([[C:\Users\hzzhuxingxing\Desktop\ximalaya.zip]],'ximalaya2.lua')
s = star.unzip([[C:\Users\hzzhuxingxing\Desktop\test.zip]],nil)
print(s)

--star.gethtml('www.baidu.com')


--STARTPATH,_,_ = [[C:\Users\hzzhuxingxing\Desktop\ximalaya]]
--root = [[C:\Users\hzzhuxingxing\Desktop\ximalaya\]]
--rawroot = root..'data\\raw\\' 
--ok = star.zipfile(root..'data\\', root..'123'..'.saz')
--