function makeLink(a) {
  if (a.className.indexOf("msdocs") === -1) {
    return;
  }
  a.href = "https://learn.microsoft.com/search/?terms=" + a.text;
  if (a.className.indexOf("archive") > -1) {
    a.href = a.href + "&dataSource=previousVersions";
  }
  a.target = "_blank";
}

window.onload = function() {
  //var anchors = document.getElementsByClassName("msdocs");
  //var anchors = document.querySelectorAll("a.msdocs");

  // Neither getElementsByClassName nor querySelectorAll used to be supported in old IE.
  // They are unavailable in chm too, unless x-ua-compatible meta specified.
  // https://learn.microsoft.com/previous-versions/windows/internet-explorer/ie-developer/compatibility/jj676915(v=vs.85)
  // <meta http-equiv="x-ua-compatible" content="IE=edge">
  // At the moment we are not ready to apply this meta, because current css was designed for quirks mode.
  // That's why we stick to more ancient getElementsByTagName for now.

  var anchors = document.getElementsByTagName('a');
  for (var i = 0; i < anchors.length; i++) {
    makeLink(anchors[i]);
  }
}
