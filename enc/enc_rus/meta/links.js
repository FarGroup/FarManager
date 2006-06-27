var OrigColor;

var roll = document.images != null;
var actcolor = "#bfd5ff";

if (roll && navigator.userAgent.indexOf("Mozilla", 0) != -1)
  document.onclick = mouseclick;

function mouseclick(){
  if(window.event.srcElement.tagName == "A" && window.event.srcElement.href!=""){
     window.event.srcElement.style.color = actcolor;
  }
}

function cpy(a){
  window.clipboardData.setData("Text",a);
};

function DrawNotFound()
{
  if (roll && navigator.userAgent.indexOf("Mozilla", 0) != -1)
  {
    var agent = navigator.userAgent.toLowerCase();
    if(agent.indexOf("msie 4")>0 || agent.indexOf("msie 5")>0 || agent.indexOf("msie 6")>0)
    {
      f=document.location.href.indexOf("?")
      if(f > 0)
      {
        qu=document.location.href.substring(f+1)
        inf.innerHTML=" по <a href='http://search.microsoft.com/default.asp?boolean=ALL&nq=NEW&so=RECCNT&p=1&ig=01&i=00&i=01&i=02&i=03&i=04&i=05&i=06&i=07&i=08&i=09&i=10&i=11&i=12&i=13&i=14&i=15&i=16&i=17&i=18&i=19&i=20&i=21&i=22&i=23&i=24&i=25&i=26&i=27&i=28&i=29&i=30&i=31&i=32&i=33&i=34&i=35&i=36&i=37&i=38&i=39&i=40&i=41&siteid=us/dev&qu=" + qu + "' target='_blank' style='font-weight:bold;'>" +qu+ "</a> "
      }
    }
  }
};
