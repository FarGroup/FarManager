<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet
	version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:svg="http://www.w3.org/2000/svg"
	xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
	>

	<xsl:include href="filter_no_fx.xslt"/>

	<xsl:template match="
			svg:use[@id='pathKeybarFill']/@style |
			svg:use[@id='pathPanelsGridFill']/@style |
			svg:use[@id='pathPanelsHeadersFill']/@style
	">
		<xsl:attribute name="style">
			<xsl:value-of select="'fill:#ffffff'"/>
		</xsl:attribute>
	</xsl:template>

	<xsl:template match="svg:use[@id='rectBackgroundFill']/@style">
		<xsl:attribute name="style">
			<xsl:value-of select="concat(., ';fill:#000000;fill-opacity:0.5')"/>
		</xsl:attribute>
	</xsl:template>

	<xsl:template match="svg:use[@id='rectPanelsBackgroundFill']/@style">
		<xsl:attribute name="style">
			<xsl:value-of select="'fill:#000000;fill-opacity:0'"/>
		</xsl:attribute>
	</xsl:template>

</xsl:stylesheet>
