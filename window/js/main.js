var WS_EX_TOOLWINDOW=0x00000080;
var width,height;
var url;
var isBase=false;
var transparent=false;
var isDefault=false;
/*
 * 函数功能：从href获得参数
 * sArgName : arg1, arg2
 * return :   the value of arg. d, re
 */
function getArg(sArgName)
{
   var sHref = decodeURI(window.document.location.href);
   var args   = sHref.split("?");
   var retval = "";

   if(args[0] == sHref) /* 参数为空 */
   {
      return retval;
      /* 无需做任何处理 */
   }
   var str = args[1];
   args = str.split("&");
   for(var i = 0; i < args.length; i ++ )
   {
      str = args[i];
      var arg = str.split("=");
      if(arg.length <= 1) continue;
      if(arg[0] == sArgName) retval = arg[1];
   }
   return retval;
}

document.onselectstart=function(){
	return false;
}
closeBtn.onclick=function(){
	closeHandler();
}
miniBtn.onclick=function(){
	webtop.mini();
	if(browserWindow){
		webtop.mini(browserWindow);
	}
}
header.onmousedown=function(){
	webtop.drag();
}
header.onmouseup=function(){
	webtop.stopDrag();
}
var sizeHandler=function(e){
	console.log(e);
	var w=e.detail.width,h=e.detail.height;
	container.style.height=h-12+'px';
	if(browserWindow){
		webtop.setSize(w-30, h-55, browserWindow);
	}
	width=w;
	height=h;
}
var moveHandler=function(e){
	if(browserWindow){
		webtop.move(e.detail.x+15,e.detail.y+40,browserWindow);
	}
}
var closeHandler=function(){
	webtop.close(browserWindow);
	webtop.close();
}
var focusHandler=function(){
	if(browserWindow){
		webtop.bringToTop(browserWindow);
		webtop.restore(browserWindow);
	}
}
var activeHandler=function(x,y){
	webtop.focus(browserWindow);
}
var dropHandler=function(e){
	var fileList=e.detail.list;
	webtop.loadUrlIn(fileList[0]);
	for(var i=1;i<fileList.length;++i){
		webtop.browse(fileList[i]);
	}
}
webtop.loadUrlIn=function(_url){
	if(!browserWindow){
		newPage(_url);
	}
	webtop.loadUrl(_url,browserWindow);
	url=_url;		
}
var browserWindow;
function newPage(url,transparent,isBase){
	if(browserWindow){
		webtop.close(browserWindow);
	}
	if(isBase){
		browserWindow=webtop.createWindowBase(url,WS_EX_TOOLWINDOW,transparent,"var parentWindow="+handler);
	}
	else{
		browserWindow=webtop.createWindow(url,WS_EX_TOOLWINDOW,transparent,"var parentWindow="+handler);
	}
	var pos=webtop.getPos();
	webtop.move(pos.x+15,pos.y+40,browserWindow);
	webtop.setSize(width-30, height-55, browserWindow);
}
var $=function(id){
	return document.getElementById(id);
}
var href=location.href;
var url=getArg('url');
var param=getArg('param');
var readyHandler=function(){
	if(url){
		if(param){
			url+='?param='+param;
		}
		webtop.loadUrlIn(url)
	}
	webtop.move(getArg('x'),getArg('y'));
	if(getArg('max')=='1'){
		webtop.max();
	}
	else{
		webtop.setSize(getArg('width'),getArg('height'));
	}
}
addEventListener("webtopReady",readyHandler);
addEventListener("webtopWindowResize",sizeHandler);
addEventListener("webtopWindowMove",moveHandler);
addEventListener("webtopDragDrop",dropHandler);
addEventListener("webtopWindowActive",activeHandler);
addEventListener("webtopWindowFocus",focusHandler);
title.innerHTML=getArg('name');
