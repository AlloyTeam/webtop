function obj2string(o){
	var r=[];
	if(typeof o=="string"){
		return "\""+o.replace(/([\'\"\\])/g,"\\$1").replace(/(\n)/g,"\\n").replace(/(\r)/g,"\\r").replace(/(\t)/g,"\\t")+"\"";
	}
	if(typeof o=="object"){
		if(!o.sort){
			for(var i in o){if(!o[i]){r.push('"'+i+"\":null");continue;}
				r.push('"'+i+"\":\""+o[i].toString().replace(/([\'\"\\])/g,"\\$1").replace(/(\n)/g,"\\n").replace(/(\r)/g,"\\r").replace(/(\t)/g,"\\t")+"\"");
			}
			if(!!document.all&&!/^\n?function\s*toString\(\)\s*\{\n?\s*\[native code\]\n?\s*\}\n?\s*$/.test(o.toString)){
				r.push("toString:"+o.toString.toString());
			}
			r="{"+r.join()+"}";
		}else{
			for(var i=0;i<o.length;i++){
				if(o[i]==null)r.push("null");else{r.push("\""+o[i].replace(/([\'\"\\])/g,"\\$1").replace(/(\n)/g,"\\n").replace(/(\r)/g,"\\r").replace(/(\t)/g,"\\t")+"\"")}
			}
			r="["+r.join()+"]";
		} 
		return r;
	}
	return o.toString();
}