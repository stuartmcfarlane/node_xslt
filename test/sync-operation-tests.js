var jasmine = require('jasmine-node');

var stylesheetXslt = [
    '<?xml version="1.0"?>',
    '<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">',
    '<xsl:template match="/">',
    '  <html>',
    '  <body>',
    '    <h2>My CD Collection</h2>',
    '    <table border="1">',
    '      <tr bgcolor="#9acd32">',
    '        <th>Title</th>',
    '        <th>Artist</th>',
    '      </tr>',
    '      <xsl:for-each select="catalog/cd">',
    '        <tr>',
    '          <td><xsl:value-of select="title"/></td>',
    '          <td><xsl:value-of select="artist"/></td>',
    '        </tr>',
    '      </xsl:for-each>',
    '    </table>',
    '  </body>',
    '  </html>',
    '  </xsl:template>',
    '</xsl:stylesheet>'
].join('');

var stringXml = [
'<?xml version="1.0" encoding="ISO-8859-1"?>',
'<catalog>',
'  <cd>',
'    <title>Empire Burlesque</title>',
'    <artist>Bob Dylan</artist>',
'    <country>USA</country>',
'    <company>Columbia</company>',
'    <price>10.90</price>',
'    <year>1985</year>',
'  </cd>',
'</catalog> '
].join('');

describe('node_xslt sync operations', function(){

    it('exist', function() {
        // require node_xslt and test of the original methods are defined
        var xslt = require('../');
        expect(xslt.readXsltString).toBeDefined();
        expect(xslt.readXsltFile).toBeDefined();
        expect(xslt.readXmlString).toBeDefined();
        expect(xslt.readXmlFile).toBeDefined();
        expect(xslt.readHtmlString).toBeDefined();
        expect(xslt.readHtmlFile).toBeDefined();
        expect(xslt.transform).toBeDefined();
    });

    it('can read an xslt string', function(){
        // read an xslt string and see if the result is a stylesheet object
        var xslt = require('../');
        var stylesheet = xslt.readXsltString(stylesheetXslt);
        expect(stylesheet).toBeDefined();
    });

    it('can read an xslt file', function(){
        // read an xslt string and see if the result is a stylesheet object
        var xslt = require('../');
        var stylesheet = xslt.readXsltFile('test/stylesheet.xslt');
        expect(stylesheet).toBeDefined();
    });

    // it('give parsing errors for a bad xslt file', function(){
    //     // read an xslt string and see if the result is a stylesheet object
    //     var xslt = require('../');
    //     var stylesheet;
    //     expect(stylesheet = xslt.readXsltFile('test/invalid.xslt')).toThrow();
    //     expect(stylesheet).toBeUndefined();
    // });

    it('can read an xml string', function(){
        // read an xslt string and see if the result is a stylesheet object
        var xslt = require('../');
        var xml = xslt.readXmlString(stringXml);
        expect(xml).toBeDefined();
    });

    it('can read an xml file', function(){
        // read an xslt string and see if the result is a stylesheet object
        var xslt = require('../');
        var xml = xslt.readXmlFile('test/file.xml');
        expect(xml).toBeDefined();
    });

    it('can transform', function() {
        var xslt = require('../');
        var stylesheet = xslt.readXsltString(stylesheetXslt);
        var xml = xslt.readXmlString(stringXml);
        var transformed = xslt.transform(stylesheet, xml, []);
        expect(transformed).toContain('<td>Bob Dylan</td>');
    });
});
