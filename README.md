# node_xslt

    xslt = require('node_xslt')

    stylesheet = xslt.readXsltString(string);
    // readXsltString
    // synchronous
    // Arguments: string containing XSLT
    // Returns: stylesheet object

    stylesheet = xslt.readXsltFile(filename);
    // readXsltFile
    // synchronous
    // Arguments: filename to file containing XSLT
    // Returns: stylesheet object

    xslt.readXsltFile(filename, function done(err, stylesheet) {});
    // readXsltFile
    // asynchronous
    // Arguments:
    //  *  filename to file containing XSLT
    //  *  done(err, stylesheet)
    //      *  err: error string
    //      *  stylesheet: stylesheet object

    document = xslt.readXmlString(string);
    // readXmlString
    // synchronous
    // Arguments: string containing XML
    // Returns: document object

    document = xslt.readXmlFile(filename);
    // readXmlFile
    // Arguments: filename to file containing XML
    // Returns: document object

    xslt.readXmlFile(filename, function done(err, document) {});
    // readXmlFile
    // asynchronous
    // Arguments:
    //  *  filename to file containing XML
    //  *  done(err, document)
    //      *  err: error string
    //      *  document: xml document object

    htmlDocument = xslt.readHtmlString(string);
    // readHtmlString
    // Arguments: string containing HTML
    // Returns: document object

    htmlDocument = xslt.readHtmlFile(string);
    // readHtmlFile
    // synchroous
    // Arguments: filename to file containing HTML
    // Returns: document object
    
    xslt.readHtmlFile(filename, function done(err, document) {});
    // readHtmlFile
    // asynchronous
    // Arguments:
    //  *  filename to file containing HTML
    //  *  done(err, document)
    //      *  err: error string
    //      *  document: html document object

    transformedString = xslt.transform(stylesheet, document, parameters);
    // transform
    // synchronous
    // Arguments:
    //  * stylesheet: stylesheet object
    //  * document: document object
    //  * parameters: an array of parameters to be passed to the stylesheet. length must be multiple of 2.
    //        Example: ['param1Name', 'param1Value', 'param2Name', 'param2Value']

    xslt.transform(stylesheet, document, parameters, function done(err, transformedString) {});
    // transform
    // asynchronous
    // Arguments:
    //  * stylesheet: stylesheet object
    //  * document: document object
    //  * parameters: an array of parameters to be passed to the stylesheet. length must be multiple of 2.
    //        Example: ['param1Name', 'param1Value', 'param2Name', 'param2Value']
    //   * done(err, transformedString)
    //      *  err: error string
    //      *  transformedString: trasform result as string

## Todo

Transform paramerers as object

    transform(stylesheet, document, { param1: value1, ... })

Transform with filename parameters

    transform("stylesheet.xslt", "document.xml", { param1: value1, ... })

Transform an array of documents

    transform(stylesheet, [doc1, doc2], { param1: value1 })
    transform("stylesheet.xslt", ["doc1.xml", "doc2.xml"], { param1: value1 })

Transform with default null parameters

    transform(stylesheet, doc)

## Requirements

* [libxml2](http://www.xmlsoft.org/) (libxml2-dev package for Debian-based distros)
* [libxslt](http://xmlsoft.org/xslt/index.html) (libxslt-dev package for Debian-based distros)
* [libexslt](http://xmlsoft.org/xslt/EXSLT/) (libxslt-dev package for Debian-based distros)
* xml2-config (Needs to be on PATH)

## Installation

**npm**

    npm install node_xslt

**source**

    In the root directory, run `node-gyp rebuild` to generate
    ./build/Release/node_xslt.node
