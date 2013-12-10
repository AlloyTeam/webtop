;(function (identifier) {
    if(window.top!=window)return;
    console.log(identifier);
    var e=document.createElement('div');
    e.innerText="Debug";
    e.style.cssText="background:#f00;position:absolute;right:0;top:0;z-index:1000;";
	document.addEventListener("DOMContentLoaded", function(){
        document.removeEventListener("DOMContentLoaded", arguments.callee, false);
        document.body.appendChild(e);
        poll = new XMLHttpRequest();
        poll.open("POST", "debug?identifier="+identifier, true);
        poll.onreadystatechange=function()
        {
            if (poll.readyState==4&&poll.status==200)
            {
                try{
                    var text=poll.responseText;
                    var index=text.indexOf(':');
                    var pre=text.substr(0,index);
                    var js=text.substr(index+1);
                    result=pre+':'+eval(js);
                }
                catch(e){
                    result="";
                }
                poll.open("POST", "debug?identifier="+identifier, true);
                poll.send(result);
            }
        };
        setTimeout(function(){
            poll.send("connect:"+identifier);
        },2000);
    });
})("guid");