<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet
	version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:svg="http://www.w3.org/2000/svg"
	xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
	>

	<xsl:include href="filter_none.xslt"/>

	<xsl:template match="svg:g[contains(@inkscape:label, ' FX')]"/>

</xsl:stylesheet>
