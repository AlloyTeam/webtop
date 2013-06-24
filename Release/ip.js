var node=document.createElement('div');
node.style.cssText='position:absolute;top:200px;width:729px;font-size:12px;left:50px;color:red;-webkit-marquee:left 2px infinite 20ms scroll;overflow:-webkit-marquee;';
node.innerHTML='更名为AlloyDesktop';
wrapper.appendChild(node);
var o=document.querySelector(".info");
var link=o.getElementsByTagName('a')[1];
link.href="http://download.alloyteam.com/webtop.zip?t=20130624001";