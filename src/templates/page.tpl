<p:Page xmlns:p="http://www.evolus.vn/Namespace/Pencil">
    <p:Properties>
        <p:Property name="id">{{PAGEID}}</p:Property>
        <p:Property name="name">Level{{PAGELEVEL}}</p:Property>
        <p:Property name="width">{{PAGEWIDTH}}</p:Property>
        <p:Property name="height">{{PAGEHEIGHT}}</p:Property>
        <p:Property name="pageFileName">page_{{PAGEID}}.xml</p:Property>
        <p:Property name="zoom">0.5</p:Property>
    </p:Properties>
    <p:Content>
        {{#RECTANGLES}}
        <g xmlns="http://www.w3.org/2000/svg" xmlns:p="http://www.evolus.vn/Namespace/Pencil" p:type="Shape" p:def="Evolus.Common:RoundedRect" id="{{SHAPEID}}" transform="matrix(1,0,0,1,{{LOWERLEFTX}},{{LOWERLEFTY}})">
            <p:metadata>
                <p:property name="box"><![CDATA[{{WIDTH}},{{HEIGHT}}]]></p:property>
                <p:property name="withBlur"><![CDATA[false]]></p:property>
                <p:property name="radius"><![CDATA[0,0]]></p:property>
                <p:property name="textPadding"><![CDATA[0,10]]></p:property>
                <p:property name="fillColor"><![CDATA[#{{FILLCOLOUR}}]]></p:property>
                <p:property name="strokeColor"><![CDATA[#1B3280FF]]></p:property>
                <p:property name="strokeStyle"><![CDATA[0|]]></p:property>
                <p:property name="textContent"><![CDATA[{{TEXTCONTENT}}]]></p:property>
                <p:property name="textFont"><![CDATA["Liberation Sans",Arial,sans-serif|normal|normal|13px|none|0]]></p:property>
                <p:property name="textColor"><![CDATA[#000000FF]]></p:property>
                <p:property name="textAlign"><![CDATA[1,1]]></p:property>
            </p:metadata>

            <defs>
                <rect width="{{WIDTH}}" height="{{HEIGHT}}" rx="0" ry="0" x="0" y="0" style="stroke-width: 0; fill: rgb(67, 136, 204); fill-opacity: 1; stroke: rgb(27, 50, 128); stroke-opacity: 1;" p:name="rrRect" id="{{RECTANGLEID}}" transform="translate(0,0)"/>
                <filter height="1.2558399" y="-0.12792" width="1.06396" x="-0.03198" p:name="shadingFilter" id="{{FILTERID}}">
                    <feGaussianBlur stdDeviation="1" in="SourceAlpha"/>
                </filter>
            </defs>
            <use xmlns:xlink="http://www.w3.org/1999/xlink" xlink:href="#{{RECTANGLEID}}" transform="translate(2, 2)" p:filter="url(#{{FILTERID}})" style="opacity: 0.5; visibility: hidden; display: none;" p:heavy="true" p:name="bgCopy" id="{{MISCID}}"/>
            <use xmlns:xlink="http://www.w3.org/1999/xlink" xlink:href="#{{RECTANGLEID}}"/>
            <foreignObject x="10" y="31" width="180" height="18" p:name="text" id="{{TEXTID}}" style="font-family: &quot;Liberation Sans&quot;, Arial, sans-serif; font-size: 13px; font-weight: normal; font-style: normal; text-decoration: none; fill: rgb(0, 0, 0); fill-opacity: 1; color: rgb(0, 0, 0); text-align: center;">
                <div xmlns="http://www.w3.org/1999/xhtml">
                    {{TEXTCONTENT}}
                </div>
            </foreignObject>
        </g>
        {{/RECTANGLES}}
    </p:Content>
</p:Page>
