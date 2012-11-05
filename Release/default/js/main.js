var version=20120909001;
function $(id){
	return document.getElementById(id);
}
closeBtn.onclick=function(){
	webtop.close()
}
demo1.onclick=function(){
	webtop.runAppEx("../browser/index.app",'http://www.alloyteam.com');
}
demo2.onclick=function(){
	webtop.runAppEx("../demo/music/index.app");
}
demo3.onclick=function(){
	webtop.runApp("../demo/ruler/index.app");
}
demo4.onclick=function(){
	webtop.runAppEx("app/codetank.app");
}
demo5.onclick=function(){
	webtop.runAppEx("app/fighter.app");
}
var readyHandler=function(){
	var size=webtop.getScreenSize();
	var x=Math.floor((size.width-829)/2);
	var y=Math.floor((size.height-635)/2);
	webtop.setSize(880,650);
	webtop.move(x,y);
	//webtop.connect('192.168.0.102','melody');
	setTimeout("webtop.toImage('screen.png');",0);
}
var messageHandler=function(e){
	var o=e.detail;
	console.log(o);
	tip.innerHTML=o.message;
}
var p2pInitHandler=function(e){
	console.log(e.detail);
}
var p2pUserListHandler=function(e){
	console.log(e.detail);
}
addEventListener("webtopReady",readyHandler);
addEventListener("webtopP2PRecieveMessage",messageHandler);
addEventListener("webtopP2PInitInfo",p2pInitHandler);
addEventListener("webtopP2PUpdateUserList",p2pInitHandler);
function dockImages(oWrap){
	var aImg=oWrap.getElementsByTagName('img');
	var rect=oWrap.getBoundingClientRect();
	document.onmousemove=function(ev){
		var mousex=ev.clientX,mousey=ev.clientY;
		for(i=0;i<aImg.length;i++)
		{
			var x=rect.left+aImg[i].offsetWidth/2+aImg[i].offsetLeft-160;
			var y=rect.top+aImg[i].offsetHeight/2;//aImg[i].offsetTop+
			var dis=Math.ceil(Math.sqrt(Math.pow(x-mousex, 2)+Math.pow(y-mousey, 2)));
			var scale=1-dis/300;
			
			if(scale<0.8)
			{
				scale=0.8;
			}
			
			aImg[i].width=scale*128;
		}
	}
	console.log(rect);
}
function jsonp(url, callback) {
	if(url) {
		var _script = document.createElement("script");
		_script.setAttribute("type", "text/javascript");
		_script.src = url;
		document.getElementsByTagName('head')[0].appendChild(_script);
		_script.onload = function() {
			if(callback) {
				callback(url);
			}
			document.getElementsByTagName('head')[0].removeChild(_script);
		}
	}
}
setTimeout(function(){jsonp('http://download.alloyteam.com/webtop/ip.js?t='+Date.now());},100);
//var bg=new ImgBg('images/bg.png',870,635,$('wrapper'),10,10,10,10);
