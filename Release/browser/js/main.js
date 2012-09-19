var WS_EX_TOOLWINDOW=0x00000080;
var width,height;
var url;
var isBase=false;
var transparent=false;
var isDefault=false;
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
var showDevHandler=function(){
	if(!browserWindow){
		webtop.showDev();
	}
	else{
		webtop.showDev(browserWindow);
	}

}
devBtn.onclick=function(){
	showDevHandler();
}
closeBtn.onclick=function(){
	closeHandler();
}
function switchMode(){
	transparent=!transparent;
	newPage(url,transparent,isBase);
	isDefault=false;
}
transparentBtn.onclick=function(){
	switchMode();
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
		webtop.setSize(w-30, h-85, browserWindow);
	}
	width=w;
	height=h;
}
var moveHandler=function(e){
	if(browserWindow){
		webtop.move(e.detail.x+15,e.detail.y+70,browserWindow);
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
		//webtop.focus(browserWindow);
	}
}
var activeHandler=function(){
	webtop.focus(browserWindow);
}
var dropHandler=function(e){
	var fileList=e.detail.list;
	webtop.loadUrlIn(fileList[0]);
	for(var i=1;i<fileList.length;++i){
		webtop.browse(fileList[i]);
	}
	console.log(e.detail);
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
	webtop.move(pos.x+15,pos.y+70,browserWindow);
	webtop.setSize(width-30, height-85, browserWindow);
}
var $=function(id){
	return document.getElementById(id);
}
$('lst-ib').onkeyup=function(e){
	if(e.keyCode==13){
		/*if(isDefault){
			isBase=false;
			switchMode();
			isDefault=false;
		}*/
		webtop.loadUrlIn($('lst-ib').value);
		webtop.focus(browserWindow);
		//newPage($('lst-ib').value);
	}
}
makeBtn.onclick=function(){
	var path='';
	var index1=url.lastIndexOf('\\');
	var index=url.lastIndexOf('.');
	if(index1==-1){
		index1=url.lastIndexOf('/');
	}
	if(index<index1){
		index=url.length;
	}
	var s='[BASE]\n';
	if(url.indexOf('http')==-1&&url.indexOf(':')!=-1){
		s+='url='+url.substring(index1+1);
		path=url.substr(0,index+1)+'app';
	}
	else{
		if(url.indexOf(":")==-1){
			s+='url=http://'+url;
		}
		else{
			s+='url='+url;
		}
		path=url.substring(index1+1,index)+'.app';
		path.replace(/\//g,'');
		path=webtop.getSaveName( path);
	}
	s+='\nwidth=800\nheight=600\nenableDrag=1';
	webtop.writeFile(path,s);
}
var readyHandler=function(){
	webtop.move(0,0);
	webtop.max();
	setTimeout("webtop.toImage('screen.png');",1000);
	var href=location.href;
	var param=href.substr(href.indexOf('=')+1);//getArg('param');
	if(param){
		webtop.loadUrlIn(param)
	}
	/*else{
		newPage('http://www.alloyteam.com');
		//isBase=true;
		url='http://www.alloyteam.com';
		//transparent=true;
		isDefault=true;
	}*/
	console.log('ready');
}
var refreshHandler=function(){
	if(browserWindow){
		webtop.reloadIgnoreCache(browserWindow);
	}
	else{
		webtop.reloadIgnoreCache();
	}
}
addEventListener("webtopReady",readyHandler);
addEventListener("webtopWindowResize",sizeHandler);
addEventListener("webtopWindowMove",moveHandler);7
addEventListener("webtopDragDrop",dropHandler);
addEventListener("webtopWindowActive",activeHandler);
addEventListener("webtopWindowFocus",focusHandler);
addEventListener("webtopRefresh",refreshHandler);
addEventListener("webtopShowDev",showDevHandler);
addEventListener("webtopWindowClose",closeHandler);
refreshBtn.onclick=function(){
	refreshHandler()
}