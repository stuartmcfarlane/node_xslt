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

describe('node_xslt async operations', function(){

    it('contains readXsltFile', function() {
        // require node_xslt and test if the new async methods are defined
        var xslt = require('../');
        expect(xslt.readXsltFile).toBeDefined();
    });

    it('contains readXmlFile', function() {
        // require node_xslt and test if the new async methods are defined
        var xslt = require('../');
        expect(xslt.readXmlFile).toBeDefined();
    });

    it('contains readHtmlFile', function() {
        // require node_xslt and test if the new async methods are defined
        var xslt = require('../');
        expect(xslt.readHtmlFile).toBeDefined();
    });

    it('contains transform', function() {
        // require node_xslt and test if the new async methods are defined
        var xslt = require('../');
        expect(xslt.transform).toBeDefined();
    });

    it('can read an xml file', function(done){
        // read an xml string and see if the result is an xml document
        var xslt = require('../');
        xslt.readXmlFile('test/file.xml', function (err, xml) {
            expect(err).toBeUndefined();
            expect(xml).toBeDefined();
            done();
        });
    });

    it('gives error if it can\'t read an xml file', function(done){
        // read an xml string and see if the result is an xml document
        var xslt = require('../');
        xslt.readXmlFile('test/missing-file.xml', function (err, xml) {
            expect(err).toBeDefined();
            if (err) {
                expect(err).toContain('Failed to parse XML');
            }
            expect(xml).toBeUndefined();
            done();
        });
    });

    it('can read an html file', function(done){
        // read an html string and see if the result is an html document
        var xslt = require('../');
        xslt.readHtmlFile('test/file.html', function (err, html) {
            expect(err).toBeUndefined();
            expect(html).toBeDefined();
            done();
        });
    });

    it('gives error if it can\'t read an html file', function(done){
        // read an html string and see if the result is an html document
        var xslt = require('../');
        xslt.readHtmlFile('test/missing-file.html', function (err, html) {
            expect(err).toBeDefined();
            if (err) {
                expect(err).toContain('Failed to parse HTML');
            }
            expect(html).toBeUndefined();
            done();
        });
    });

    it('can read an xslt file', function(done){
        // read an xslt string and see if the result is a stylesheet object
        var xslt = require('../');
        xslt.readXsltFile('test/stylesheet.xslt', function (err, xslt) {
            expect(err).toBeUndefined();
            expect(xslt).toBeDefined();
            done();
        });
    });

    it('gives error if it can\'t read an xslt file', function(done){
        // read an missing xslt file and see if the error is defined
        var xslt = require('../');
        xslt.readXmlFile('test/missing-file.xslt', function (err, xslt) {
            expect(err).toBeDefined();
            if (err) {
                expect(err).toContain('Failed to parse XML');
            }
            expect(xslt).toBeUndefined();
            done();
        });
    });

    it('gives error if it can\'t parse an xslt file', function(done){
        // read an missing xslt file and see if the error is defined
        var xslt = require('../');
        xslt.readXsltFile('test/invalid.xslt', function (err, xslt) {
            expect(err).toBeDefined();
            if (err) {
                expect(err).toContain('Failed to parse XSLT');
            }
            expect(xslt).toBeUndefined();
            done();
        });
    });


    it('can transform', function(done) {
        var xslt = require('../');
        var stylesheet = xslt.readXsltString(stylesheetXslt);
        var xml = xslt.readXmlString(stringXml);
        xslt.transform(stylesheet, xml, [], function (err, transformed){
            expect(err).toBeUndefined();
            expect(transformed).toContain('<td>Bob Dylan</td>');
            done();
        });
    });
});
