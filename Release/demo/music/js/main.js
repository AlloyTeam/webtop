var img=new Image();
function report(tag){
	//img.src='http://cgi.appstore.qplus.com/qplusown/report?tag='+tag+'&t='+new Date().getTime();
}
var getCookie=function(name) {
	var r = new RegExp("(?:^|;+|\\s+)" + name + "=([^;]*)");
	// var r = new RegExp("(?:^|;+|\\s+)" + name + "=([^;]*?)(?:;|$)");
	var m = window.document.cookie.match(r);
	return (!m ? "" : m[1]);
	// document.cookie.match(new
	// RegExp("(?:^|;+|\\s+)speedMode=([^;]*?)(?:;|$)"))
}
// 获取cookie中的uin
var getCookieUin = function(){
	var cookieUin = getCookie("uin", 'qplus.com');
	if(cookieUin){
		cookieUin = parseInt(cookieUin.substr(1), 10);
	}else{
		cookieUin = null;
	}
	
	return cookieUin;
};

function PngAnimate(imgSrc,singleWidth,width,height,duration,el){
	this.imgSrc=imgSrc;
	this.singleWidth=singleWidth;
	this.width=width;
	if(!el){
		el=document.createElement('el');
		el.style.width=singleWidth+'px';
		el.style.height=height+'px';
	}
	this.duration=duration;
	this.el=el;
	this.init();
}
PngAnimate.prototype={
	init:function(){
		var el=this.el;
		var imgSrc=this.imgSrc;
		el.style.background='url('+imgSrc+') 0 0 no-repeat';
		//this.start();
	},
	stop:function(){
		this.timeId&&clearInterval(this.timeId);
		this.timeId=0;
	},
	start:function(){
		var duration=this.duration;
		var width=this.width;
		var i=1;
		var el=this.el;
		var singleWidth=this.singleWidth;
		var n=parseInt(width/singleWidth);
		var id=setInterval(function(){
			el.style.backgroundPosition='-'+i*singleWidth+'px 0';
			++i;
			if(i>=n){
				i=0;
			}
		}, duration);
		this.timeId=id;
	},
	hide:function(){
		this.el.style.display='none';
	},
	show:function(){
		this.el.style.display='block';
	}
}

