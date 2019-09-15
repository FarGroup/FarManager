<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet
	version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:svg="http://www.w3.org/2000/svg"
	xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
	>

	<xsl:include href="filter_none.xslt"/>

	<xsl:template match="svg:use[@id='rectPanelsBackgroundFill']/@style">
		<xsl:attribute name="style">
			<xsl:value-of select="'fill:#ff0000'"/>
		</xsl:attribute>
	</xsl:template>

	<xsl:template match="svg:use[@id='pathPanelsGridFill']/@style">
		<xsl:attribute name="style">
			<xsl:value-of select="'fill:#ffffff'"/>
		</xsl:attribute>
	</xsl:template>

</xsl:stylesheet>
