function GetHtmlFileTitle(fname)
  local fp = io.open(fname, "r")
  if fp == nil then
    return false
  end

  -- Read up to 8KB (avoid problems when trying to parse /dev/urandom)
  local s = fp:read(8192)
  fp:close()

  -- Remove optional spaces from the tags.
  s = string.gsub(s, "\n", " ")
  s = string.gsub(s, " *< *", "<")
  s = string.gsub(s, " *> *", ">")

  -- Put all the tags in lowercase.
  s = string.gsub(s, "(<[^ >]+)", string.lower)

  local i, f, t = string.find(s, "<title>(.+)</title>")
  return t or ""
end

--����:htmlԴ��
function GetHtmlTitle(s)
  -- Remove optional spaces from the tags.
  s = string.gsub(s, "\n", " ")
  s = string.gsub(s, " *< *", "<")
  s = string.gsub(s, " *> *", ">")

  -- Put all the tags in lowercase.
  s = string.gsub(s, "(<[^ >]+)", string.lower)

  local i, f, t = string.find(s, "<title>(.+)</title>")
  return t or ""
end


local s=[[<!doctype html><html><head><meta http-equiv="Content-Type" content="text/html;charset=gb2312"><title>�ٶ�һ�£����֪��      </title><style>html{overflow-y:auto}body{font:12px arial;text-align:center;background:#fff}body,p,form,ul{margin:0;padding:0}body,form,#fm{position:relative}td{text-align:left}img{border:0}a{color:#00c}a:active{color:#f60}#u{padding:7px 10px 3px 0;text-align:right}#m{width:680px;margin:0 auto}#nv{font-size:16px;margin:0 0 4px;text-align:left;text-indent:117px}#nv a,#nv b,.btn,#lk{font-size:14px}#fm{padding-left:90px;text-align:left}#kw{width:404px;height:22px;padding:4px 7px;padding:6px 7px 2px\9;font:16px arial;background:url(http://www.baidu.com/img/i-1.0.0.png) no-repeat -304px 0;_background-attachment:fixed;border:1px solid #cdcdcd;border-color:#9a9a9a #cdcdcd #cdcdcd #9a9a9a;vertical-align:top}.btn{width:95px;height:32px;padding:0;padding-top:2px\9;border:0;background:#ddd url(http://www.baidu.com/img/i-1.0.0.png) no-repeat;cursor:pointer}.btn_h{background-position:-100px 0}#kw,.btn_wr{margin:0 5px 0 0}.btn_wr{width:97px;height:34px;display:inline-block;background:url(http://www.baidu.com/img/i-1.0.0.png) no-repeat -202px 0;_top:1px;*position:relative}#lk{margin:33px 0}#lk span{font:14px "����"}#lm{height:60px}#lh{margin:16px 0 5px;word-spacing:3px}#mCon{height:18px;line-height:18px;position:absolute;right:7px;top:8px;top:10px\9;cursor:pointer;padding:0 18px 0 0;background:url(http://www.baidu.com/img/bg-1.0.0.gif) no-repeat right -134px;background-position:right -136px\9}#mCon span{color:#00c;cursor:default;display:block}#mCon .hw{text-decoration:underline;cursor:pointer}#mMenu{width:56px;border:1px solid #9a99ff;list-style:none;position:absolute;right:7px;top:28px;display:none;background:#fff}#mMenu a{width:100%;height:100%;display:block;line-height:22px;text-indent:6px;text-decoration:none}#mMenu a:hover{background:#d9e1f6}#mMenu .ln{height:1px;background:#ccf;overflow:hidden;margin:2px;font-size:1px;line-height:1px}#cp,#cp a{color:#77c}#sh{display:none;behavior:url(#default#homepage)}</style></head>
<body><p id="u"><a href="http://passport.baidu.com" target="_blank"><b>asmcvc</b></a>&nbsp;|&nbsp;<a href="http://hi.baidu.com/sys/checkuser/asmcvc/3" target="_blank">�ҵĿռ�</a>&nbsp;|&nbsp;<a href="/gaoji/preferences.html">��������</a>&nbsp;|&nbsp;<a href="http://passport.baidu.com/?logout&tpl=mn&bdstoken=f9284932388dd9bffb99bbe96638bc25">�˳�</a></p><div id="m"><p id="lg"><img src="http://www.baidu.com/img/baidu_sylogo1.gif" width="270" height="129" usemap="#mp"></p><p id="nv"><a href="http://news.baidu.com">��&nbsp;��</a>��<b>��&nbsp;ҳ</b>��<a href="http://tieba.baidu.com">��&nbsp;��</a>��<a href="http://zhidao.baidu.com">֪&nbsp;��</a>��<a href="http://mp3.baidu.com">MP3</a>��<a href="http://image.baidu.com">ͼ&nbsp;Ƭ</a>��<a href="http://video.baidu.com">��&nbsp;Ƶ</a>��<a href="http://map.baidu.com">��&nbsp;ͼ</a></p><div id="fm"><form name="f" action="/s"><input type="text" name="wd" id="kw" maxlength="100"><input type="hidden" name="rsv_bp" value="0"><span class="btn_wr"><input type="submit" value="�ٶ�һ��" id="su" class="btn" onmousedown="this.className='btn btn_h'" onmouseout="this.className='btn'"></span></form><div id="mCon"><span>���뷨</span></div><ul id="mMenu"><li><a href="#" name="ime_hw">��д</a></li><li><a href="#" name="ime_py">ƴ��</a></li><li class="ln"></li><li><a href="#" name="ime_cl">�ر�</a></li></ul></div>
<p id="lk"><a href="http://hi.baidu.com">�ռ�</a>��<a href="http://baike.baidu.com">�ٿ�</a>��<a href="http://www.hao123.com">hao123</a><span> | <a href="/more/">����&gt;&gt;</a></span></p><p id="lm"></p><p><a id="sh" onClick="this.setHomePage('http://www.baidu.com/')" href="http://utility.baidu.com/traf/click.php?id=215&url=http://www.baidu.com" onmousedown="return ns_c({'fm':'behs','tab':'homepage','pos':0})">�Ѱٶ���Ϊ��ҳ</a></p><p id="lh"><a href="http://e.baidu.com/?refer=888">����ٶ��ƹ�</a> | <a href="http://top.baidu.com">�������ư�</a> | <a href="http://home.baidu.com">���ڰٶ�</a> | <a href="http://ir.baidu.com">About Baidu</a></p><p id="cp">&copy;2011 Baidu <a href="/duty/">ʹ�ðٶ�ǰ�ض�</a> <a href="http://www.miibeian.gov.cn" target="_blank">��ICP֤030173��</a> <img src="http://gimg.baidu.com/img/gs.gif"></p></div><map name="mp"><area shape="rect" coords="40,25,230,95"  href="http://hi.baidu.com/baidu/" target="_blank" title="��˽��� �ٶȵĿռ�" ></map></body>
<script>var w=window,d=document,n=navigator,k=d.f.wd,a=d.getElementById("nv").getElementsByTagName("a"),isIE=n.userAgent.indexOf("MSIE")!=-1&&!window.opera,sh=d.getElementById("sh");if(isIE&&sh&&!sh.isHomePage("http://www.baidu.com/")){sh.style.display="inline"}for(var i=0;i<a.length;i++){a[i].onclick=function(){if(k.value.length>0){var C=this,A=C.href,B=encodeURIComponent(k.value);if(A.indexOf("q=")!=-1){C.href=A.replace(/q=[^&$]*/,"q="+B)}else{this.href+="?q="+B}}}}(function(){if(/q=([^&]+)/.test(location.search)){k.value=decodeURIComponent(RegExp.$1)}})();if(n.cookieEnabled&&!/sug?=0/.test(d.cookie)){d.write('<script src=http://www.baidu.com/js/bdsug.js?v=1.0.3.0><\/script>')}function addEV(C,B,A){if(w.attachEvent){C.attachEvent("on"+B,A)}else{if(w.addEventListener){C.addEventListener(B,A,false)}}}function G(A){return d.getElementById(A)}function ns_c(E){var F=encodeURIComponent(window.document.location.href),D="",A="",B="",C=window["BD_PS_C"+(new Date()).getTime()]=new Image();for(v in E){A=E[v];D+=v+"="+A+"&"}B="&mu="+F;C.src="http://nsclick.baidu.com/v.gif?pid=201&pj=www&"+D+"path="+F+"&t="+new Date().getTime();return true}var bdimeHW={hasF:1};var imeTar="kw";var ime_t1=(new Date()).getTime();(function(){var M=G("mCon"),A=G("mMenu");var B=["���뷨","��д","ƴ��"],O=["cl","hw","py"],D=["","http://www.baidu.com/hw/hwInput_1.1.js","http://www.baidu.com/olime/bdime.js"],N=[0,0,0];var L=n.cookieEnabled;if(L&&/\bbdime=(\d)/.test(d.cookie)){H(O[RegExp.$1],false)}var K=A.getElementsByTagName("a");for(var I=0;I<K.length;I++){K[I].onclick=F}if(isIE){var E=[];var P=M.getElementsByTagName("*");for(var I=0;I<P.length;I++){E.push(P[I])}E.push(M);var P=A.getElementsByTagName("*");for(var I=0;I<P.length;I++){E.push(P[I])}E.push(A);for(var I=0;I<E.length;I++){E[I].setAttribute("unselectable","on")}}function F(){ime_t1=(new Date()).getTime();var R=this.name.split("_")[1];try{if(w.bdime){bdime.control.closeIme()}}catch(Q){}H(R,true);return false}function H(V,Q){var T=0;if(V==O[1]){T=1;M.innerHTML='<span id="imeS" class="hw">'+B[1]+"</span>";if(isIE){G("imeS").setAttribute("unselectable","on")}function S(){if(!N[1]){if(d.selection&&d.activeElement.id&&d.activeElement.id=="kw"){bdimeHW.hasF=1}bdimeHW.input=imeTar;bdimeHW.submit="su";J(D[1]);setTimeout(function(){if(typeof bdsug!="undefined"){bdsug.sug.initial()}},1000);N[1]=1}else{bdimeHW.reload(Q)}}if(Q){S()}else{addEV(G("imeS"),"click",S)}}else{if(V==O[2]){T=2;M.innerHTML="<span>"+B[2]+"</span>";if(!N[2]){J(D[2]);N[2]=1}else{try{if(w.bdime){bdime.control.openIme()}}catch(U){}}}else{M.innerHTML="<span>"+B[0]+"</span>"}}if(Q&&L){var R=new Date();R.setTime(R.getTime()+365*24*3600*1000);d.cookie="bdime="+T+";domain=baidu.com;path=/;expires="+R.toGMTString()}}function J(Q){if(Q){var R=d.createElement("script");R.src=Q;d.getElementsByTagName("head")[0].appendChild(R)}}function C(R){var R=R||window.event;var Q=R.target||R.srcElement;A.style.display=Q.id=="mCon"&&A.style.display!="block"?"block":"none"}addEV(d,"click",C)})();addEV(w,"load",function(){k.focus()});w.onunload=function(){};;</script>
<script type="text/javascript" src="http://www.baidu.com/cache/hps/js/hps-1.1.js"></script>
</html><!--af78f7b5aaf16920-->]]

title=GetHtmlTitle(s)
print(title)