document.onselectstart=function(){
	return false;
}
title.onmousedown=singer.onmousedown=dragEl.onmousedown=function(e){
  parent.postMessage('click:'+e.clientX+'|'+e.clientY,'*');
  return false;
}
var MAX_ERROR_TIMES=3;
var errorTimes=0;
var audioEl;
var silent=false;
var timeState=0;
var wait=false;
function initAudio(){
	var _audio;
	if(audioEl){ return; } //如果存在,说明已经初始化
	if(window['Audio'] && (_audio=document.createElement('audio'))){
		_audio.addEventListener('canplay',onCanPlay,false);
		_audio.addEventListener('play',onPlay,false);
		_audio.addEventListener('pause',onPause,false);
		_audio.addEventListener('ended',onEnded,false);
		_audio.addEventListener('error',onError,false);
		_audio.addEventListener('progress',onProgress,false);
		_audio.addEventListener('timeupdate',onTimeUpdate,false);
		prevVolume=_audio.volume=0.5;
		container.appendChild(_audio);
		
		audioEl=_audio;
		
		range.addEventListener('click',onRangeClick,false);
		volume.addEventListener('click',onVolumnClick,false);
		play.addEventListener('click',onPlayButtonClick,false);
		next.addEventListener('click',onNextButtonClick,false);
		speaker.addEventListener('click',onSpeakerClick,false);
	}else{
		return;
	}
}
var id=0,src;
function initMusic(){
	time.innerHTML=formatTime(-1);
	pass.style.width=buffer.style.width=0;
}
function loadAudio(songData,flag){
	var singerTip=singer.innerHTML=songData[1]+' - '+songData[0];
	//var titleTip=title.innerHTML=songData[0];
	pic.src=songData[3];
	if(!audioEl){ return; }
	//time.innerHTML='--:-- / --:--';
	audioEl.pause();
	wait=false;
	if(id){
		clearTimeout(id);
		id=null;
	}
	if(audioEl.autoplay){
		if(flag){
			//audioEl.src='';
			id=setTimeout(function(){
				audioEl.src=songData[2];
				id=null;
				wait=false;
			}, 1000);
			wait=true;
		}
		else{
			audioEl.src=songData[2];
		}
	}
	else{
		src=songData[2];
	}
	if(singer.scrollWidth>singer.clientWidth){
		singer.setAttribute('title', singerTip);
	}
	else{
		singer.setAttribute('title', '');
	}
	initMusic();
	//title.setAttribute('title',titleTip);
	//audioEl.load();
}
function onCanPlay(){
}
function onPlay(){
	//play.className='pause';
  	/*parent.postMessage('start','*');*/
	animate.start();
	animate.show();
}
function onPause(){
	//play.className='play';
  	/*parent.postMessage('stop','*');*/
	animate.stop();
	animate.hide();
}
function onEnded(){
	audioEl.pause();
	audioEl.currentTime=0;
  	parent.postMessage('switch','*');
	nextSong();
	animate.stop();
	animate.hide();
}
function onError(){
	singer.innerHTML='<span style="color:red">加载错误</span>';
	play.className='play';
	audioEl.pause();
	//audioEl.src='';
	console.log('err');
	if(errorTimes<MAX_ERROR_TIMES){
		audioEl.autoplay=true;
		nextSong();
		animate.stop();
		animate.hide();
	}
	else{
  		parent.postMessage('stop','*');
	}
	++errorTimes;
}
function onTimeUpdate(){
	var pos=audioEl.currentTime,
		dora=audioEl.duration;
	var buf=audioEl.buffered.length?audioEl.buffered.end(0):0;
	if(timeState==0){
		time.innerHTML=formatTime(pos);//+'/'+formatTime();//
	}
	else{
		if(dora){
			time.innerHTML='-'+formatTime(dora-pos);//+'/'+formatTime();//
		}
		else{
			time.innerHTML=formatTime(dora);//+'/'+formatTime();//
		}
	}
	if(isFinite(dora) && dora>0){
		pass.style.width=pos/dora*100+"%";
		animateContainer.style.left=pass.offsetWidth+50+'px';
	}
}
function onProgress(){
	if(!wait){
		var buf=audioEl.buffered.length?audioEl.buffered.end(0):0,
			dora=audioEl.duration;
		buffer.style.width=buf/dora*100+"%";
	}
}
function onSpeakerClick(e){
	silent=!silent;
	if(silent){
		speaker.className='silent';
    	if(audioEl){
			audioEl.volume=0;
			volumeIn.style.width="0";
		}  
	}
	else{
		speaker.className='speak';
    	if(audioEl){
			audioEl.volume=prevVolume;
			volumeIn.style.width=prevVolume*100+"%";
		}  
	}
	return false;
}
function onRangeClick(e){
	if(!audioEl){ return false; }
	var buf=audioEl.buffered.length?audioEl.buffered.end(0):0,
		dora=audioEl.duration;
	if(isFinite(dora) && dora>0){
		var value=(e.clientX-87)/172,
			pos=value*dora;
		if(pos>buf){
			pos=buf;
		}
		audioEl.currentTime=pos;
	}
	return false;
}
function onVolumnClick(e){
    if(!audioEl){ return false; }  
	var value=(e.clientX-volume.offsetLeft-volume.parentNode.offsetLeft)/volume.offsetWidth;
	volumeIn.style.width=value*100+"%";
    audioEl.volume=value; 
	prevVolume=value; 
	if(value){
		silent=false;
		speaker.className='speak';
	}
	else{
		silent=true;
		speaker.className='silent';
	}
	return false;
}
function onPlayButtonClick(){
	if(!audioEl){ return false; }
	if(audioEl.error){ //加载错误
		return false;
	}else if(audioEl.readyState<2){ //还没可以播放
		if(src){
			audioEl.src=src;
			src=null;
		}
		audioEl.autoplay^=true; //切换是否autoplay
		play.className=audioEl.autoplay?'pause':'play';
		if(audioEl.autoplay){
			parent.postMessage('start','*');
			//animate.start();
			//animate.show();
		}
		else{
			parent.postMessage('stop','*');
			//animate.stop();
			//animate.hide();
		}
		report(30405);
	}else if(audioEl.paused){
		audioEl.play();
		play.className='pause';
		parent.postMessage('start','*');
		report(30405);
	}else{
		audioEl.pause();
		play.className='play';
		parent.postMessage('stop','*');
	}
	return false;
}
function formatTime(sec){
	if(!isFinite(sec) || sec<0){
		return '';//'--:--';
	}
	else{
		var m=Math.floor(sec/60),
			s=Math.floor(sec)%60;
		return (m<10?'0'+m:m)+':'+(s<10?'0'+s:s);
	}
}
function makeAlbumPic(album_id){
	return 'http://imgcache.qq.com/music/photo/album/'+album_id%100+'/68_albumpic_'+album_id+'_0.jpg';
}
function makeSingerPic(singer_id){
	return 'http://imgcache.qq.com/music/photo/singer/'+singer_id%100+'/68_singerpic_'+singer_id+'_0.jpg';
}
var index=0;
var playList=[];
function onNextButtonClick(e){
	next.removeEventListener('click',onNextButtonClick);
	nextSong(true);
	report(30406);
	setTimeout(function(){next.addEventListener('click',onNextButtonClick);},500);
	return false;
}
function nextSong(flag){
	++index;
	if(index==playList.length-1){
		loadList();
	}
	else if(index>=playList.length){
		index=0;
	}
	audioEl.autoplay=true;
	play.className='pause';
	parent.postMessage('start','*');
	playSong(index,flag);
	//audioEl.play();
}
var init=false;
var JsonCallBack=SongRecCallback=function(data){
	if(!init){
		init=true;
		initAudio();
	}
	var regexp = new RegExp('(upload|stream)(\\d+)\\.(music\\.qzone\\.soso\\.com|qqmusic\\.qq\\.com)\\/(\\d+)\\.wma');
	var replacement = function(word,x,a,y,b){
		return 'stream'+(10+Number(a))+'.qqmusic.qq.com/'+(18000000+Number(b))+'.mp3';
	};
	var songs=data.songs;
	for(var i=0;i<songs.length;++i){
		var song=songs[i];
		var args=decodeURIComponent(song.data).replace(/\+/g,' ').split('|');
		var singerId=args[2];
		var albumId=args[4];
		var name=args[1];
		var singer=args[3];
		playList.push([name, singer, decodeURIComponent(song.url).replace(regexp, replacement), makeAlbumPic(albumId)]);
	}
	playSong(index,true);
}
function playSong(index,flag){
	var song=playList[index];
	loadAudio(song,flag);
}
function injectScript(url){
	var oScript=document.createElement("script");
	oScript.src=url;
	oScript.charset='gb2312';
	document.body.appendChild(oScript);
}
pic.onclick=openApp.onclick=function(e){
	parent.postMessage('openApp','*');
	return false;
}
function isLogin(){
	return getCookie('skey');
}
var logined=false;//isLogin();
if(logined){
	var url="http://s.plcloud.music.qq.com/fcgi-bin/song_sim.fcg?uin="+getCookieUin()+"&simple=0&num=20&rnd=";
	var loadList=function(){
		injectScript("http://ptlogin2.qplus.com/ho_cross_domain?tourl="+encodeURIComponent(url+new Date().getTime()+"&start="+playList.length));
		loadList=function(){
			injectScript(url+new Date().getTime()+"&start="+playList.length);
		}
	}
}
else{
	var loadList=function(){
		injectScript("http://radio.cloud.music.qq.com/fcgi-bin/qm_guessyoulike.fcg?labelid=118&start=0&num=20&rnd="+new Date().getTime());
	}
}
loadList();
window.addEventListener("message", function(e){
   var data=e.data;
   switch(data){
	   case 'play':
		  if(audioEl){
			 if(src){
				 audioEl.src=src;
				 audioEl.play();
			 }
			 else{
				 audioEl.play();
			 }
			 audioEl.autoplay=true;
		  }
		  break;
	   case 'pause':
		  if(audioEl){
			 audioEl.pause();
		  }
		  break;
   }
});
var animate=new PngAnimate('http://qplus2.idqqimg.com/module/widget/music/images/animate.png',46,368,26,200,animateContainer);
//animate.stop();
time.onclick=function(){
	++timeState;
	if(timeState>1){
		timeState=0;
	}
	return false;
}
var readyHandler=function(){
	webtop.move(100,100);
	webtop.setSize(270,80);
	setTimeout('webtop.toImage("screen.png")',2000);
}
addEventListener("webtopReady",readyHandler);
closeBtn.onclick=function(){
	webtop.close();
}
