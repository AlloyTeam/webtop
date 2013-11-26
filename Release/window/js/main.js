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
	AlloyDesktop.mini();
	if(browserWindow){
		AlloyDesktop.mini(browserWindow);
	}
}
header.onmousedown=function(){
	AlloyDesktop.drag();
}
header.onmouseup=function(){
	AlloyDesktop.stopDrag();
}
var sizeHandler=function(e){
	console.log(e);
	var w=e.detail.width,h=e.detail.height;
	container.style.height=h-12+'px';
	if(browserWindow){
		AlloyDesktop.setSize(w-30, h-55, browserWindow);
	}
	width=w;
	height=h;
}
var moveHandler=function(e){
	if(browserWindow){
		AlloyDesktop.move(e.detail.x+15,e.detail.y+40,browserWindow);
	}
}
var closeHandler=function(){
	AlloyDesktop.close(browserWindow);
	AlloyDesktop.close();
}
var focusHandler=function(){
	if(browserWindow){
		AlloyDesktop.bringToTop(browserWindow);
		AlloyDesktop.restore(browserWindow);
	}
}
var activeHandler=function(x,y){
	AlloyDesktop.focus(browserWindow);
}
var dropHandler=function(e){
	var fileList=e.detail.list;
	AlloyDesktop.loadUrlIn(fileList[0]);
	for(var i=1;i<fileList.length;++i){
		AlloyDesktop.browse(fileList[i]);
	}
}
AlloyDesktop.loadUrlIn=function(_url){
	if(!browserWindow){
		newPage(_url);
	}
	AlloyDesktop.loadUrl(_url,browserWindow);
	url=_url;		
}
var browserWindow;
function newPage(url,transparent,isBase){
	if(browserWindow){
		AlloyDesktop.close(browserWindow);
	}
	if(isBase){
		browserWindow=AlloyDesktop.createWindowBase(url,WS_EX_TOOLWINDOW,transparent,"var parentWindow="+handler);
	}
	else{
		browserWindow=AlloyDesktop.createWindow(url,WS_EX_TOOLWINDOW,transparent,"var parentWindow="+handler);
	}
	var pos=AlloyDesktop.getPos();
	AlloyDesktop.move(pos.x+15,pos.y+40,browserWindow);
	AlloyDesktop.setSize(width-30, height-55, browserWindow);
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
		AlloyDesktop.loadUrlIn(url)
	}
	AlloyDesktop.move(getArg('x'),getArg('y'));
	if(getArg('max')=='1'){
		AlloyDesktop.max();
	}
	else{
		AlloyDesktop.setSize(getArg('width'),getArg('height'));
	}
}
var closeHandler = function () {
    AlloyDesktop.close(browserWindow);
    AlloyDesktop.close();
}
addEventListener("AlloyDesktopReady",readyHandler);
addEventListener("AlloyDesktopWindowResize",sizeHandler);
addEventListener("AlloyDesktopWindowMove",moveHandler);
addEventListener("AlloyDesktopDragDrop",dropHandler);
addEventListener("AlloyDesktopWindowActive",activeHandler);
addEventListener("AlloyDesktopWindowFocus",focusHandler);
addEventListener("AlloyDesktopWindowClose", closeHandler);
title.innerHTML = getArg('name');
