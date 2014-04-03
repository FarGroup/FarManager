var OrigColor;

var roll = document.images != null;
var actcolor = "#bfd5ff";

if (document.images != null && navigator.userAgent.indexOf("Mozilla", 0) != -1)
	document.onclick = mouseclick;

function mouseclick(evt){
	evt= evt || window.event;
	if (evt.type == "click")
	{
		var current = evt.target || evt.srcElement || evt.originalTarget;
		if(current.tagName == "A" && current.href!="")
			current.style.color = actcolor;
	}
}

function cpy(a){
	window.clipboardData.setData("Text",a);
};
